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

#include "airspace.h"
#include "sectorlayer.h"
#include "airspacesector.h"
#include "evaluationtargetposition.h"
#include "json.h"
#include "logger.h"

#include <fstream>

const std::string AirSpace::LayerName = "air_space_sectors";

/**
 */
AirSpace::AirSpace()
:   layer_(new SectorLayer(LayerName))
{
}

/**
 */
AirSpace::~AirSpace() = default;

/**
 */
void AirSpace::clear()
{
    assert(layer_);

    layer_->clearSectors();
}

/**
 */
bool AirSpace::readJSON(const std::string& fn)
{
    assert(layer_);

    layer_->clearSectors();

    std::ifstream input_file(fn, std::ifstream::in);

    try
    {
        nlohmann::json j = nlohmann::json::parse(input_file);

        if (!j.contains("air_space_sector_definition"))
        {
            logerr << "EvaluationManager: importFlightLevelFilter: file does not contain air_space_sector_definition";
            return false;
        }

        nlohmann::json& air_space_sector_definition = j["air_space_sector_definition"];

        if (!air_space_sector_definition.contains("air_space_sectors"))
        {
            logerr << "EvaluationManager: importFlightLevelFilter: file does not contain air_space_sectors";
            return false;
        }

        nlohmann::json& air_space_sectors = air_space_sector_definition["air_space_sectors"];
        if (!air_space_sectors.is_array())
        {
            logerr << "EvaluationManager: importFlightLevelFilter: air_space_sectors is not an array";
            return false;
        }

        std::vector<std::shared_ptr<Sector>> sectors;

        unsigned int sec_cnt = 0;

        for (auto& j_sector : air_space_sectors.get<nlohmann::json::array_t>())
        {
            if (!j_sector.contains("name") ||
                !j_sector.contains("own_height_min") ||
                !j_sector.contains("own_height_max") ||
                !j_sector.contains("own_volume") ||
                !j_sector.contains("used_for_checking") ||
                !j_sector.contains("geographic_points"))
            {
                logerr << "EvaluationManager: importFlightLevelFilter: ill-defined sector, json '" << j_sector.dump(4);
                return false;
            }

            std::string name = j_sector["name"];

            if (name == "LowerHeightFilter")
                continue;

            int    own_volume        = std::atoi(j_sector["own_volume"       ].get<std::string>().c_str());
            int    used_for_checking = std::atoi(j_sector["used_for_checking"].get<std::string>().c_str());
            double own_height_min    = std::atof(j_sector["own_height_min"   ].get<std::string>().c_str());
            double own_height_max    = std::atof(j_sector["own_height_max"   ].get<std::string>().c_str());

            nlohmann::json& geographic_points = j_sector["geographic_points"];
            if (!geographic_points.is_array())
            {
                logerr << "EvaluationManager: importFlightLevelFilter: geographic_points is not an array";
                return false;
            }

            std::cout << "Scanning points of sector '" << name << "'" << std::endl;

            std::map<int, std::pair<double, double>> ordered_points;
            for (auto& j_gp : geographic_points.get<nlohmann::json::array_t>())
            {
                if (!j_gp.contains("index") ||
                    !j_gp.contains("latitude") ||
                    !j_gp.contains("longitude"))
                {
                    logerr << "EvaluationManager: importFlightLevelFilter: ill-defined geographic point, json '" << j_gp.dump(4);
                    return false;
                }

                int    id  = std::atoi(j_gp["index"    ].get<std::string>().c_str());
                double lat = std::atof(j_gp["latitude" ].get<std::string>().c_str());
                double lon = std::atof(j_gp["longitude"].get<std::string>().c_str());

                if (!ordered_points.insert(std::make_pair(id, std::make_pair(lat, lon))).second)
                {
                    logerr << "EvaluationManager: importFlightLevelFilter: duplicate geographic point ID " << id;
                    return false;
                }
            }

            if (ordered_points.size() < 3)
            {
                logerr << "EvaluationManager: importFlightLevelFilter: sector with " << ordered_points.size() << " point(s) skipped";
                continue;
            }

            std::vector<std::pair<double, double>> points;
            points.reserve(ordered_points.size());

            for (const auto& elem : ordered_points)
                points.push_back(elem.second);

            auto sec = new AirSpaceSector(sec_cnt++, 
                                          name, 
                                          LayerName, 
                                          own_height_min, 
                                          own_height_max, 
                                          used_for_checking, 
                                          QColor(255, 255, 0), 
                                          points);

            sectors.emplace_back(sec);
        }

        if (sectors.empty())
        {
            logerr << "EvaluationManager: importFlightLevelFilter: no sectors found";
            return false;
        }

        for (auto sec : sectors)
            layer_->addSector(sec);
    }
    catch (nlohmann::json::exception& e)
    {
        logerr << "EvaluationManager: importFlightLevelFilter: could not load file '" << fn << "': " << e.what();

        layer_->clearSectors();

        return false;
    }

    return true;
}

