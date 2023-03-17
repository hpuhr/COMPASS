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

#pragma once

#include "sector.h"

/**
*/
class AirSpaceSector : public Sector
{
public:
    AirSpaceSector(unsigned int id, 
                   const std::string& name, 
                   const std::string& layer_name,
                   double altitude_min,
                   double altitude_max,
                   bool use_for_eval,
                   QColor color, 
                   std::vector<std::pair<double,double>> points);
    AirSpaceSector(unsigned int id,
                   const std::string& name, 
                   const std::string& layer_name);
    virtual ~AirSpaceSector() = default;

    bool usedForEval() const;

    virtual void exclude(bool ok) override;

    virtual void setMinimumAltitude(double value) override;
    virtual void removeMinimumAltitude() override;
    virtual void setMaximumAltitude(double value) override;
    virtual void removeMaximumAltitude() override;

protected:
    virtual bool readJSON_impl(const nlohmann::json& json_obj) override;
    virtual void writeJSON_impl(nlohmann::json& json_obj) const override;

    bool use_for_eval_ = true;
};
