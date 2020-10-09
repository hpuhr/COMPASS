#include "eval/results/position/single.h"
#include "eval/results/position/joined.h"
#include "eval/requirement/base.h"
#include "eval/requirement/detection/detection.h"
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

JoinedPositionMaxDistance::JoinedPositionMaxDistance(
        const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
        EvaluationManager& eval_man)
    : Joined("JoinedPositionMaxDistance", result_id, requirement, eval_man)
{
}


void JoinedPositionMaxDistance::join(std::shared_ptr<Base> other)
{
    Joined::join(other);

    std::shared_ptr<SinglePositionMaxDistance> other_sub =
            std::static_pointer_cast<SinglePositionMaxDistance>(other);
    assert (other_sub);

    num_pos_ += other_sub->numPos();
    num_no_ref_ += other_sub->numNoRef();
    num_pos_ok_ += other_sub->numPosOk();
    num_pos_nok_ += other_sub->numPosNOk();

    updatePMaxPos();
}

void JoinedPositionMaxDistance::updatePMaxPos()
{
    assert (num_no_ref_ <= num_pos_);

    if (num_pos_ - num_no_ref_)
    {
        assert (num_pos_ == num_no_ref_ + num_pos_ok_ + num_pos_nok_);
        p_max_pos_ = (float)num_pos_nok_/(float)(num_pos_ - num_no_ref_);
        has_p_max_pos_ = true;
    }
    else
    {
        p_max_pos_ = 0;
        has_p_max_pos_ = false;
    }
}

void JoinedPositionMaxDistance::print()
{
    std::shared_ptr<EvaluationRequirement::PositionMaxDistance> req =
            std::static_pointer_cast<EvaluationRequirement::PositionMaxDistance>(requirement_);
    assert (req);

    if (num_pos_)
        loginf << "JoinedPositionMaxDistance: print: req. name " << req->name()
               << " pd " << String::percentToString(100.0 * p_max_pos_)
               << " passed " << (p_max_pos_ <= req->maximumProbability());
    else
        loginf << "JoinedPositionMaxDistance: print: req. name " << req->name() << " has no data";
}

void JoinedPositionMaxDistance::addToReport (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "JoinedPositionMaxDistance " <<  requirement_->name() <<": addToReport";

    if (!results_.size()) // some data must exist
    {
        logerr << "JoinedPositionMaxDistance " <<  requirement_->name() <<": addToReport: no data";
        return;
    }

    logdbg << "JoinedPositionMaxDistance " <<  requirement_->name() << ": addToReport: adding joined result";

    EvaluationResultsReport::SectionContentTable& ov_table = getReqOverviewTable(root_item);

    // condition
    std::shared_ptr<EvaluationRequirement::PositionMaxDistance> req =
            std::static_pointer_cast<EvaluationRequirement::PositionMaxDistance>(requirement_);
    assert (req);

    string condition = "<= "+String::percentToString(req->maximumProbability() * 100.0);

    // pd
    QVariant pd_var;

    string result {"Unknown"};

    if (has_p_max_pos_)
    {
        pd_var = String::percentToString(p_max_pos_ * 100.0).c_str();

        result = p_max_pos_ <= req->maximumProbability() ? "Passed" : "Failed";
    }

    // "Req.", "Group", "Result", "Condition", "Result"
    ov_table.addRow({requirement_->shortname().c_str(), requirement_->groupName().c_str(),
                     pd_var, condition.c_str(), result.c_str()},
                    eval_man_.getViewableForEvaluation(req_grp_id_, result_id_),
                    "Report:Results:"+getRequirementSectionID()); // "Report:Results:Overview"
}

}
