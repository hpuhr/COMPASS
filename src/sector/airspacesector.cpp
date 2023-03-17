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

#include "airspacesector.h"

#include "logger.h"

/**
*/
AirSpaceSector::AirSpaceSector(unsigned int id, 
                               const std::string& name, 
                               const std::string& layer_name,
                               double altitude_min,
                               double altitude_max,
                               bool use_for_eval,
                               QColor color, 
                               std::vector<std::pair<double,double>> points)
:   Sector       (id, name, layer_name, false, false, color, points)
,   use_for_eval_(use_for_eval)
{
    min_altitude_ = altitude_min;
    max_altitude_ = altitude_max;
}

/**
*/
AirSpaceSector::AirSpaceSector(unsigned int id,
                               const std::string& name, 
                               const std::string& layer_name)
:   Sector(id, name, layer_name, false)
{
}

/**
*/
bool AirSpaceSector::usedForEval() const
{
    return use_for_eval_;
}

/**
*/
bool AirSpaceSector::readJSON_impl(const nlohmann::json& json_obj)
{
    assert(json_obj.contains("use_for_eval"));

    use_for_eval_ = json_obj.at("use_for_eval");

    return true;
}

/**
*/
void AirSpaceSector::writeJSON_impl(nlohmann::json& json_obj) const
{
    json_obj["use_for_eval"] = use_for_eval_;
}


void AirSpaceSector::exclude(bool ok)
{
    logerr << "AirSpaceSector: exclude: value may not be altered";
    throw std::runtime_error("AirSpaceSector: exclude: value may not be altered");
}

void AirSpaceSector::setMinimumAltitude(double value)
{
    logerr << "AirSpaceSector: setMinimumAltitude: value may not be altered";
    throw std::runtime_error("AirSpaceSector: setMinimumAltitude: value may not be altered");
}

void AirSpaceSector::removeMinimumAltitude()
{
    logerr << "AirSpaceSector: removeMinimumAltitude: value may not be altered";
    throw std::runtime_error("AirSpaceSector: removeMinimumAltitude: value may not be altered");
}

void AirSpaceSector::setMaximumAltitude(double value)
{
    logerr << "AirSpaceSector: setMaximumAltitude: value may not be altered";
    throw std::runtime_error("AirSpaceSector: setMaximumAltitude: value may not be altered");
}

void AirSpaceSector::removeMaximumAltitude()
{
    logerr << "AirSpaceSector: removeMaximumAltitude: value may not be altered";
    throw std::runtime_error("AirSpaceSector: removeMaximumAltitude: value may not be altered");
}
