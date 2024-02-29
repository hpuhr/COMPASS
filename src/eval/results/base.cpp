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

#include "eval/results/base.h"
#include "eval/requirement/base/base.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
//#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "sectorlayer.h"
//#include "logger.h"
#include "evaluationmanager.h"

#include <sstream>
#include <cassert>

using namespace std;

namespace EvaluationRequirementResult
{

const std::string Base::req_overview_table_name_ {"Results Overview"};

Base::Base(const std::string& type, 
            const std::string& result_id,
            std::shared_ptr<EvaluationRequirement::Base> requirement, 
            const SectorLayer& sector_layer,
            EvaluationManager& eval_man)
:   type_        (type)
,   result_id_   (result_id)
,   requirement_ (requirement)
,   sector_layer_(sector_layer)
,   eval_man_    (eval_man)
{
    assert (requirement_);

    req_grp_id_ = sector_layer_.name() + ":" + requirement_->groupName() + ":" + requirement_->name();
}

Base::~Base() = default;

bool Base::isSingle() const
{
    return (baseType() == BaseType::Single);
}

bool Base::isJoined() const
{
    return (baseType() == BaseType::Joined);
}

std::shared_ptr<EvaluationRequirement::Base> Base::requirement() const
{
    return requirement_;
}

std::string Base::type() const
{
    return type_;
}

std::string Base::resultId() const
{
    return result_id_;
}

std::string Base::reqGrpId() const
{
    return req_grp_id_;
}

bool Base::use() const
{
    return use_;
}

void Base::use(bool use)
{
    use_ = use;
}

bool Base::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    return false;
}

std::unique_ptr<nlohmann::json::object_t> Base::viewableData(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    return nullptr;
}


bool Base::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    return false;
}

std::string Base::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (false);
}

EvaluationResultsReport::SectionContentTable& Base::getReqOverviewTable (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::Section& ov_sec = root_item->getSection("Overview:Results");

    if (!ov_sec.hasTable(req_overview_table_name_))
        ov_sec.addTable(req_overview_table_name_, 8,
        {"Sector Layer", "Group", "Req.", "Id", "#Updates", "Value", "Condition", "Result"});

    //loginf << "UGA '" << req_overview_table_name_ << "'";

    return ov_sec.getTable(req_overview_table_name_);
}

std::string Base::getRequirementSectionID ()
{
    return "Sectors:"+requirement_->groupName()+" "+sector_layer_.name()+":"+result_id_+":"+requirement_->name();
}

std::string Base::getRequirementSumSectionID ()
{
    // hacky
    return "Sectors:"+requirement_->groupName()+" "+sector_layer_.name()+":"+"Sum"+":"+requirement_->name();
}

EvaluationResultsReport::Section& Base::getRequirementSection (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    return root_item->getSection(getRequirementSectionID());
}

/**
*/
size_t Base::numDetails() const
{
    return details_.size();
}

/**
*/
const Base::EvaluationDetails& Base::getDetails() const
{
    return details_;
}

/**
*/
const EvaluationDetail& Base::getDetail(int idx) const
{
    return getDetails().at(idx);
}

/**
*/
void Base::clearDetails()
{
    details_ = {};
}

/**
*/
void Base::setDetails(const EvaluationDetails& details)
{
    details_ = details;
}

/**
*/
void Base::addDetails(const EvaluationDetails& details)
{
    details_.insert(details_.end(), details.begin(), details.end());
}

}
