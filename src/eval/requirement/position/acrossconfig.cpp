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

#include "eval/requirement/position/acrossconfig.h"
#include "eval/requirement/position/acrossconfigwidget.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base/base.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"

using namespace EvaluationResultsReport;
using namespace std;


namespace EvaluationRequirement
{
PositionAcrossConfig::PositionAcrossConfig(
        const std::string& class_id, const std::string& instance_id,
        Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
    : BaseConfig(class_id, instance_id, group, standard, eval_man)
{
    registerParameter("max_abs_value", &max_abs_value_, 50.0);
}

PositionAcrossConfig::~PositionAcrossConfig()
{

}

std::shared_ptr<Base> PositionAcrossConfig::createRequirement()
{
    shared_ptr<PositionAcross> req = make_shared<PositionAcross>(
                name_, short_name_, group_.name(), prob_, prob_check_type_, eval_man_, max_abs_value_);

    return req;
}

float PositionAcrossConfig::maxAbsValue() const
{
    return max_abs_value_;
}

void PositionAcrossConfig::maxAbsValue(float value)
{
    max_abs_value_ = value;
}

void PositionAcrossConfig::createWidget()
{
    assert (!widget_);
    widget_.reset(new PositionAcrossConfigWidget(*this));
    assert (widget_);
}

void PositionAcrossConfig::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
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
