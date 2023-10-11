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

#include "eval/requirement/extra/dataconfig.h"
#include "eval/requirement/extra/dataconfigwidget.h"
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

ExtraDataConfig::ExtraDataConfig(
        const std::string& class_id, const std::string& instance_id,
        Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
    : ProbabilityBaseConfig(class_id, instance_id, group, standard, eval_man)
{
    registerParameter("min_duration", &min_duration_, 60.0f);
    registerParameter("min_num_updates", &min_num_updates_, 10u);
    registerParameter("ignore_primary_only", &ignore_primary_only_, true);
}

ExtraDataConfig::~ExtraDataConfig()
{
}

std::shared_ptr<Base> ExtraDataConfig::createRequirement()
{
    shared_ptr<ExtraData> req = make_shared<ExtraData>(
                name_, short_name_, group_.name(), prob_, prob_check_type_, eval_man_, min_duration_,
                min_num_updates_, ignore_primary_only_);

    return req;
}

float ExtraDataConfig::minDuration() const
{
    return min_duration_;
}

void ExtraDataConfig::minDuration(float value)
{
    min_duration_ = value;
}

unsigned int ExtraDataConfig::minNumUpdates() const
{
    return min_num_updates_;
}

void ExtraDataConfig::minNumUpdates(unsigned int value)
{
    min_num_updates_ = value;
}

bool ExtraDataConfig::ignorePrimaryOnly() const
{
    return ignore_primary_only_;
}

void ExtraDataConfig::ignorePrimaryOnly(bool value)
{
    ignore_primary_only_ = value;
}

void ExtraDataConfig::createWidget()
{
    assert (!widget_);
    widget_.reset(new ExtraDataConfigWidget(*this));
    assert (widget_);
}

void ExtraDataConfig::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    Section& section = root_item->getSection("Appendix:Requirements:"+group_.name()+":"+name_);

    section.addTable("req_table", 3, {"Name", "Comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& table = section.getTable("req_table");

    table.addRow({"Probability [1]", "Probability of extra data",
                  roundf(prob_ * 10000.0) / 100.0}, nullptr);
    table.addRow({"Probability Check Type", "",
                  comparisonTypeString(prob_check_type_).c_str()}, nullptr);

    table.addRow({"Minimum Duration [s]",
                  "Minimum track duration, requirement result is ignored if less",
                  min_duration_}, nullptr);

    table.addRow({"Minimum Number of Updates [1]",
                  "Minimum number of extra target reports, requirement result is ignored if less",
                  min_num_updates_}, nullptr);

    table.addRow({"Ignore Primary Only",
                  "Requirement result is ignored if target is primary only (has no"
                  " secondary attributes, also not in reference)",
                  String::boolToString(ignore_primary_only_).c_str()}, nullptr);
}
}
