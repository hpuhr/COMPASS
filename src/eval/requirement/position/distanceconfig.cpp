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

#include "eval/requirement/position/distanceconfig.h"
#include "eval/requirement/position/distanceconfigwidget.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base/probabilitybase.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "stringconv.h"

using namespace Utils;
using namespace EvaluationResultsReport;
using namespace std;


namespace EvaluationRequirement
{
PositionDistanceConfig::PositionDistanceConfig(
        const std::string& class_id, const std::string& instance_id,
        Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
    : ProbabilityBaseConfig(class_id, instance_id, group, standard, eval_man)
{
    registerParameter("threshold_value", &threshold_value_, 50.0f);
    registerParameter("threshold_value_check_type", (unsigned int*)&threshold_value_check_type_,
                      (unsigned int) COMPARISON_TYPE::LESS_THAN_OR_EQUAL);
    registerParameter("failed_values_of_interest", &failed_values_of_interest_, true);
}

PositionDistanceConfig::~PositionDistanceConfig()
{
}

std::shared_ptr<Base> PositionDistanceConfig::createRequirement()
{
    shared_ptr<PositionDistance> req = make_shared<PositionDistance>(
                name_, short_name_, group_.name(), prob_, prob_check_type_, eval_man_,
                threshold_value_, threshold_value_check_type_, failed_values_of_interest_);

    return req;
}

float PositionDistanceConfig::thresholdValue() const
{
    return threshold_value_;
}

void PositionDistanceConfig::thresholdValue(float value)
{
    threshold_value_ = value;
}

COMPARISON_TYPE PositionDistanceConfig::thresholdValueCheckType() const
{
    return threshold_value_check_type_;
}

void PositionDistanceConfig::thresholdValueCheckType(const COMPARISON_TYPE &type)
{
    threshold_value_check_type_ = type;
}

bool PositionDistanceConfig::failedValuesOfInterest() const
{
    return failed_values_of_interest_;
}

void PositionDistanceConfig::failedValuesOfInterest(bool value)
{
    failed_values_of_interest_ = value;
}

void PositionDistanceConfig::createWidget()
{
    assert (!widget_);
    widget_.reset(new PositionDistanceConfigWidget(*this));
    assert (widget_);
}

void PositionDistanceConfig::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    Section& section = root_item->getSection("Appendix:Requirements:"+group_.name()+":"+name_);

    section.addTable("req_table", 3, {"Name", "Comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& table = section.getTable("req_table");

    table.addRow({"Probability [1]", "Probability of correct/false position",
                  roundf(prob_ * 10000.0) / 100.0}, nullptr);
    table.addRow({"Probability Check Type", "",
                  comparisonTypeString(prob_check_type_).c_str()}, nullptr);

    table.addRow({"Threshold Value [m]",
                  "Minimum/Maximum allowed distance from test target report to reference",
                  threshold_value_}, nullptr);

    table.addRow({"Threshold Value Check Type",
                  "Distance comparison operator with the given threshold",
                  comparisonTypeString(threshold_value_check_type_).c_str()}, nullptr);

    table.addRow({"Failed Values are of Interest",
                  "If the distances of interest are the ones not passing the check",
                  String::boolToString(failed_values_of_interest_).c_str()}, nullptr);
}
}
