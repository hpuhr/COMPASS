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
            const SectorLayer& sector_layer,
            unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man,
            int num_pos, int num_no_ref, int num_pos_outside, int num_pos_inside, int num_pos_ok, int num_pos_nok,
            std::vector<EvaluationRequirement::PositionMaxDistanceDetail> details)
        : Single("SinglePositionMaxDistance", result_id, requirement, sector_layer, utn, target, eval_man),
          num_pos_(num_pos), num_no_ref_(num_no_ref), num_pos_outside_(num_pos_outside),
          num_pos_inside_(num_pos_inside), num_pos_ok_(num_pos_ok), num_pos_nok_(num_pos_nok),
          details_(details)
    {
        updatePMaxPos();
    }


    void SinglePositionMaxDistance::updatePMaxPos()
    {
        assert (num_no_ref_ <= num_pos_);
        assert (num_pos_ - num_no_ref_ == num_pos_inside_ + num_pos_outside_);
        assert (num_pos_inside_ == num_pos_ok_ + num_pos_nok_);

        if (num_pos_ - num_no_ref_ - num_pos_outside_)
        {
            assert (num_pos_ == num_no_ref_ + num_pos_outside_+ num_pos_ok_ + num_pos_nok_);
            p_max_pos_ = (float)num_pos_nok_/(float)(num_pos_ - num_no_ref_ - num_pos_outside_);
            has_p_max_pos_ = true;

            result_usable_ = true;
        }
        else
        {
            p_max_pos_ = 0;
            has_p_max_pos_ = false;

            result_usable_ = false;
        }

        updateUseFromTarget();
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
            tgt_overview_section.addTable("target_table", 12,
            {"UTN", "Begin", "End", "Callsign", "Target Addr.", "Mode 3/A", "Mode C Min", "Mode C Max",
             "#PosInside", "#PosOK", "#PosNOK", "PNOsK"});

        EvaluationResultsReport::SectionContentTable& target_table = tgt_overview_section.getTable("target_table");

        QVariant pd_var;

        if (has_p_max_pos_)
            pd_var = roundf(p_max_pos_ * 10000.0) / 100.0;

        string utn_req_section_heading = getTargetSectionID();

        target_table.addRow(
        {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
         target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
         target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(),
         target_->modeCMaxStr().c_str(), num_pos_inside_, num_pos_ok_, num_pos_nok_, pd_var}, this, {utn_});

        // add requirement to targets->utn->requirements->group->req

        EvaluationResultsReport::Section& utn_req_section = root_item->getSection(utn_req_section_heading);

        if (!utn_req_section.hasTable("details_overview_table"))
            utn_req_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"});

        EvaluationResultsReport::SectionContentTable& utn_req_table =
                utn_req_section.getTable("details_overview_table");

        utn_req_table.addRow({"Use", "To be used in results", use_}, this);
        utn_req_table.addRow({"#Pos [1]", "Number of updates", num_pos_}, this);
        utn_req_table.addRow({"#NoRef [1]", "Number of updates w/o reference positions", num_no_ref_}, this);
        utn_req_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
        utn_req_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_no_ref_}, this);
        utn_req_table.addRow({"#PosOK [1]", "Number of updates with acceptable distance", num_pos_ok_}, this);
        utn_req_table.addRow({"#PosNOK [1]", "Number of updates with unacceptable distance ", num_pos_nok_}, this);
        utn_req_table.addRow({"PNOK [%]", "Probability of unacceptable position", pd_var}, this);

        // condition
        std::shared_ptr<EvaluationRequirement::PositionMaxDistance> req =
                std::static_pointer_cast<EvaluationRequirement::PositionMaxDistance>(requirement_);
        assert (req);

        string condition = "<= "+String::percentToString(req->maximumProbability() * 100.0);

        utn_req_table.addRow({"Condition", "", condition.c_str()}, this);

        string result {"Unknown"};

        if (has_p_max_pos_)
            result = p_max_pos_ <= req->maximumProbability() ? "Passed" : "Failed";

        utn_req_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);

        // add further details

        if (!utn_req_section.hasTable("details_table"))
            utn_req_section.addTable("details_table", 9,
            {"ToD", "NoRef", "PosInside", "Distance", "PosOK", "#Pos", "#NoRef", "#PosOK", "#PosNOK"});

        EvaluationResultsReport::SectionContentTable& utn_req_details_table =
                utn_req_section.getTable("details_table");

        unsigned int detail_cnt = 0;

        for (auto& rq_det_it : details_)
        {
            //if (rq_det_it.has_ref_pos_)
            utn_req_details_table.addRow(
            {String::timeStringFromDouble(rq_det_it.tod_).c_str(),
             !rq_det_it.has_ref_pos_, rq_det_it.pos_inside_, rq_det_it.distance_, rq_det_it.pos_ok_,
             rq_det_it.num_pos_, rq_det_it.num_no_ref_, rq_det_it.num_pos_ok_, rq_det_it.num_pos_nok_},
                        this, detail_cnt);
            //            else
//                utn_req_details_table.addRow(
//                {String::timeStringFromDouble(rq_det_it.tod_).c_str(), rq_det_it.pos_ok_,
//                 !rq_det_it.has_ref_pos_, rq_det_it.pos_inside_, {},
//                 rq_det_it.num_pos_, rq_det_it.num_no_ref_, rq_det_it.num_pos_ok_, rq_det_it.num_pos_nok_},
//                            this, detail_cnt);

            ++detail_cnt;
        }
        // TODO add requirement description, methods
    }


    bool SinglePositionMaxDistance::hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        if (table.name() == "target_table" && annotation.toUInt() == utn_)
            return true;
        else if (table.name() == "details_table" && annotation.isValid() && annotation.toUInt() < details_.size())
            return true;
        else
            return false;
    }

    std::unique_ptr<nlohmann::json::object_t> SinglePositionMaxDistance::viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {

        assert (hasViewableData(table, annotation));

        if (table.name() == "target_table")
            return eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_);
        else if (table.name() == "details_table" && annotation.isValid())
        {
            unsigned int detail_cnt = annotation.toUInt();

            loginf << "SinglePositionMaxDistance: viewableData: detail_cnt " << detail_cnt;

            std::unique_ptr<nlohmann::json::object_t> viewable_ptr
                    = eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_);
            assert (viewable_ptr);

            const EvaluationRequirement::PositionMaxDistanceDetail& detail = details_.at(detail_cnt);

            (*viewable_ptr)["position_latitude"] = detail.tst_pos_.latitude_;
            (*viewable_ptr)["position_longitude"] = detail.tst_pos_.longitude_;
            (*viewable_ptr)["position_window_latitude"] = 0.02;
            (*viewable_ptr)["position_window_longitude"] = 0.02;
            (*viewable_ptr)["time"] = detail.tod_;

            if (!detail.pos_ok_)
                (*viewable_ptr)["evaluation_results"]["highlight_details"] = vector<unsigned int>{detail_cnt};

            return viewable_ptr;
        }
        else
            return nullptr;
    }

    bool SinglePositionMaxDistance::hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        if (table.name() == "target_table" && annotation.toUInt() == utn_)
            return true;
        else
            return false;;
    }

    std::string SinglePositionMaxDistance::reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        assert (hasReference(table, annotation));

        return "Report:Results:"+getTargetSectionID();
    }

    int SinglePositionMaxDistance::numPosOutside() const
    {
        return num_pos_outside_;
    }

    int SinglePositionMaxDistance::numPosInside() const
    {
        return num_pos_inside_;
    }

    std::shared_ptr<Joined> SinglePositionMaxDistance::createEmptyJoined(const std::string& result_id)
    {
        return make_shared<JoinedPositionMaxDistance> (result_id, requirement_, sector_layer_, eval_man_);
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
