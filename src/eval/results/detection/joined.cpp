#include "eval/results/detection/joined.h"
#include "eval/results/detection/single.h"
#include "evaluationrequirement.h"
#include "evaluationrequirementdetection.h"
#include "evaluationtargetdata.h"
#include "evaluationmanager.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "logger.h"
#include "stringconv.h"

#include <cassert>

using namespace std;
using namespace Utils;

namespace EvaluationRequirementResult
{

JoinedDetection::JoinedDetection(
        std::shared_ptr<EvaluationRequirement> requirement, EvaluationManager& eval_man)
    : Joined("JoinedDetection", requirement, eval_man)
{
}


void JoinedDetection::join(std::shared_ptr<Base> other)
{
    Joined::join(other);

    std::shared_ptr<SingleDetection> other_sub =
            std::static_pointer_cast<SingleDetection>(other);
    assert (other_sub);

    sum_uis_ += other_sub->sumUIs();
    missed_uis_ += other_sub->missedUIs();
    max_gap_uis_ += other_sub->maxGapUIs();
    no_ref_uis_ += other_sub->noRefUIs();

    updatePD();
}

void JoinedDetection::updatePD()
{
    if (sum_uis_)
    {
        assert (sum_uis_ > max_gap_uis_ + no_ref_uis_);
        pd_ = 1.0 - (missed_uis_/(sum_uis_ - max_gap_uis_ - no_ref_uis_));
        has_pd_ = true;
    }
    else
    {
        pd_ = 0;
        has_pd_ = false;
    }
}

void JoinedDetection::print()
{
    std::shared_ptr<EvaluationRequirementDetection> req =
            std::static_pointer_cast<EvaluationRequirementDetection>(requirement_);
    assert (req);

    if (sum_uis_)
        loginf << "JoinedDetection: print: req. name " << req->name()
               << " pd " << String::percentToString(100.0 * pd_) << " passed " << (pd_ >= req->minimumProbability());
    else
        loginf << "JoinedDetection: print: req. name " << req->name()
               << " has no data";
}

void JoinedDetection::addToReport (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "JoinedDetection " <<  requirement_->name() <<": addToReport";

    if (!results_.size()) // some data must exist
    {
        logerr << "JoinedDetection " <<  requirement_->name() <<": addToReport: no data";
        return;
    }

    logdbg << "JoinedDetection " <<  requirement_->name() << ": addToReport: adding joined result";

    EvaluationResultsReport::SectionContentTable& ov_table = getReqOverviewTable(root_item);

    // condition
    std::shared_ptr<EvaluationRequirementDetection> req =
            std::static_pointer_cast<EvaluationRequirementDetection>(requirement_);
    assert (req);

    string condition = ">= "+String::percentToString(req->minimumProbability() * 100.0);

    // pd
    QVariant pd_var;

    string result {"Unknown"};

    if (has_pd_)
    {
        pd_var = String::percentToString(pd_ * 100.0).c_str();

        result = pd_ >= req->minimumProbability() ? "Passed" : "Failed";
    }

    // "Req.", "Group", "Result", "Condition", "Result"
    ov_table.addRow({requirement_->shortname().c_str(), requirement_->groupName().c_str(),
                     pd_var, condition.c_str(), result.c_str()}, nullptr,
                    "Report:Results:"+getRequirementSectionID()); // "Report:Results:Overview"
}

}
