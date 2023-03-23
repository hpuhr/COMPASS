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
#include "sector.h"
#include "evaluationtargetposition.h"
#include "json.h"
#include "logger.h"

#include <fstream>

const std::string AirSpace::LowerHeightLayerName = "LowerHeightFilter";
const QColor      AirSpace::DefaultSectorColor   = QColor(255, 255, 0);

/**
 */
AirSpace::AirSpace() = default;

/**
 */
AirSpace::~AirSpace() = default;

/**
 */
void AirSpace::clear()
{
    layers_.clear();
}

/**
 */
std::shared_ptr<SectorLayer> AirSpace::lowerHeightFilterLayer() const
{
    for (const auto& l : layers_)
        if (l->name() == LowerHeightLayerName)
            return l;

    return nullptr;
}

/**
 */
bool AirSpace::readJSON(const std::string& fn,
                        unsigned int base_id)
{
    std::ifstream input_file(fn, std::ifstream::in);

    clear();

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

        std::shared_ptr<SectorLayer> lower_height_layer(new SectorLayer(LowerHeightLayerName));
        std::vector<std::shared_ptr<SectorLayer>> layers;

        unsigned int sec_id = base_id;

        for (auto& j_sector : air_space_sectors.get<nlohmann::json::array_t>())
        {
            if (!j_sector.contains("name") ||
                !j_sector.contains("own_height_min") ||
                !j_sector.contains("own_height_max") ||
                !j_sector.contains("own_volume") ||
                !j_sector.contains("lower_height_only") ||
                !j_sector.contains("used_for_checking") ||
                !j_sector.contains("geographic_points"))
            {
                logerr << "EvaluationManager: importFlightLevelFilter: ill-defined sector, json '" << j_sector.dump(4);
                return false;
            }

            std::string name = j_sector["name"];

            int    own_volume        = std::atoi(j_sector["own_volume"       ].get<std::string>().c_str());
            int    used_for_checking = std::atoi(j_sector["used_for_checking"].get<std::string>().c_str());
            int    lower_height_only = std::atoi(j_sector["lower_height_only"].get<std::string>().c_str());
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

            if (lower_height_only)
            {
                auto sec = std::make_shared<Sector>(sec_id++, 
                                                    name,
                                                    LowerHeightLayerName,
                                                    true, 
                                                    false, 
                                                    DefaultSectorColor, 
                                                    points);
                sec->setMinimumAltitude(own_height_min);

                lower_height_layer->addSector(sec);
            }
            else 
            {
                auto sec = std::make_shared<Sector>(sec_id++, 
                                                    name,
                                                    name,
                                                    true, 
                                                    false, 
                                                    DefaultSectorColor, 
                                                    points);
                sec->setMinimumAltitude(own_height_min);
                sec->setMaximumAltitude(own_height_max);

                auto layer = std::make_shared<SectorLayer>(name);
                layer->addSector(sec);

                layers.push_back(layer);
            }
        }

        if (sec_id == base_id)
        {
            logerr << "EvaluationManager: importFlightLevelFilter: no sectors found";
            return false;
        }

        if (lower_height_layer->size() > 0)
            layers_.push_back(lower_height_layer);

        layers_.insert(layers_.begin(), layers.begin(), layers.end());
    }
    catch (nlohmann::json::exception& e)
    {
        logerr << "EvaluationManager: importFlightLevelFilter: could not load file '" << fn << "': " << e.what();

        clear();

        return false;
    }

    return true;
}

/**
 */
const std::vector<std::shared_ptr<SectorLayer>>& AirSpace::layers() const
{
    return layers_;
}

/**
*/
AirSpace::AboveCheckResult AirSpace::isAbove(const SectorLayer* layer,
                                             const EvaluationTargetPosition& pos,
                                             bool has_ground_bit,
                                             bool ground_bit_set)
{
    assert(layer);

    //check bounds of layer
    auto lat_min_max = layer->getMinMaxLatitude();
    auto lon_min_max = layer->getMinMaxLongitude();

    if (pos.latitude_  < lat_min_max.first  ||
        pos.latitude_  > lat_min_max.second ||
        pos.longitude_ < lon_min_max.first  ||
        pos.longitude_ > lon_min_max.second)
        return AboveCheckResult::OutOfAirspace;

    bool was_inside_xy = false;

    //check individual sectors
    for (auto sector : layer->sectors())
    {
        assert( sector->hasMinimumAltitude());
        assert(!sector->hasMaximumAltitude());

        //not inside of sector area?
        if (!sector->isInside(pos, has_ground_bit, ground_bit_set, Sector::InsideCheckType::XY))
            continue;

        was_inside_xy = true;
        
        //if we are inside a sector and above min altitude => above
        if (sector->isInside(pos, has_ground_bit, ground_bit_set, Sector::InsideCheckType::Z))
            return AboveCheckResult::Above;
    }

    //inside but not above any min height => below
    if (was_inside_xy)
        return AboveCheckResult::Below;

    //not inside any sector?
    return AboveCheckResult::OutOfAirspace;
}
