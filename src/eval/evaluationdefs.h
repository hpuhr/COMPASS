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

#include "json_fwd.hpp"

namespace EvaluationRequirementResult
{
    class Base;
}

namespace Evaluation
{

/**
 */
struct RequirementResultID
{
    RequirementResultID() = default;
    RequirementResultID(const std::string& r_sec_layer_name,
                        const std::string& r_group_name,
                        const std::string& r_name);

    inline bool operator<(const RequirementResultID& id) const
    {
        return std::tie(sec_layer_name, req_group_name, req_name)
             < std::tie(id.sec_layer_name, id.req_group_name, id.req_name);
    }

    virtual bool valid() const;
    virtual void fromResult(const EvaluationRequirementResult::Base& result);
    virtual nlohmann::json toJSON() const;
    virtual bool fromJSON(const nlohmann::json& j);

    static const std::string FieldLayerName;
    static const std::string FieldGroupName;
    static const std::string FieldReqName;

    std::string sec_layer_name;
    std::string req_group_name;
    std::string req_name;
};

/**
 */
struct RequirementSumResultID : public RequirementResultID
{
    RequirementSumResultID() = default;
    RequirementSumResultID(const std::string& r_sec_layer_name,
                           const std::string& r_group_name,
                           const std::string& r_name,
                           const std::string& r_sum_name);

    inline bool operator<(const RequirementSumResultID& id) const
    {
        return std::tie(sec_layer_name, req_group_name, req_name, req_sum_name)
             < std::tie(id.sec_layer_name, id.req_group_name, id.req_name, id.req_sum_name);
    }

    virtual bool valid() const override;
    virtual void fromResult(const EvaluationRequirementResult::Base& result) override;
    virtual nlohmann::json toJSON() const override;
    virtual bool fromJSON(const nlohmann::json& j) override;

    static const std::string FieldSumName;

    std::string req_sum_name;
};

} // namespace Evaluation
