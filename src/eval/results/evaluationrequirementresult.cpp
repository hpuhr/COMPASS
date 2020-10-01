#include "evaluationrequirementresult.h"
#include "evaluationrequirement.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "logger.h"

#include <sstream>
#include <cassert>

using namespace std;

EvaluationRequirementResult::EvaluationRequirementResult(
        std::shared_ptr<EvaluationRequirement> requirement, std::vector<unsigned int> utns,
        std::vector<const EvaluationTargetData*> targets, EvaluationManager& eval_man)
    : requirement_(requirement), utns_(utns), targets_(targets), eval_man_(eval_man)
{
    assert (utns_.size() == targets_.size());
}

std::shared_ptr<EvaluationRequirement> EvaluationRequirementResult::requirement() const
{
    return requirement_;
}

void EvaluationRequirementResult::join(const std::shared_ptr<EvaluationRequirementResult> other_base)
{
    logdbg << "EvaluationRequirementResult: join";

    assert (other_base->requirement_.get() == requirement_.get());

    utns_.insert(utns_.end(), other_base->utns_.begin(), other_base->utns_.end());
    targets_.insert(targets_.end(), other_base->targets_.begin(), other_base->targets_.end());

    assert (utns_.size() == targets_.size());
}

std::vector<unsigned int> EvaluationRequirementResult::utns() const
{
    return utns_;
}

std::string EvaluationRequirementResult::utnsString() const
{
    ostringstream ss;

    bool first = true;

    for (auto utn : utns_)
    {
        if (first)
        {
            ss << to_string(utn);
            first = false;
        }
        else
            ss << "," << to_string(utn);
    }

    if (!utns_.size()) // no utns
        return "None";
    else
        return ss.str();
}

EvaluationResultsReport::SectionContentTable& EvaluationRequirementResult::getReqOverviewTable (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::Section& ov_sec = root_item->getSection("Overview:Requirements");

    if (!ov_sec.hasTable("req_overview"))
        ov_sec.addTable("req_overview", 5,
        {"Req.", "Group", "Result", "Condition", "Result"});

    return ov_sec.getTable("req_overview");
}

EvaluationResultsReport::Section& EvaluationRequirementResult::getRequirementSection (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    return root_item->getSection("Requirements:"+requirement_->groupName()+":"+requirement_->name());
}

