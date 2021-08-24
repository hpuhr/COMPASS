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

#include "dubioustrackconfig.h"
#include "dubioustrackconfigwidget.h"
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
DubiousTrackConfig::DubiousTrackConfig(const std::string& class_id, const std::string& instance_id,
                                       Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
    : BaseConfig(class_id, instance_id, group, standard, eval_man)
{
    prob_check_type_ = COMPARISON_TYPE::LESS_THAN_OR_EQUAL;

    registerParameter("mark_primary_only", &mark_primary_only_, true);

    registerParameter("use_min_updates", &use_min_updates_, true);
    registerParameter("min_updates", &min_updates_, 10);

    registerParameter("use_min_duration", &use_min_duration_, true);
    registerParameter("min_duration", &min_duration_, 30.0);
}

DubiousTrackConfig::~DubiousTrackConfig()
{

}

bool DubiousTrackConfig::markPrimaryOnly() const
{
    return mark_primary_only_;
}

void DubiousTrackConfig::markPrimaryOnly(bool mark_primary_only)
{
    mark_primary_only_ = mark_primary_only;
}

bool DubiousTrackConfig::useMinUpdates() const
{
    return use_min_updates_;
}

void DubiousTrackConfig::useMinUpdates(bool use_min_updates)
{
    use_min_updates_ = use_min_updates;
}

unsigned int DubiousTrackConfig::minUpdates() const
{
    return min_updates_;
}

void DubiousTrackConfig::minUpdates(unsigned int min_updates)
{
    min_updates_ = min_updates;
}

bool DubiousTrackConfig::useMinDuration() const
{
    return use_min_duration_;
}

void DubiousTrackConfig::useMinDuration(bool use_min_duration)
{
    use_min_duration_ = use_min_duration;
}

float DubiousTrackConfig::minDuration() const
{
    return min_duration_;
}

void DubiousTrackConfig::minDuration(float min_duration)
{
    min_duration_ = min_duration;
}

std::shared_ptr<Base> DubiousTrackConfig::createRequirement()
{
    shared_ptr<DubiousTrack> req = make_shared<DubiousTrack>(
                name_, short_name_, group_.name(),
                mark_primary_only_, use_min_updates_, min_updates_,
                use_min_duration_, min_duration_, prob_, COMPARISON_TYPE::LESS_THAN_OR_EQUAL, eval_man_);

    return req;
}

void DubiousTrackConfig::createWidget()
{
    assert (!widget_);
    widget_.reset(new DubiousTrackConfigWidget(*this));
    assert (widget_);
}

void DubiousTrackConfig::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
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