/**
 */
const SectorLayer& AirSpace::layer() const
{
    assert(layer_);
    return *layer_;
}

/**
 */
size_t AirSpace::numEvaluationSectors() const
{
    assert(layer_);

    size_t n = 0;

    for (auto sector : layer_->sectors())
    {
        auto as_sector = dynamic_cast<const AirSpaceSector*>(sector.get());
        assert(as_sector);

        if (as_sector->usedForEval())
            ++n;
    }

    return n;
}

/**
*/
AirSpace::InsideCheckResult AirSpace::isInside(const EvaluationTargetPosition& pos,
                                               bool has_ground_bit,
                                               bool ground_bit_set,
                                               bool evaluation_only) const
{
    assert(layer_);

    //check bounds of layer
    auto lat_min_max = layer_->getMinMaxLatitude();
    auto lon_min_max = layer_->getMinMaxLongitude();

    if (pos.latitude_  < lat_min_max.first  ||
        pos.latitude_  > lat_min_max.second ||
        pos.longitude_ < lon_min_max.first  ||
        pos.longitude_ > lon_min_max.second)
        return InsideCheckResult::OutOfAirspace;

    //check individual sectors
    for (auto sector : layer_->sectors())
    {
        auto as_sector = dynamic_cast<const AirSpaceSector*>(sector.get());
        assert(as_sector);

        //only evaluation sectors desired?
        if (evaluation_only && !as_sector->usedForEval())
            continue;

        //not inside of sector area?
        if (!sector->isInside(pos, has_ground_bit, ground_bit_set, Sector::InsideCheckType::XY))
            continue;
        
        //if we are inside a sector we are greedy, the result is either inside or altitude is outside
        if (sector->isInside(pos, has_ground_bit, ground_bit_set, Sector::InsideCheckType::Z))
            return InsideCheckResult::Inside;
        else
            return InsideCheckResult::AltitudeOOR;
    }

    //not inside any sector?
    return InsideCheckResult::OutOfAirspace;
}

/**
*/
void AirSpace::isInside(InsideCheckResult& result_gb,
                        InsideCheckResult& result_no_gb,
                        const EvaluationTargetPosition& pos,
                        bool evaluation_only) const
{
    assert(layer_);

    //check bounds of layer
    auto lat_min_max = layer_->getMinMaxLatitude();
    auto lon_min_max = layer_->getMinMaxLongitude();

    if (pos.latitude_  < lat_min_max.first  ||
        pos.latitude_  > lat_min_max.second ||
        pos.longitude_ < lon_min_max.first  ||
        pos.longitude_ > lon_min_max.second)
    {
        result_gb    = InsideCheckResult::OutOfAirspace;
        result_no_gb = InsideCheckResult::OutOfAirspace;
        return;
    }

    //check individual sectors
    for (auto sector : layer_->sectors())
    {
        auto as_sector = dynamic_cast<const AirSpaceSector*>(sector.get());
        assert(as_sector);

        //only evaluation sectors desired?
        if (evaluation_only && !as_sector->usedForEval())
            continue;

        //not inside of sector area?
        if (!sector->isInside(pos, false, false, Sector::InsideCheckType::XY))
            continue;

        bool inside_gb    = sector->isInside(pos, true , true , Sector::InsideCheckType::Z);
        bool inside_no_gb = sector->isInside(pos, false, false, Sector::InsideCheckType::Z);

        result_gb    = inside_gb    ? InsideCheckResult::Inside : InsideCheckResult::AltitudeOOR;
        result_no_gb = inside_no_gb ? InsideCheckResult::Inside : InsideCheckResult::AltitudeOOR;
        return;
    }

    //not inside any sector?
    result_gb    = InsideCheckResult::OutOfAirspace;
    result_no_gb = InsideCheckResult::OutOfAirspace;
    return;
}
