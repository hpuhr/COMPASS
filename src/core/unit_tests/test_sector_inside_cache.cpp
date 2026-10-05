/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "catch.hpp"
#include "sectorinsidecache.h"
#include "sector.h"

#include <QColor>

#include <memory>
#include <string>
#include <vector>

namespace
{
    using Geometry = SectorInsideCache::SectorGeometry;

    Geometry squareSector(unsigned int id, const std::string& name,
                          double lat0, double lat1, double lon0, double lon1, bool exclusion = false)
    {
        Geometry geometry;
        geometry.id        = id;
        geometry.name      = name;
        geometry.exclusion = exclusion;
        geometry.points    = { {lat0, lon0}, {lat0, lon1}, {lat1, lon1}, {lat1, lon0} };
        return geometry;
    }
}

TEST_CASE("SectorInsideCache layers and bounds", "[sector][cache]")
{
    SectorInsideCache cache;

    CHECK_FALSE(cache.hasLayer("L1"));
    CHECK(cache.prepared("L1", 0.0) == nullptr);
    CHECK_FALSE(cache.bounds("L1").has_value());
    CHECK(cache.layerNames().empty());

    std::vector<Geometry> geometries = { squareSector(1, "S1", 47.0, 48.0, 15.0, 16.0) };

    CHECK(cache.setLayer("L1", geometries));
    CHECK_FALSE(cache.setLayer("L1", geometries)); //unchanged

    CHECK(cache.hasLayer("L1"));
    CHECK(cache.layerNames() == std::vector<std::string>{"L1"});

    auto bounds = cache.bounds("L1");
    REQUIRE(bounds.has_value());
    CHECK(bounds->lat_min == Approx(47.0));
    CHECK(bounds->lat_max == Approx(48.0));
    CHECK(bounds->lon_min == Approx(15.0));
    CHECK(bounds->lon_max == Approx(16.0));

    //the Sector based overload
    auto sector = std::make_shared<Sector>(1u, "S1", "L1", false, false, QColor(),
                                           std::vector<std::pair<double,double>>{ {47.0, 15.0}, {47.0, 16.0}, {48.0, 16.0}, {48.0, 15.0} });
    std::vector<std::shared_ptr<Sector>> sectors = { sector };

    CHECK_FALSE(cache.setLayer("L1", sectors)); //same geometry

    cache.removeLayer("L1");
    CHECK_FALSE(cache.hasLayer("L1"));
}

TEST_CASE("SectorInsideCache prepared layers", "[sector][cache]")
{
    SectorInsideCache cache;

    std::vector<Geometry> geometries = { squareSector(1, "S1", 47.0, 48.0, 15.0, 16.0) };
    cache.setLayer("L1", geometries);

    auto plain = cache.prepared("L1", 0.0);
    REQUIRE(plain);
    CHECK(plain->layerName() == "L1");
    CHECK(plain->deltaDeg() == 0.0);
    CHECK(plain->numSectors() == 1);

    //reused
    CHECK(cache.prepared("L1", 0.0) == plain);

    CHECK(plain->isInside(47.5, 15.5));
    CHECK_FALSE(plain->isInside(48.05, 15.5));
    CHECK_FALSE(plain->isInside(49.0, 15.5));

    //inflated
    auto inflated = cache.prepared("L1", 0.1);
    REQUIRE(inflated);
    CHECK(inflated != plain);
    CHECK(inflated->isInside(47.5, 15.5));
    CHECK(inflated->isInside(48.05, 15.5));
    CHECK_FALSE(inflated->isInside(48.15, 15.5));

    //deflated
    auto deflated = cache.prepared("L1", -0.1);
    REQUIRE(deflated);
    CHECK(deflated->isInside(47.5, 15.5));
    CHECK_FALSE(deflated->isInside(47.95, 15.5));

    //a geometry change drops the prepared shapes
    geometries[0].points[2] = { 49.0, 16.0 };
    geometries[0].points[3] = { 49.0, 15.0 };

    CHECK(cache.setLayer("L1", geometries));

    auto plain2 = cache.prepared("L1", 0.0);
    REQUIRE(plain2);
    CHECK(plain2 != plain);
    CHECK(plain2->isInside(48.5, 15.5));

    //the old prepared layer stays usable for a holder
    CHECK_FALSE(plain->isInside(48.5, 15.5));
}

TEST_CASE("SectorInsideCache ignores exclusion sectors", "[sector][cache]")
{
    SectorInsideCache cache;

    std::vector<Geometry> geometries = {
        squareSector(1, "S1", 47.0, 48.0, 15.0, 16.0),
        squareSector(2, "X1", 40.0, 41.0, 10.0, 11.0, true)
    };
    cache.setLayer("L1", geometries);

    //bounds without the exclusion sector
    auto bounds = cache.bounds("L1");
    REQUIRE(bounds.has_value());
    CHECK(bounds->lat_min == Approx(47.0));
    CHECK(bounds->lon_min == Approx(15.0));

    auto prepared = cache.prepared("L1", 0.0);
    REQUIRE(prepared);
    CHECK(prepared->numSectors() == 1);
    CHECK_FALSE(prepared->isInside(40.5, 10.5));

    //exclusion sectors only
    cache.setLayer("L2", { squareSector(3, "X2", 40.0, 41.0, 10.0, 11.0, true) });

    CHECK_FALSE(cache.bounds("L2").has_value());

    auto empty = cache.prepared("L2", 0.0);
    REQUIRE(empty);
    CHECK(empty->numSectors() == 0);
    CHECK_FALSE(empty->isInside(40.5, 10.5));

    cache.clear();
    CHECK(cache.layerNames().empty());
    CHECK(cache.prepared("L1", 0.0) == nullptr);
}
