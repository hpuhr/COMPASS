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

#include "eval/requirement/extra/trackconfig.h"
#include "eval/requirement/extra/track.h"
#include "eval/requirement/extra/trackconfigwidget.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base/base.h"

#include "task/result/report/report.h"
#include "task/result/report/section.h"
#include "task/result/report/sectioncontenttable.h"

#include "stringconv.h"

using namespace Utils;
using namespace std;

namespace EvaluationRequirement
{

ExtraTrackConfig::ExtraTrackConfig(
        const std::string& class_id, const std::string& instance_id,
        Group& group, EvaluationStandard& standard, EvaluationCalculator& calculator)
    : ProbabilityBaseConfig(class_id, instance_id, group, standard, calculator)
{
    registerParameter("min_duration", &min_duration_, 60.0f);
    registerParameter("min_num_updates", &min_num_updates_, 10u);
    registerParameter("ignore_primary_only", &ignore_primary_only_, true);
}

ExtraTrackConfig::~ExtraTrackConfig()
{

}

std::shared_ptr<Base> ExtraTrackConfig::createRequirement()
{
    shared_ptr<ExtraTrack> req = make_shared<ExtraTrack>(
                name_, short_name_, group_.name(), prob_, prob_check_type_, calculator_, min_duration_,
                min_num_updates_, ignore_primary_only_);

    return req;
}

float ExtraTrackConfig::minDuration() const
{
    return min_duration_;
}

void ExtraTrackConfig::minDuration(float value)
{
    min_duration_ = value;
}

unsigned int ExtraTrackConfig::minNumUpdates() const
{
    return min_num_updates_;
}

void ExtraTrackConfig::minNumUpdates(unsigned int value)
{
    min_num_updates_ = value;
}

bool ExtraTrackConfig::ignorePrimaryOnly() const
{
    return ignore_primary_only_;
}

void ExtraTrackConfig::ignorePrimaryOnly(bool value)
{
    ignore_primary_only_ = value;
}

BaseConfigWidget* ExtraTrackConfig::createWidget()
{
    return new ExtraTrackConfigWidget(*this);
}

void ExtraTrackConfig::addToReport (std::shared_ptr<ResultReport::Report> report)
{
    auto& section = report->getSection("Appendix:Requirements:"+group_.name()+":"+name_);

    auto& table = section.addTable("req_table", 3, {"Name", "Comment", "Value"}, false);

    table.addRow({"Probability [1]", "Probability of extra data",
                  roundf(prob_ * 10000.0) / 100.0});
    table.addRow({"Probability Check Type", "",
                  comparisonTypeString(prob_check_type_)});

    table.addRow({"Minimum Duration [s]",
                  "Minimum track duration, requirement result is ignored if less",
                  min_duration_});

    table.addRow({"Minimum Number of Updates [1]",
                  "Minimum number of extra target reports, requirement result is ignored if less",
                  min_num_updates_});

    table.addRow({"Ignore Primary Only",
                  "Requirement result is ignored if target is primary only (has no"
                  " secondary attributes, also not in reference)",
                  String::boolToString(ignore_primary_only_)});
}

}
