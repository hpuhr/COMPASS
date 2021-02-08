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

#include "eval/requirement/mode_a/presentconfig.h"
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

ModeAPresentConfig::ModeAPresentConfig(const std::string& class_id, const std::string& instance_id,
                                       Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
    : BaseConfig(class_id, instance_id, group, standard, eval_man)
{
}

std::shared_ptr<Base> ModeAPresentConfig::createRequirement()
{
    shared_ptr<ModeAPresent> req = make_shared<ModeAPresent>(
                name_, short_name_, group_.name(), prob_, prob_check_type_, eval_man_);

    return req;
}

void ModeAPresentConfig::createWidget()
{
    assert (!widget_);
    widget_.reset(new ModeAPresentConfigWidget(*this));
    assert (widget_);
}

void ModeAPresentConfig::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    Section& section = root_item->getSection("Appendix:Requirements:"+group_.name()+":"+name_);

    section.addTable("req_table", 3, {"Name", "Comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& table = section.getTable("req_table");

    table.addRow({"Probability [1]", "Probability of Mode 3/A code present",
                  roundf(prob_ * 10000.0) / 100.0}, nullptr);
    table.addRow({"Probability Check Type", "",
                  comparisonTypeString(prob_check_type_).c_str()}, nullptr);
}
}
