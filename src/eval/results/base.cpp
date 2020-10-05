#include "eval/results/base.h"
#include "evaluationrequirement.h"
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

Base::Base( std::shared_ptr<EvaluationRequirement> requirement, EvaluationManager& eval_man)
    : requirement_(requirement), eval_man_(eval_man)
{
}

std::shared_ptr<EvaluationRequirement> Base::requirement() const
{
    return requirement_;
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
