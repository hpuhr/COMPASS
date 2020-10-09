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
        int num_pos, int num_no_ref, int num_pos_ok, int num_pos_nok,
        std::vector<EvaluationRequirement::PositionMaxDistanceDetail> details)
    : Single("SinglePositionMaxDistance", result_id, requirement, utn, target, eval_man),
      num_pos_(num_pos), num_no_ref_(num_no_ref), num_pos_ok_(num_pos_ok), num_pos_nok_(num_pos_nok),
      details_(details)
{
    updatePMaxPos();
}


void SinglePositionMaxDistance::updatePMaxPos()
{
    assert (num_no_ref_ <= num_pos_);

    if (num_pos_ - num_no_ref_)
    {
        assert (num_pos_ == num_no_ref_ + num_pos_ok_ + num_pos_nok_);
        p_max_pos_ = (float)num_pos_nok_/(float)(num_pos_ - num_no_ref_);
        has_p_max_pos_ = true;

        use_ = target_->use();
    }
    else
    {
        p_max_pos_ = 0;
        has_p_max_pos_ = false;

        use_ = false;
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
    logdbg << "SinglePositionMaxDistance " <<  requirement_->name() <<": addToReport";

    // add target to requirements->group->req

    logdbg << "SinglePositionMaxDistance " <<  requirement_->name() << ": addToReport: adding single result";

    EvaluationResultsReport::Section& tgt_overview_section = getRequirementSection(root_item);

    if (!tgt_overview_section.hasTable("target_table"))
        tgt_overview_section.addTable("target_table", 13,
        {"UTN", "Begin", "End", "Callsign", "Target Addr.", "Mode 3/A", "Mode C Min", "Mode C Max",
         "#Pos", "#NoRef", "#PosOK", "#PosNOK", "PNOK"});

    EvaluationResultsReport::SectionContentTable& target_table = tgt_overview_section.getTable("target_table");

    QVariant pd_var;

    if (has_p_max_pos_)
        pd_var = roundf(p_max_pos_ * 10000.0) / 100.0;

    string utn_req_section_heading =
            "Details:Targets:"+to_string(utn_)+":"+requirement_->groupName()+":"+requirement_->name();

    target_table.addRow(
    {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
     target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
     target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(),
     target_->modeCMaxStr().c_str(), num_pos_, num_no_ref_, num_pos_ok_, num_pos_nok_, pd_var},
                eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_),
                "Report:Results:"+utn_req_section_heading, !use_);

    // add requirement to targets->utn->requirements->group->req

    EvaluationResultsReport::Section& utn_req_section = root_item->getSection(utn_req_section_heading);

    if (!utn_req_section.hasTable("details_overview_table"))
        utn_req_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"});

    EvaluationResultsReport::SectionContentTable& utn_req_table =
            utn_req_section.getTable("details_overview_table");

    utn_req_table.addRow({"Use", "To be used in results", use_});
    utn_req_table.addRow({"#Pos [1]", "Number of updates", num_pos_});
    utn_req_table.addRow({"#NoRef [1]", "Number of updates w/o reference positions ", num_no_ref_});
    utn_req_table.addRow({"#PosOK [1]", "Number of updates with acceptable distance", num_pos_ok_});
    utn_req_table.addRow({"#PosNOK [1]", "Number of updates with unacceptable distance ", num_pos_nok_});
    utn_req_table.addRow({"PNOK [%]", "Probability of unacceptable position", pd_var});

    // condition
    std::shared_ptr<EvaluationRequirement::PositionMaxDistance> req =
            std::static_pointer_cast<EvaluationRequirement::PositionMaxDistance>(requirement_);
    assert (req);

    string condition = "<= "+String::percentToString(req->maximumProbability() * 100.0);

    utn_req_table.addRow({"Condition", "", condition.c_str()});

    string result {"Unknown"};

    if (has_p_max_pos_)
        result = p_max_pos_ <= req->maximumProbability() ? "Passed" : "Failed";

    utn_req_table.addRow({"Condition Fulfilled", "", result.c_str()});

    // add further details

    if (!utn_req_section.hasTable("details_table"))
        utn_req_section.addTable("details_table", 7,
        {"ToD", "NoRef", "Distance", "#Pos", "#NoRef", "#PosOK", "#PosNOK"});

    EvaluationResultsReport::SectionContentTable& utn_req_details_table =
            utn_req_section.getTable("details_table");

    for (auto& rq_det_it : details_)
    {
        if (rq_det_it.has_ref_pos_)
            utn_req_details_table.addRow(
            {String::timeStringFromDouble(rq_det_it.tod_).c_str(),
             !rq_det_it.has_ref_pos_, rq_det_it.distance_,
             rq_det_it.num_pos_, rq_det_it.num_no_ref_, rq_det_it.num_pos_ok_, rq_det_it.num_pos_nok_});
        else
            utn_req_details_table.addRow(
            {String::timeStringFromDouble(rq_det_it.tod_).c_str(),
             !rq_det_it.has_ref_pos_, {},
             rq_det_it.num_pos_, rq_det_it.num_no_ref_, rq_det_it.num_pos_ok_, rq_det_it.num_pos_nok_});
    }
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

std::vector<EvaluationRequirement::PositionMaxDistanceDetail>& SinglePositionMaxDistance::details()
{
    return details_;
}


}
