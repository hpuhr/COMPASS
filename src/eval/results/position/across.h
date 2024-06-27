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

#include "eval/results/position/positionbase.h"

namespace EvaluationRequirementResult
{

/**
*/
class SinglePositionAcross : public SinglePositionProbabilityBase
{
public:
    SinglePositionAcross(const std::string& result_id, 
                         std::shared_ptr<EvaluationRequirement::Base> requirement,
                         const SectorLayer& sector_layer,
                         unsigned int utn, 
                         const EvaluationTargetData* target, 
                         EvaluationManager& eval_man,
                         const EvaluationDetails& details,
                         unsigned int num_pos, 
                         unsigned int num_no_ref,
                         unsigned int num_pos_outside, 
                         unsigned int num_pos_inside,
                         unsigned int num_value_ok, 
                         unsigned int num_value_nok);

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) override;
    
protected:
    virtual std::vector<std::string> targetTableHeadersCustom() const override;
    virtual std::vector<QVariant> targetTableValuesCustom() const override;
    virtual std::vector<TargetInfo> targetInfos() const override;
    virtual std::vector<std::string> detailHeaders() const override;
    virtual std::vector<QVariant> detailValues(const EvaluationDetail& detail,
                                               const EvaluationDetail* parent_detail) const override;
};

/**
*/
class JoinedPositionAcross : public JoinedPositionProbabilityBase
{
public:
    JoinedPositionAcross(const std::string& result_id, 
                         std::shared_ptr<EvaluationRequirement::Base> requirement,
                         const SectorLayer& sector_layer, 
                         EvaluationManager& eval_man);
protected:
    virtual std::vector<SectorInfo> sectorInfos() const override;
};

}
