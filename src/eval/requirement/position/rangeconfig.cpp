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

#include "eval/requirement/position/rangeconfig.h"
#include "eval/requirement/position/rangeconfigwidget.h"
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
PositionRangeConfig::PositionRangeConfig(
        const std::string& class_id, const std::string& instance_id,
        Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
    : BaseConfig(class_id, instance_id, group, standard, eval_man)
{
    registerParameter("threshold_value", &threshold_value_, 50.0);
}

PositionRangeConfig::~PositionRangeConfig()
{

}

std::shared_ptr<Base> PositionRangeConfig::createRequirement()
{
    shared_ptr<PositionRange> req = make_shared<PositionRange>(
                name_, short_name_, group_.name(), eval_man_, threshold_value_);

    return req;
}

float PositionRangeConfig::thresholdValue() const
{
    return threshold_value_;
}

void PositionRangeConfig::thresholdValue(float value)
{
    threshold_value_ = value;
}

void PositionRangeConfig::createWidget()
{
    assert (!widget_);
    widget_.reset(new PositionRangeConfigWidget(*this));
    assert (widget_);
}

void PositionRangeConfig::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    Section& section = root_item->getSection("Appendix:Requirements:"+group_.name()+":"+name_);

    section.addTable("req_table", 3, {"Name", "Comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& table = section.getTable("req_table");

    table.addRow({"Threshold Value [m]",
                  "Maximum allowed RMS from test target report to reference",
                  threshold_value_}, nullptr);

}
}
