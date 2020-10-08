#include "eval/results/position/single.h"
#include "eval/results/position/joined.h"
#include "eval/requirement/base.h"
#include "eval/requirement/position/positionmaxdistance.h"
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

SinglePositionMaxDistance::SinglePositionMaxDistance(
        const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
        unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man,
        int num_pos, int num_no_ref, int num_pos_ok, int num_pos_nok)
    : Single("SinglePositionMaxDistance", result_id, requirement, utn, target, eval_man),
      num_pos_(num_pos), num_no_ref_(num_no_ref), num_pos_ok_(num_pos_ok), num_pos_nok_(num_pos_nok)
{
    updatePMaxPos();
}


void SinglePositionMaxDistance::updatePMaxPos()
{
    if (num_pos_)
    {
        assert (num_pos_ == num_no_ref_ + num_pos_ok_ + num_pos_nok_);
        p_max_pos_ = num_pos_nok_/(num_pos_ - num_no_ref_);
        has_p_max_pos_ = true;
    }
    else
    {
        p_max_pos_ = 0;
        has_p_max_pos_ = false;
    }
}

void SinglePositionMaxDistance::print()
{
    std::shared_ptr<EvaluationRequirement::PositionMaxDistance> req =
            std::static_pointer_cast<EvaluationRequirement::PositionMaxDistance>(requirement_);
    assert (req);

    if (num_pos_)
        loginf << "SinglePositionMaxDistance: print: req. name " << req->name()
               << " utn " << utn_
               << " pd " << String::percentToString(100.0 * p_max_pos_)
               << " passed " << (p_max_pos_ <= req->maximumProbability());
    else
        loginf << "SinglePositionMaxDistance: print: req. name " << req->name()
               << " utn " << utn_ << " has no data";
}

void SinglePositionMaxDistance::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
//    logdbg << "SinglePositionMaxDistance " <<  requirement_->name() <<": addToReport";

//    // add target to requirements->group->req

//    logdbg << "SinglePositionMaxDistance " <<  requirement_->name() << ": addToReport: adding single result";

//    EvaluationResultsReport::Section& tgt_overview_section = getRequirementSection(root_item);

//    if (!tgt_overview_section.hasTable("target_table"))
//        tgt_overview_section.addTable("target_table", 13,
//        {"UTN", "Begin", "End", "Callsign", "Target Addr.", "Mode 3/A", "Mode C Min", "Mode C Max",
//         "EUIs", "MUIs", "MGUIs", "NRUIs", "PD"});

//    EvaluationResultsReport::SectionContentTable& target_table = tgt_overview_section.getTable("target_table");

//    QVariant pd_var;

//    if (has_pd_)
//        pd_var = roundf(pd_ * 10000.0) / 100.0;

//    string utn_req_section_heading =
//            "Details:Targets:"+to_string(utn_)+":"+requirement_->groupName()+":"+requirement_->name();

//    target_table.addRow(
//    {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
//     target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
//     target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(),
//     target_->modeCMaxStr().c_str(), sum_uis_, missed_uis_, max_gap_uis_, no_ref_uis_, pd_var},
//                eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_),
//                "Report:Results:"+utn_req_section_heading);

//    // add requirement to targets->utn->requirements->group->req

//    EvaluationResultsReport::Section& utn_req_section = root_item->getSection(utn_req_section_heading);

//    if (!utn_req_section.hasTable("details_overview_table"))
//        utn_req_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"});

//    EvaluationResultsReport::SectionContentTable& utn_req_table =
//            utn_req_section.getTable("details_overview_table");

//    utn_req_table.addRow({"EUIs", "Expected Update Intervals", sum_uis_});
//    utn_req_table.addRow({"MUIs", "Missed Update Intervals", missed_uis_});
//    utn_req_table.addRow({"MGUIs", "Max. Gap Update Intervals", max_gap_uis_});
//    utn_req_table.addRow({"NRUIs", "No Reference Update Intervals", no_ref_uis_});
//    utn_req_table.addRow({"PD", "Probability of Dectection", pd_var});

//    // condition
//    std::shared_ptr<EvaluationRequirement::Detection> req =
//            std::static_pointer_cast<EvaluationRequirement::Detection>(requirement_);
//    assert (req);

//    string condition = ">= "+String::percentToString(req->minimumProbability() * 100.0);

//    utn_req_table.addRow({"Condition", "", condition.c_str()});

//    string result {"Unknown"};

//    if (has_pd_)
//        result = pd_ >= req->minimumProbability() ? "Passed" : "Failed";

//    utn_req_table.addRow({"Condition Fulfilled", "", result.c_str()});

//    // add further details

//    if (!utn_req_section.hasTable("details_table"))
//        utn_req_section.addTable("details_table", 7,
//        {"ToD", "DToD", "Ref.", "MUI", "MGUI", "NRUI", "Comment"});

//    EvaluationResultsReport::SectionContentTable& utn_req_details_table =
//            utn_req_section.getTable("details_table");

//    for (auto& rq_det_it : details_)
//    {
//        if (rq_det_it.has_d_tod_)
//            utn_req_details_table.addRow(
//            {String::timeStringFromDouble(rq_det_it.tod_).c_str(),
//             String::timeStringFromDouble(rq_det_it.d_tod_).c_str(),
//             rq_det_it.ref_exists_, rq_det_it.missed_uis_,
//             rq_det_it.max_gap_uis_, rq_det_it.no_ref_uis_, rq_det_it.comment_.c_str()});
//        else
//            utn_req_details_table.addRow(
//            {String::timeStringFromDouble(rq_det_it.tod_).c_str(),
//             QVariant(),
//             rq_det_it.ref_exists_, rq_det_it.missed_uis_,
//             rq_det_it.max_gap_uis_, rq_det_it.no_ref_uis_,
//             rq_det_it.comment_.c_str()});
//    }

    // TODO add requirement description, methods
}

std::shared_ptr<Joined> SinglePositionMaxDistance::createEmptyJoined(const std::string& result_id)
{
    return make_shared<JoinedPositionMaxDistance> (result_id, requirement_, eval_man_);
}

int SinglePositionMaxDistance::numPos() const
{
    return num_pos_;
}

int SinglePositionMaxDistance::numNoRef() const
{
    return num_no_ref_;
}

int SinglePositionMaxDistance::numPosOk() const
{
    return num_pos_ok_;
}

int SinglePositionMaxDistance::numPosNOk() const
{
    return num_pos_nok_;
}

}
