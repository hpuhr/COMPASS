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

#include <string>

namespace Evaluation
{

/**
 */
struct RequirementResultID
{
    RequirementResultID() = default;
    RequirementResultID(const std::string& layer,
                        const std::string& group,
                        const std::string& req)
    :   sec_layer_name(layer)
    ,   req_group_name(group)
    ,   req_name      (req  ) {}

    bool valid() const
    {
        return !sec_layer_name.empty() &&
               !req_group_name.empty() &&
               !req_name.empty();
    }

    std::string sec_layer_name;
    std::string req_group_name;
    std::string req_name;
};

} // namespace Evaluation
