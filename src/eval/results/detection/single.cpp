#include "eval/results/detection/single.h"
#include "eval/results/detection/joined.h"
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

SingleDetection::SingleDetection(
        std::shared_ptr<EvaluationRequirement> requirement,
        unsigned int utn, const EvaluationTargetData* target,
        EvaluationManager& eval_man,
        float sum_uis, float missed_uis, float max_gap_uis, float no_ref_uis,
        std::vector<EvaluationRequirementDetectionDetail> details)
    : Single(requirement, utn, target, eval_man),
      sum_uis_(sum_uis), missed_uis_(missed_uis), max_gap_uis_(max_gap_uis), no_ref_uis_(no_ref_uis), details_(details)
{
    updatePD();
}


void SingleDetection::updatePD()
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

//void EvaluationRequirementDetectionResult::join(const std::shared_ptr<EvaluationRequirementResult> other_base)
//{
//    logdbg << "EvaluationRequirementDetectionResult: join";

//    EvaluationRequirementResult::join(other_base);

//    const std::shared_ptr<EvaluationRequirementDetectionResult> other =
//            std::static_pointer_cast<EvaluationRequirementDetectionResult>(other);

//    assert (other);

//    sum_uis_ += other->sum_uis_;
//    missed_uis_ += other->missed_uis_;
//    max_gap_uis_ += other->max_gap_uis_;
//    no_ref_uis_ += other->no_ref_uis_;

//    // details not joined

//    updatePD();
//}

//std::shared_ptr<EvaluationRequirementResult> EvaluationRequirementDetectionResult::copy ()
//{
//    loginf << "EvaluationRequirementDetectionResult: copy";

//    std::shared_ptr<EvaluationRequirementDetectionResult> copy = make_shared<EvaluationRequirementDetectionResult>(
//                requirement_, utns_, targets_, eval_man_, sum_uis_, missed_uis_, max_gap_uis_, no_ref_uis_,
//                std::vector<EvaluationRequirementDetectionDetail>{}); // details not copied
//    copy->updatePD();

//    return copy;
//}

void SingleDetection::print()
{
    std::shared_ptr<EvaluationRequirementDetection> req =
            std::static_pointer_cast<EvaluationRequirementDetection>(requirement_);
    assert (req);

    if (sum_uis_)
        loginf << "SingleDetection: print: req. name " << req->name()
               << " utn " << utn_
               << " pd " << String::percentToString(100.0 * pd_) << " passed " << (pd_ >= req->minimumProbability());
    else
        loginf << "SingleDetection: print: req. name " << req->name()
               << " utn " << utn_ << " has no data";
}

void SingleDetection::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    loginf << "SingleDetection " <<  requirement_->name() <<": addToReport";

    // add to main requirements overview

    // add target to requirements->group->req

    loginf << "SingleDetection " <<  requirement_->name()
           << ": addToReport: adding single result";

    EvaluationResultsReport::Section& tgt_overview_section = getRequirementSection(root_item);

    if (!tgt_overview_section.hasTable("target_table"))
        tgt_overview_section.addTable("target_table", 13,
        {"UTN", "Begin", "End", "Callsign", "Target Addr.", "Mode 3/A", "Mode C Min", "Mode C Max",
         "EUIs", "MUIs", "MGUIs", "NRUIs", "PD"});

    EvaluationResultsReport::SectionContentTable& target_table = tgt_overview_section.getTable("target_table");

    QVariant pd_var;

    if (has_pd_)
        pd_var = roundf(pd_ * 10000.0) / 100.0;

    string utn_req_section_heading =
            "Details:Targets:"+to_string(utn_)+":"+requirement_->groupName()+":"+requirement_->name();

    target_table.addRow(
    {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
     target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
     target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(),
     target_->modeCMaxStr().c_str(), sum_uis_, missed_uis_, max_gap_uis_, no_ref_uis_, pd_var},
                eval_man_.getViewableForUTN(utn_), "Report:Results:"+utn_req_section_heading);

    // add requirement to targets->utn->requirements->group->req

    EvaluationResultsReport::Section& utn_req_section = root_item->getSection(utn_req_section_heading);

    if (!utn_req_section.hasTable("details_overview_table"))
        utn_req_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"});

    EvaluationResultsReport::SectionContentTable& utn_req_table =
            utn_req_section.getTable("details_overview_table");

    utn_req_table.addRow({"EUIs", "Expected Update Intervals", sum_uis_});
    utn_req_table.addRow({"MUIs", "Missed Update Intervals", missed_uis_});
    utn_req_table.addRow({"MGUIs", "Max. Gap Update Intervals", max_gap_uis_});
    utn_req_table.addRow({"NRUIs", "No Reference Update Intervals", no_ref_uis_});
    utn_req_table.addRow({"PD", "Probability of Dectection", pd_var});

    // condition
    std::shared_ptr<EvaluationRequirementDetection> req =
            std::static_pointer_cast<EvaluationRequirementDetection>(requirement_);
    assert (req);

    string condition = ">= "+String::percentToString(req->minimumProbability() * 100.0);

    utn_req_table.addRow({"Condition", "", condition.c_str()});

    string result {"Unknown"};

    if (has_pd_)
        result = pd_ >= req->minimumProbability() ? "Passed" : "Failed";

    utn_req_table.addRow({"Condition Fulfilled", "", result.c_str()});

    // add further details

    if (!utn_req_section.hasTable("details_table"))
        utn_req_section.addTable("details_table", 7,
        {"ToD", "DToD", "Ref.", "MUI", "MGUI", "NRUI", "Comment"});

    EvaluationResultsReport::SectionContentTable& utn_req_details_table =
            utn_req_section.getTable("details_table");

    for (auto& rq_det_it : details_)
    {
        if (rq_det_it.has_d_tod_)
            utn_req_details_table.addRow(
            {String::timeStringFromDouble(rq_det_it.tod_).c_str(),
             String::timeStringFromDouble(rq_det_it.d_tod_).c_str(),
             rq_det_it.ref_exists_, rq_det_it.missed_uis_,
             rq_det_it.max_gap_uis_, rq_det_it.no_ref_uis_, rq_det_it.comment_.c_str()});
        else
            utn_req_details_table.addRow(
            {String::timeStringFromDouble(rq_det_it.tod_).c_str(),
             QVariant(),
             rq_det_it.ref_exists_, rq_det_it.missed_uis_,
             rq_det_it.max_gap_uis_, rq_det_it.no_ref_uis_,
             rq_det_it.comment_.c_str()});
    }

    // TODO add requirement description, methods
}

std::shared_ptr<Joined> SingleDetection::createEmptyJoined()
{
    return make_shared<JoinedDetection> (requirement_, eval_man_);
}

float SingleDetection::sumUIs() const
{
    return sum_uis_;
}

float SingleDetection::missedUIs() const
{
    return missed_uis_;
}

float SingleDetection::maxGapUIs() const
{
    return max_gap_uis_;
}

float SingleDetection::noRefUIs() const
{
    return no_ref_uis_;
}

}
