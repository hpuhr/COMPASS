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

#include "eval/results/position/distancesingle.h"
#include "eval/results/position/distancejoined.h"

#include "evaluationtargetdata.h"
#include "evaluationmanager.h"

#include "logger.h"

namespace EvaluationRequirementResult
{

/**
*/
SinglePositionDistance::SinglePositionDistance(const std::string& result_id, 
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
                                               unsigned int num_comp_passed,
                                               unsigned int num_comp_failed,
                                               const std::vector<double>& values)
:   SinglePositionProbabilityBase("SinglePositionDistance", result_id, requirement, sector_layer, utn, target, eval_man, details,
                                  num_pos, num_no_ref,num_pos_outside, num_pos_inside, num_comp_passed, num_comp_failed, values)
{
    updateResult();
}

/**
*/
std::shared_ptr<Joined> SinglePositionDistance::createEmptyJoined(const std::string& result_id)
{
    return std::make_shared<JoinedPositionDistance> (result_id, requirement_, sector_layer_, eval_man_);
}
/**
*/
std::vector<std::string> SinglePositionDistance::targetTableHeadersCustom() const
{
    return { "DMin", "DMax", "DAvg", "DSDev", "#CF", "#CP" };
}

/**
*/
std::vector<QVariant> SinglePositionDistance::targetTableValuesCustom() const
{
    return { formatValue(value_min_),
             formatValue(value_max_), 
             formatValue(value_avg_),
             formatValue(std::sqrt(value_var_)),
             num_failed_,
             num_passed_ }; 
}

/**
*/
std::vector<Single::TargetInfo> SinglePositionDistance::targetInfos() const
{
    return { { "#Pos [1]"       , "Number of updates"                        , num_pos_                           }, 
             { "#NoRef [1]"     , "Number of updates w/o reference positions", num_no_ref_                        },
             { "#PosInside [1]" , "Number of updates inside sector"          , num_pos_inside_                    },
             { "#PosOutside [1]", "Number of updates outside sector"         , num_pos_outside_                   }, 
             { "DMin [m]"       , "Minimum of distance"                      , formatValue(value_min_)            }, 
             { "DMax [m]"       , "Maximum of distance"                      , formatValue(value_max_)            },
             { "DAvg [m]"       , "Average of distance"                      , formatValue(value_avg_)            }, 
             { "DSDev [m]"      , "Standard Deviation of distance"           , formatValue(std::sqrt(value_var_)) }, 
             { "DVar [m^2]"     , "Variance of distance"                     , formatValue(value_var_)            }, 
             { "#CF [1]"        , "Number of updates with failed comparison" , num_failed_                        }, 
             { "#CP [1]"        , "Number of updates with passed comparison" , num_passed_                        } };
}

/**
*/
std::vector<std::string> SinglePositionDistance::detailHeaders() const
{
    return { "ToD", "NoRef", "PosInside", "Distance", "CP", "#CF", "#CP", "Comment" };
}

/**
*/
std::vector<QVariant> SinglePositionDistance::detailValues(const EvaluationDetail& detail,
                                                           const EvaluationDetail* parent_detail) const
{
    bool has_ref_pos = detail.numPositions() >= 2;

    return { Utils::Time::toString(detail.timestamp()).c_str(),
            !has_ref_pos,
             detail.getValue(SinglePositionBaseCommon::DetailKey::PosInside),
             detail.getValue(SinglePositionBaseCommon::DetailKey::Value),
             detail.getValue(SinglePositionBaseCommon::DetailKey::CheckPassed), 
             detail.getValue(SinglePositionBaseCommon::DetailKey::NumCheckFailed), 
             detail.getValue(SinglePositionBaseCommon::DetailKey::NumCheckPassed), 
             detail.comments().generalComment().c_str() }; 
}

}
