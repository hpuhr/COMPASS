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

#include "eval/results/position/latencysingle.h"
#include "eval/results/position/latencyjoined.h"

#include "evaluationtargetdata.h"
#include "evaluationmanager.h"

#include "logger.h"

namespace EvaluationRequirementResult
{

/**
*/
SinglePositionLatency::SinglePositionLatency(const std::string& result_id, 
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
                                             unsigned int num_value_nok,
                                             const std::vector<double>& values)
:   SinglePositionProbabilityBase("SinglePositionLatency", result_id, requirement, sector_layer, utn, target, eval_man, details,
                                  num_pos, num_no_ref,num_pos_outside, num_pos_inside, num_value_ok, num_value_nok, values)
{
    updateResult();
}

/**
*/
std::shared_ptr<Joined> SinglePositionLatency::createEmptyJoined(const std::string& result_id)
{
    return std::make_shared<JoinedPositionLatency> (result_id, requirement_, sector_layer_, eval_man_);
}

/**
*/
std::vector<std::string> SinglePositionLatency::targetTableHeadersCustom() const
{
    return { "LTMin", "LTMax", "LTAvg", "LTSDev", "#LTOK", "#LTNOK" };
}

/**
*/
std::vector<QVariant> SinglePositionLatency::targetTableValuesCustom() const
{
    return { formatValue(value_min_),
             formatValue(value_max_), 
             formatValue(value_avg_),
             formatValue(std::sqrt(value_var_)),
             num_passed_, 
             num_failed_ }; 
}

/**
*/
std::vector<Single::TargetInfo> SinglePositionLatency::targetInfos() const
{
    return { { "#Pos [1]"       , "Number of updates"                          , num_pos_                           }, 
             { "#NoRef [1]"     , "Number of updates w/o reference positions"  , num_no_ref_                        },
             { "#PosInside [1]" , "Number of updates inside sector"            , num_pos_inside_                    },
             { "#PosOutside [1]", "Number of updates outside sector"           , num_pos_outside_                   }, 
             { "LTMin [m]"      , "Minimum of latency"                         , formatValue(value_min_)            }, 
             { "LTMax [m]"      , "Maximum of latency"                         , formatValue(value_max_)            },
             { "LTAvg [m]"      , "Average of latency"                         , formatValue(value_avg_)            }, 
             { "LTSDev [m]"     , "Standard Deviation of latency"              , formatValue(std::sqrt(value_var_)) }, 
             { "LTVar [m^2]"    , "Variance of latency"                        , formatValue(value_var_)            }, 
             { "#LTOK [1]"      , "Number of updates with acceptable latency"  , num_passed_                        }, 
             { "#LTNOK [1]"     , "Number of updates with unacceptable latency", num_failed_                        } };
}

/**
*/
std::vector<std::string> SinglePositionLatency::detailHeaders() const
{
    return { "ToD", "NoRef", "PosInside", "DLatency", "DLatencyOK", "#LTOK", "#LTNOK", "Comment" };
}

/**
*/
std::vector<QVariant> SinglePositionLatency::detailValues(const EvaluationDetail& detail,
                                                          const EvaluationDetail* parent_detail) const
{
    bool has_ref_pos = detail.numPositions() >= 2;

    return { Utils::Time::toString(detail.timestamp()).c_str(),
            !has_ref_pos,
             detail.getValue(SinglePositionBaseCommon::DetailKey::PosInside),
             detail.getValue(SinglePositionBaseCommon::DetailKey::Value),
             detail.getValue(SinglePositionBaseCommon::DetailKey::CheckPassed), 
             detail.getValue(SinglePositionBaseCommon::DetailKey::NumCheckPassed), 
             detail.getValue(SinglePositionBaseCommon::DetailKey::NumCheckFailed), 
             detail.comments().generalComment().c_str() }; 
}

}
