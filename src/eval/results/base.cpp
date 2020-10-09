#include "eval/results/base.h"
#include "eval/requirement/base.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "logger.h"

#include <sstream>
#include <cassert>

using namespace std;

namespace EvaluationRequirementResult
{

Base::Base(const std::string& type, const std::string& result_id,
           std::shared_ptr<EvaluationRequirement::Base> requirement, EvaluationManager& eval_man)
    : type_(type), result_id_(result_id), requirement_(requirement), eval_man_(eval_man)
{
    assert (requirement_);
    req_grp_id_ = requirement_->groupName()+":"+requirement_->name();
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

EvaluationResultsReport::SectionContentTable& Base::getReqOverviewTable (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::Section& ov_sec = root_item->getSection("Overview:Requirements");

    if (!ov_sec.hasTable("req_overview"))
        ov_sec.addTable("req_overview", 5,
        {"Req.", "Group", "Result", "Condition", "Result"});

    return ov_sec.getTable("req_overview");
}

std::string Base::getRequirementSectionID ()
{
    return "Requirements:"+requirement_->groupName()+":"+requirement_->name();
}

EvaluationResultsReport::Section& Base::getRequirementSection (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    return root_item->getSection(getRequirementSectionID());
}

}
