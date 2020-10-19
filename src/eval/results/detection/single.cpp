#include "eval/results/detection/single.h"
#include "eval/results/detection/joined.h"
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

    SingleDetection::SingleDetection(
            const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            const SectorLayer& sector_layer, unsigned int utn, const EvaluationTargetData* target,
            EvaluationManager& eval_man,
            int sum_uis, int missed_uis, int max_gap_uis, int no_ref_uis,
            std::vector<EvaluationRequirement::DetectionDetail> details)
        : Single("SingleDetection", result_id, requirement, sector_layer, utn, target, eval_man),
          sum_uis_(sum_uis), missed_uis_(missed_uis), max_gap_uis_(max_gap_uis), no_ref_uis_(no_ref_uis),
          details_(details)
    {
        updatePD();
    }


    void SingleDetection::updatePD()
    {
        if (sum_uis_)
        {

            //assert (sum_uis_ > max_gap_uis_ + no_ref_uis_);

            if (sum_uis_ < max_gap_uis_+no_ref_uis_) // TODO error in calc
            {
                std::shared_ptr<EvaluationRequirement::Detection> req =
                        std::static_pointer_cast<EvaluationRequirement::Detection>(requirement_);
                assert (req);

                loginf << "SingleDetection: print: req. name " << req->name()
                       << " utn " << utn_ << " calculation error";

                pd_ = 0;
                has_pd_ = false;

                result_usable_ = false;
            }
            else
            {
                pd_ = 1.0 - ((float)missed_uis_/(float)(sum_uis_ - max_gap_uis_ - no_ref_uis_));
                has_pd_ = true;

                result_usable_ = true;
            }
        }
        else
        {
            pd_ = 0;
            has_pd_ = false;

            result_usable_ = false;
        }

        updateUseFromTarget();
    }

    void SingleDetection::print()
    {
        std::shared_ptr<EvaluationRequirement::Detection> req =
                std::static_pointer_cast<EvaluationRequirement::Detection>(requirement_);
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
        logdbg << "SingleDetection " <<  requirement_->name() <<": addToReport";

        // add target to requirements->group->req

        logdbg << "SingleDetection " <<  requirement_->name() << ": addToReport: adding single result";

        EvaluationResultsReport::Section& tgt_overview_section = getRequirementSection(root_item);

        if (!tgt_overview_section.hasTable("target_table"))
            tgt_overview_section.addTable("target_table", 13,
            {"UTN", "Begin", "End", "Callsign", "Target Addr.", "Mode 3/A", "Mode C Min", "Mode C Max",
             "EUIs", "MUIs", "MGUIs", "NRUIs", "PD"});

        EvaluationResultsReport::SectionContentTable& target_table = tgt_overview_section.getTable("target_table");

        QVariant pd_var;

        if (has_pd_)
            pd_var = roundf(pd_ * 10000.0) / 100.0;

        string utn_req_section_heading = getTargetSectionID();

        target_table.addRow(
        {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
         target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
         target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(),
         target_->modeCMaxStr().c_str(), sum_uis_, missed_uis_, max_gap_uis_, no_ref_uis_, pd_var}, this, {utn_});

        // add requirement to targets->utn->requirements->group->req

        EvaluationResultsReport::Section& utn_req_section = root_item->getSection(utn_req_section_heading);

        if (!utn_req_section.hasTable("details_overview_table"))
            utn_req_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"});

        EvaluationResultsReport::SectionContentTable& utn_req_table =
                utn_req_section.getTable("details_overview_table");

        utn_req_table.addRow({"Use", "To be used in results", use_}, this);
        utn_req_table.addRow({"EUIs [1]", "Expected Update Intervals", sum_uis_}, this);
        utn_req_table.addRow({"MUIs [1]", "Missed Update Intervals", missed_uis_}, this);
        utn_req_table.addRow({"MGUIs [1]", "Max. Gap Update Intervals", max_gap_uis_}, this);
        utn_req_table.addRow({"NRUIs [1]", "No Reference Update Intervals", no_ref_uis_}, this);
        utn_req_table.addRow({"PD [%]", "Probability of Detection", pd_var}, this);

        // condition
        std::shared_ptr<EvaluationRequirement::Detection> req =
                std::static_pointer_cast<EvaluationRequirement::Detection>(requirement_);
        assert (req);

        string condition = ">= "+String::percentToString(req->minimumProbability() * 100.0);

        utn_req_table.addRow({"Condition", "", condition.c_str()}, this);

        string result {"Unknown"};

        if (has_pd_)
            result = pd_ >= req->minimumProbability() ? "Passed" : "Failed";

        utn_req_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);

        // add further details

        if (!utn_req_section.hasTable("details_table"))
            utn_req_section.addTable("details_table", 7,
            {"ToD", "DToD", "Ref.", "MUI", "MGUI", "NRUI", "Comment"});

        EvaluationResultsReport::SectionContentTable& utn_req_details_table =
                utn_req_section.getTable("details_table");

        unsigned int detail_cnt = 0;

        for (auto& rq_det_it : details_)
        {
            if (rq_det_it.d_tod_.isValid())
                utn_req_details_table.addRow(
                {String::timeStringFromDouble(rq_det_it.tod_).c_str(),
                 String::timeStringFromDouble(rq_det_it.d_tod_.toFloat()).c_str(),
                 rq_det_it.ref_exists_, rq_det_it.missed_uis_,
                 rq_det_it.max_gap_uis_, rq_det_it.no_ref_uis_, rq_det_it.comment_.c_str()},
                            this, detail_cnt);
            else
                utn_req_details_table.addRow(
                {String::timeStringFromDouble(rq_det_it.tod_).c_str(),
                 rq_det_it.d_tod_,
                 rq_det_it.ref_exists_, rq_det_it.missed_uis_,
                 rq_det_it.max_gap_uis_, rq_det_it.no_ref_uis_,
                 rq_det_it.comment_.c_str()},
                            this, detail_cnt);

            ++detail_cnt;
        }

        // TODO add requirement description, methods
    }

    bool SingleDetection::hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        if (table.name() == "target_table" && annotation.toUInt() == utn_)
            return true;
        else if (table.name() == "details_table" && annotation.isValid() && annotation.toUInt() < details_.size())
            return true;
        else
            return false;
    }

    std::unique_ptr<nlohmann::json::object_t> SingleDetection::viewableData(
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

            const EvaluationRequirement::DetectionDetail& detail = details_.at(detail_cnt);

            (*viewable_ptr)["position_latitude"] = detail.pos_current_.latitude_;
            (*viewable_ptr)["position_longitude"] = detail.pos_current_.longitude_;
            (*viewable_ptr)["position_window_latitude"] = 0.02;
            (*viewable_ptr)["position_window_longitude"] = 0.02;
            (*viewable_ptr)["time"] = detail.tod_;

            if (detail.miss_occurred_)
                (*viewable_ptr)["evaluation_results"]["highlight_details"] = vector<unsigned int>{detail_cnt};

            return viewable_ptr;
        }
        else
            return nullptr;
    }

    bool SingleDetection::hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        if (table.name() == "target_table" && annotation.toUInt() == utn_)
            return true;
        else
            return false;;
    }

    std::string SingleDetection::reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        assert (hasReference(table, annotation));

        return "Report:Results:"+getTargetSectionID();
    }

    std::shared_ptr<Joined> SingleDetection::createEmptyJoined(const std::string& result_id)
    {
        return make_shared<JoinedDetection> (result_id, requirement_, sector_layer_, eval_man_);
    }

    int SingleDetection::sumUIs() const
    {
        return sum_uis_;
    }

    int SingleDetection::missedUIs() const
    {
        return missed_uis_;
    }

    int SingleDetection::maxGapUIs() const
    {
        return max_gap_uis_;
    }

    int SingleDetection::noRefUIs() const
    {
        return no_ref_uis_;
    }

    std::vector<EvaluationRequirement::DetectionDetail>& SingleDetection::details()
    {
        return details_;
    }

}
