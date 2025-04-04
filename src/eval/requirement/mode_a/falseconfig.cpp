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

#include "eval/requirement/mode_a/falseconfig.h"
#include "eval/requirement/mode_a/modeafalseconfigwidget.h"
#include "eval/requirement/mode_a/false.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base/base.h"

#include "task/result/report/report.h"
#include "task/result/report/section.h"
#include "task/result/report/sectioncontenttable.h"

using namespace std;

namespace EvaluationRequirement
{

ModeAFalseConfig::ModeAFalseConfig(const std::string& class_id, const std::string& instance_id,
                                   Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
    : ProbabilityBaseConfig(class_id, instance_id, group, standard, eval_man)
{

}

std::shared_ptr<Base> ModeAFalseConfig::createRequirement()
{
    shared_ptr<ModeAFalse> req = make_shared<ModeAFalse>(
                name_, short_name_, group_.name(), prob_, prob_check_type_, eval_man_);

    return req;
}

void ModeAFalseConfig::createWidget()
{
    assert (!widget_);
    widget_.reset(new ModeAFalseConfigWidget(*this));
    assert (widget_);
}

void ModeAFalseConfig::addToReport (std::shared_ptr<ResultReport::Report> report)
{
    auto& section = report->getSection("Appendix:Requirements:"+group_.name()+":"+name_);

    auto& table = section.addTable("req_table", 3, {"Name", "Comment", "Value"}, false);

    table.addRow({"Probability [1]", "Probability of false Mode 3/A code",
                  roundf(prob_ * 10000.0) / 100.0});
    table.addRow({"Probability Check Type", "",
                  comparisonTypeString(prob_check_type_)});

}

}
