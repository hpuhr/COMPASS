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

#include "eval/results/position/along.h"

#include "logger.h"

#include <cassert>

namespace EvaluationRequirementResult
{

/**********************************************************************************************
 * SinglePositionAlong
 **********************************************************************************************/

/**
*/
SinglePositionAlong::SinglePositionAlong(const std::string& result_id, 
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
                                         unsigned int num_value_nok)
:   SinglePositionProbabilityBase("SinglePositionAlong", result_id, requirement, sector_layer, utn, target, eval_man, details,
                                  num_pos, num_no_ref,num_pos_outside, num_pos_inside, num_value_ok, num_value_nok)
{
    updateResult();
}

/**
*/
std::shared_ptr<Joined> SinglePositionAlong::createEmptyJoined(const std::string& result_id)
{
    return std::make_shared<JoinedPositionAlong> (result_id, requirement_, sector_layer_, eval_man_);
}

/**
*/
std::vector<std::string> SinglePositionAlong::targetTableHeadersCustom() const
{
    return { "ALMin", "ALMax", "ALAvg", "ALSDev", "#ALOK", "#ALNOK" };
}

/**
*/
std::vector<QVariant> SinglePositionAlong::targetTableValuesCustom() const
{
    return { formatValue(accumulator_.min()),
             formatValue(accumulator_.max()), 
             formatValue(accumulator_.mean()),
             formatValue(accumulator_.stddev()),
             num_passed_, 
             num_failed_ }; 
}

/**
*/
std::vector<Single::TargetInfo> SinglePositionAlong::targetInfos() const
{
    return { { "#Pos [1]"       , "Number of updates"                                    , num_pos_                           }, 
             { "#NoRef [1]"     , "Number of updates w/o reference positions"            , num_no_ref_                        },
             { "#PosInside [1]" , "Number of updates inside sector"                      , num_pos_inside_                    },
             { "#PosOutside [1]", "Number of updates outside sector"                     , num_pos_outside_                   }, 
             { "ALMin [m]"      , "Minimum of along-track error"                         , formatValue(accumulator_.min())    }, 
             { "ALMax [m]"      , "Maximum of along-track error"                         , formatValue(accumulator_.max())    },
             { "ALAvg [m]"      , "Average of along-track error"                         , formatValue(accumulator_.mean())   }, 
             { "ALSDev [m]"     , "Standard Deviation of along-track error"              , formatValue(accumulator_.stddev()) }, 
             { "ALVar [m^2]"    , "Variance of along-track error"                        , formatValue(accumulator_.var())    }, 
             { "#ALOK [1]"      , "Number of updates with acceptable along-track error"  , num_passed_                        }, 
             { "#ALNOK [1]"     , "Number of updates with unacceptable along-track error", num_failed_                        } };
}

/**
*/
std::vector<std::string> SinglePositionAlong::detailHeaders() const
{
    return { "ToD", "NoRef", "PosInside", "DAlong", "DAlongOK", "#ALOK", "#ALNOK", "Comment" };
}

/**
*/
std::vector<QVariant> SinglePositionAlong::detailValues(const EvaluationDetail& detail,
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

/**********************************************************************************************
 * JoinedPositionAlong
 **********************************************************************************************/

/**
*/
JoinedPositionAlong::JoinedPositionAlong(const std::string& result_id, 
                                         std::shared_ptr<EvaluationRequirement::Base> requirement,
                                         const SectorLayer& sector_layer, 
                                         EvaluationManager& eval_man)
:   JoinedPositionProbabilityBase("JoinedPositionAlong", result_id, requirement, sector_layer, eval_man, "d_along")
{
}

/**
*/
std::vector<Joined::SectorInfo> JoinedPositionAlong::sectorInfos() const
{
    return { { "#Pos [1]"       , "Number of updates"                                    , num_pos_                           }, 
             { "#NoRef [1]"     , "Number of updates w/o reference positions"            , num_no_ref_                        },
             { "#PosInside [1]" , "Number of updates inside sector"                      , num_pos_inside_                    },
             { "#PosOutside [1]", "Number of updates outside sector"                     , num_pos_outside_                   }, 
             { "ALMin [m]"      , "Minimum of along-track error"                         , formatValue(accumulator_.min())    }, 
             { "ALMax [m]"      , "Maximum of along-track error"                         , formatValue(accumulator_.max())    },
             { "ALAvg [m]"      , "Average of along-track error"                         , formatValue(accumulator_.mean())   }, 
             { "ALSDev [m]"     , "Standard Deviation of along-track error"              , formatValue(accumulator_.stddev()) }, 
             { "ALVar [m^2]"    , "Variance of along-track error"                        , formatValue(accumulator_.var())    }, 
             { "#ALOK [1]"      , "Number of updates with acceptable along-track error"  , num_passed_                        }, 
             { "#ALNOK [1]"     , "Number of updates with unacceptable along-track error", num_failed_                        } };
}

}
