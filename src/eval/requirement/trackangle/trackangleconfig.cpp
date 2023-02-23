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

#include "eval/requirement/trackangle/trackangleconfig.h"
#include "eval/requirement/trackangle/trackangleconfigwidget.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base/base.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "stringconv.h"

using namespace Utils;
using namespace EvaluationResultsReport;
using namespace std;


namespace EvaluationRequirement
{
TrackAngleConfig::TrackAngleConfig(
        const std::string& class_id, const std::string& instance_id,
        Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
    : BaseConfig(class_id, instance_id, group, standard, eval_man)
{
    registerParameter("threshold_value", &threshold_value_, 50.0);
    registerParameter("use_percent_if_higher", &use_percent_if_higher_, false);
    registerParameter("threshold_percent", &threshold_percent_, 10.0);

    registerParameter("threshold_value_check_type", (unsigned int*)&threshold_value_check_type_,
                      (unsigned int) COMPARISON_TYPE::LESS_THAN_OR_EQUAL);

    registerParameter("failed_values_of_interest", &failed_values_of_interest_, true);
}

TrackAngleConfig::~TrackAngleConfig()
{

}

std::shared_ptr<Base> TrackAngleConfig::createRequirement()
{
    shared_ptr<TrackAngle> req = make_shared<TrackAngle>(
                name_, short_name_, group_.name(), prob_, prob_check_type_, eval_man_,
                threshold_value_, use_percent_if_higher_, threshold_percent_,
                threshold_value_check_type_, failed_values_of_interest_);

    return req;
}

float TrackAngleConfig::thresholdValue() const
{
    return threshold_value_;
}

void TrackAngleConfig::thresholdValue(float value)
{
    threshold_value_ = value;
}

COMPARISON_TYPE TrackAngleConfig::thresholdValueCheckType() const
{
    return threshold_value_check_type_;
}

void TrackAngleConfig::thresholdValueCheckType(const COMPARISON_TYPE &type)
{
    threshold_value_check_type_ = type;
}

bool TrackAngleConfig::failedValuesOfInterest() const
{
    return failed_values_of_interest_;
}

void TrackAngleConfig::failedValuesOfInterest(bool value)
{
    failed_values_of_interest_ = value;
}

bool TrackAngleConfig::usePercentIfHigher() const
{
    return use_percent_if_higher_;
}

void TrackAngleConfig::usePercentIfHigher(bool value)
{
    use_percent_if_higher_ = value;
}

float TrackAngleConfig::thresholdPercent() const
{
    return threshold_percent_;
}

void TrackAngleConfig::thresholdPercent(float value)
{
    threshold_percent_ = value;
}

void TrackAngleConfig::createWidget()
{
    assert (!widget_);
    widget_.reset(new TrackAngleConfigWidget(*this));
    assert (widget_);
}

void TrackAngleConfig::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    Section& section = root_item->getSection("Appendix:Requirements:"+group_.name()+":"+name_);

    //   section.addTable("req_table", 3, {"Name", "Comment", "Value"}, false);

    //    EvaluationResultsReport::SectionContentTable& table = section.getTable("req_table");

    //    table.addRow({"Name", "Requirement name", name_.c_str()}, nullptr);
    //    table.addRow({"Short Name", "Requirement short name", short_name_.c_str()}, nullptr);
    //    table.addRow({"Comment", "", comment_.c_str()}, nullptr);

    // prob & check type added in subclass
}
}
