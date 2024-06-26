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

#include "eval/results/position/radarrangesingle.h"
#include "eval/results/position/radarrangejoined.h"

#include "evaluationtargetdata.h"
#include "evaluationmanager.h"

#include "logger.h"

#include <Eigen/Dense>

#include <cassert>

namespace EvaluationRequirementResult
{

/**
*/
SinglePositionRadarRange::SinglePositionRadarRange(const std::string& result_id,
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
                                                   const std::vector<double>& values,
                                                   const std::vector<double>& ref_range_values, 
                                                   const std::vector<double>& tst_range_values)
:   SinglePositionValueBase("SinglePositionRadarRange", result_id, requirement, sector_layer, utn, target, eval_man, details,
                            num_pos, num_no_ref,num_pos_outside, num_pos_inside, num_comp_passed, num_comp_failed, values)
,   ref_range_values_(ref_range_values)
,   tst_range_values_(tst_range_values)
{
    updateResult();
}

/**
*/
std::shared_ptr<Joined> SinglePositionRadarRange::createEmptyJoined(const std::string& result_id)
{
    return std::make_shared<JoinedPositionRadarRange> (result_id, requirement_, sector_layer_, eval_man_);
}

/**
*/
std::vector<std::string> SinglePositionRadarRange::targetTableHeadersCustom() const
{
    return { "DMin", "DMax", "DAvg", "DSDev", "Bias", "Gain", "#CF", "#CP" };
}

/**
*/
std::vector<QVariant> SinglePositionRadarRange::targetTableValuesCustom() const
{
    return { formatValue(value_min_),            // "DMin"
             formatValue(value_max_),            // "DMax"
             formatValue(value_avg_),            // "DAvg"
             formatValue(std::sqrt(value_var_)), // "DSDev"
             range_bias_,
             range_gain_,
             num_failed_,                        // "#DNOK"
             num_passed_ };                      // "#DOK"
}

/**
*/
std::vector<Single::TargetInfo> SinglePositionRadarRange::targetInfos() const
{
    std::vector<Single::TargetInfo> infos = 
        { { "#Pos [1]"       , "Number of updates"                        , num_pos_                           }, 
          { "#NoRef [1]"     , "Number of updates w/o reference positions", num_no_ref_                        },
          { "#PosInside [1]" , "Number of updates inside sector"          , num_pos_inside_                    },
          { "#PosOutside [1]", "Number of updates outside sector"         , num_pos_outside_                   }, 
          { "DMin [m]"       , "Minimum of distance"                      , formatValue(value_min_)            }, 
          { "DMax [m]"       , "Maximum of distance"                      , formatValue(value_max_)            },
          { "DAvg [m]"       , "Average of distance"                      , formatValue(value_avg_)            }, 
          { "DSDev [m]"      , "Standard Deviation of distance"           , formatValue(std::sqrt(value_var_)) }, 
          { "DVar [m^2]"     , "Variance of distance"                     , formatValue(value_var_)            } };

    if (range_bias_.isValid())
        infos.push_back({ "Range Bias [m]", "Range bias (linear estimation)", formatValue(range_bias_.toDouble()) });

    if (range_gain_.isValid())
        infos.push_back({ "Range Gain [1]", "Range gain (linear estimation)", formatValue(range_gain_.toDouble()) });

    infos.push_back({ "#CF [1]", "Number of updates with failed comparison" , num_failed_});
    infos.push_back({ "#CP [1]", "Number of updates with passed comparison" , num_passed_});

    return infos;
}

/**
*/
std::vector<std::string> SinglePositionRadarRange::detailHeaders() const
{
    return { "ToD", "NoRef", "PosInside", "Range", "CP", "#CF", "#CP", "Comment" };
}

/**
*/
std::vector<QVariant> SinglePositionRadarRange::detailValues(const EvaluationDetail& detail,
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

/**
*/
boost::optional<double> SinglePositionRadarRange::computeFinalResultValue() const
{
    assert (values_.size() == ref_range_values_.size() && ref_range_values_.size() == tst_range_values_.size());

    range_gain_ = QVariant();
    range_bias_ = QVariant();

    if (values_.empty())
        return {};

    // linear regression

    unsigned int num_distances = values_.size();

    Eigen::MatrixXd x_mat = Eigen::MatrixXd::Ones(num_distances, 2);
    Eigen::MatrixXd y_mat = Eigen::MatrixXd::Ones(num_distances, 1);

    for (unsigned int cnt=0; cnt < num_distances; ++cnt)
    {
        x_mat(cnt, 0) = tst_range_values_.at(cnt);
        y_mat(cnt, 0) = ref_range_values_.at(cnt);
    }

    Eigen::JacobiSVD<Eigen::MatrixXd> svd;

    svd.compute(x_mat, Eigen::ComputeThinV | Eigen::ComputeThinU);
    Eigen::MatrixXd x = svd.solve(y_mat);

    //loginf << "x " << x;

    range_gain_ = x(0, 0);
    range_bias_ = x(1, 0);

    //@TODO_EVAL
    return 0.0;
}

}
