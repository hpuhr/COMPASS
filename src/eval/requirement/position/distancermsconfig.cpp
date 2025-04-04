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

#include "eval/requirement/position/distancermsconfig.h"
#include "eval/requirement/position/distancermsconfigwidget.h"
#include "eval/requirement/position/distancerms.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base/base.h"

#include "task/result/report/report.h"
#include "task/result/report/section.h"
#include "task/result/report/sectioncontenttable.h"

using namespace std;


namespace EvaluationRequirement
{
PositionDistanceRMSConfig::PositionDistanceRMSConfig(
        const std::string& class_id, const std::string& instance_id,
        Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
    : BaseConfig(class_id, instance_id, group, standard, eval_man)
{
    registerParameter("threshold_value", &threshold_value_, 50.0);
}

PositionDistanceRMSConfig::~PositionDistanceRMSConfig()
{
}

std::shared_ptr<Base> PositionDistanceRMSConfig::createRequirement()
{
    shared_ptr<PositionDistanceRMS> req = make_shared<PositionDistanceRMS>(
                name_, short_name_, group_.name(), eval_man_, threshold_value_);

    return req;
}

double PositionDistanceRMSConfig::thresholdValue() const
{
    return threshold_value_;
}

void PositionDistanceRMSConfig::thresholdValue(double value)
{
    threshold_value_ = value;
}

void PositionDistanceRMSConfig::createWidget()
{
    assert (!widget_);
    widget_.reset(new PositionDistanceRMSConfigWidget(*this));
    assert (widget_);
}

void PositionDistanceRMSConfig::addToReport (std::shared_ptr<ResultReport::Report> report)
{
    auto& section = report->getSection("Appendix:Requirements:"+group_.name()+":"+name_);

    auto& table = section.addTable("req_table", 3, {"Name", "Comment", "Value"}, false);

    table.addRow({"Threshold Value [m]",
                  "Maximum allowed RMS from test target report to reference",
                  threshold_value_});

}
}
