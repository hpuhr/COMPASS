#include "eval/results/identification/single.h"
#include "eval/results/identification/joined.h"
#include "eval/requirement/base.h"
#include "eval/requirement/identification/identification.h"
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

    SingleIdentification::SingleIdentification(
            const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            const SectorLayer& sector_layer,
            unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man,
            int num_updates, int num_no_ref_pos, int num_no_ref_id, int num_pos_outside, int num_pos_inside,
            int num_unknown_id, int num_correct_id, int num_false_id,
            std::vector<EvaluationRequirement::IdentificationDetail> details)
        : Single("SingleIdentification", result_id, requirement, sector_layer, utn, target, eval_man),
          num_updates_(num_updates), num_no_ref_pos_(num_no_ref_pos), num_no_ref_id_(num_no_ref_id),
          num_pos_outside_(num_pos_outside), num_pos_inside_(num_pos_inside),
          num_unknown_id_(num_unknown_id),
          num_correct_id_(num_correct_id), num_false_id_(num_false_id), details_(details)
    {
        updatePID();
    }

    void SingleIdentification::updatePID()
    {
        assert (num_updates_ - num_no_ref_pos_ == num_pos_inside_ + num_pos_outside_);
        assert (num_pos_inside_ == num_no_ref_id_+num_unknown_id_+num_correct_id_+num_false_id_);

        if (num_correct_id_+num_false_id_)
        {
            pid_ = (float)num_correct_id_/(float)(num_correct_id_+num_false_id_);
            has_pid_ = true;

            result_usable_ = true;
        }
        else
        {
            pid_ = 0;
            has_pid_ = false;

            result_usable_ = false;
        }

        updateUseFromTarget();
    }

    void SingleIdentification::print()
    {
        std::shared_ptr<EvaluationRequirement::Identification> req =
                std::static_pointer_cast<EvaluationRequirement::Identification>(requirement_);
        assert (req);

        if (has_pid_)
            loginf << "SingleIdentification: print: req. name " << req->name()
                   << " utn " << utn_
                   << " pid " << String::percentToString(100.0 * pid_)
                   << " passed " << (pid_ >= req->minimumProbability());
        else
            loginf << "SingleIdentification: print: req. name " << req->name()
                   << " utn " << utn_ << " has no data";
    }

    void SingleIdentification::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        logdbg << "SingleIdentification " <<  requirement_->name() <<": addToReport";

        // add target to requirements->group->req

        logdbg << "SingleIdentification " <<  requirement_->name() << ": addToReport: adding single result";

        EvaluationResultsReport::Section& tgt_overview_section = getRequirementSection(root_item);

        if (!tgt_overview_section.hasTable("target_table"))
            tgt_overview_section.addTable("target_table", 14,
            {"UTN", "Begin", "End", "Callsign", "Target Addr.", "Mode 3/A", "Mode C Min", "Mode C Max",
             "#Up", "#NoRef", "#UID", "#CID", "#FID", "PID"});

        EvaluationResultsReport::SectionContentTable& target_table = tgt_overview_section.getTable("target_table");

        QVariant pd_var;

        if (has_pid_)
            pd_var = roundf(pid_ * 10000.0) / 100.0;

        string utn_req_section_heading = getTargetSectionID();

        target_table.addRow(
        {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
         target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
         target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(), target_->modeCMaxStr().c_str(),
         num_updates_, num_no_ref_pos_+num_no_ref_id_, num_unknown_id_, num_correct_id_, num_false_id_,
         pd_var}, this, {utn_});

        // add requirement to targets->utn->requirements->group->req

        EvaluationResultsReport::Section& utn_req_section = root_item->getSection(utn_req_section_heading);

        if (!utn_req_section.hasTable("details_overview_table"))
            utn_req_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"});

        EvaluationResultsReport::SectionContentTable& utn_req_table =
                utn_req_section.getTable("details_overview_table");

        utn_req_table.addRow({"Use", "To be used in results", use_}, this);
        utn_req_table.addRow({"#Up [1]", "Number of updates", num_updates_}, this);
        utn_req_table.addRow({"#NoRef [1]", "Number of updates w/o reference position or callsign",
                                num_no_ref_pos_+num_no_ref_id_}, this);
        utn_req_table.addRow({"#NoRefPos [1]", "Number of updates w/o reference position ", num_no_ref_pos_}, this);
        utn_req_table.addRow({"#NoRef [1]", "Number of updates w/o reference callsign", num_no_ref_id_}, this);
        utn_req_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
        utn_req_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);
        utn_req_table.addRow({"#UID [1]", "Number of updates unknown identification", num_unknown_id_}, this);
        utn_req_table.addRow({"#CID [1]", "Number of updates with correct identification", num_correct_id_}, this);
        utn_req_table.addRow({"#FID [1]", "Number of updates with false identification", num_false_id_}, this);
        utn_req_table.addRow({"PNOK [%]", "Probability of correct identificaiton", pd_var}, this);

        // condition
        std::shared_ptr<EvaluationRequirement::Identification> req =
                std::static_pointer_cast<EvaluationRequirement::Identification>(requirement_);
        assert (req);

        string condition = ">= "+String::percentToString(req->minimumProbability() * 100.0);

        utn_req_table.addRow({"Condition", "", condition.c_str()}, this);

        string result {"Unknown"};

        if (has_pid_)
            result = pid_ >= req->minimumProbability() ? "Passed" : "Failed";

        utn_req_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);

        // add further details

        if (!utn_req_section.hasTable("details_table"))
            utn_req_section.addTable("details_table", 10,
            {"ToD", "Ref", "#Up", "#NoRef", "#PosInside", "#PosOutside", "#UID", "#CID", "#FID", "Comment"});

        EvaluationResultsReport::SectionContentTable& utn_req_details_table =
                utn_req_section.getTable("details_table");

        unsigned int detail_cnt = 0;

        for (auto& rq_det_it : details_)
        {
            utn_req_details_table.addRow(
            {String::timeStringFromDouble(rq_det_it.tod_).c_str(), rq_det_it.ref_exists_,
             rq_det_it.num_updates_, rq_det_it.num_no_ref_,
             rq_det_it.num_inside_, rq_det_it.num_outside_, rq_det_it.num_unknown_id_,
             rq_det_it.num_correct_id_, rq_det_it.num_false_id_, rq_det_it.comment_.c_str()},
                        this, detail_cnt);

            ++detail_cnt;
        }
        // TODO add requirement description, methods
    }


    bool SingleIdentification::hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        if (table.name() == "target_table" && annotation.toUInt() == utn_)
            return true;
        else if (table.name() == "details_table" && annotation.isValid() && annotation.toUInt() < details_.size())
            return true;
        else
            return false;
    }

    std::unique_ptr<nlohmann::json::object_t> SingleIdentification::viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {

        assert (hasViewableData(table, annotation));

        if (table.name() == "target_table")
            return eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_);
        else if (table.name() == "details_table" && annotation.isValid())
        {
            unsigned int detail_cnt = annotation.toUInt();

            loginf << "SingleIdentification: viewableData: detail_cnt " << detail_cnt;

            std::unique_ptr<nlohmann::json::object_t> viewable_ptr
                    = eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_);
            assert (viewable_ptr);

            const EvaluationRequirement::IdentificationDetail& detail = details_.at(detail_cnt);

            (*viewable_ptr)["position_latitude"] = detail.pos_tst_.latitude_;
            (*viewable_ptr)["position_longitude"] = detail.pos_tst_.longitude_;
            (*viewable_ptr)["position_window_latitude"] = 0.02;
            (*viewable_ptr)["position_window_longitude"] = 0.02;
            (*viewable_ptr)["time"] = detail.tod_;

//            if (!detail.pos_ok_)
//                (*viewable_ptr)["evaluation_results"]["highlight_details"] = vector<unsigned int>{detail_cnt};

            return viewable_ptr;
        }
        else
            return nullptr;
    }

    bool SingleIdentification::hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        if (table.name() == "target_table" && annotation.toUInt() == utn_)
            return true;
        else
            return false;;
    }

    std::string SingleIdentification::reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        assert (hasReference(table, annotation));

        return "Report:Results:"+getTargetSectionID();
    }

    std::shared_ptr<Joined> SingleIdentification::createEmptyJoined(const std::string& result_id)
    {
        return make_shared<JoinedIdentification> (result_id, requirement_, sector_layer_, eval_man_);
    }

    int SingleIdentification::numNoRefPos() const
    {
        return num_no_ref_pos_;
    }

    int SingleIdentification::numNoRefId() const
    {
        return num_no_ref_id_;
    }

    int SingleIdentification::numPosOutside() const
    {
        return num_pos_outside_;
    }

    int SingleIdentification::numPosInside() const
    {
        return num_pos_inside_;
    }

    int SingleIdentification::numUpdates() const
    {
        return num_updates_;
    }

    int SingleIdentification::numUnknownId() const
    {
        return num_unknown_id_;
    }

    int SingleIdentification::numCorrectId() const
    {
        return num_correct_id_;
    }

    int SingleIdentification::numFalseId() const
    {
        return num_false_id_;
    }

    std::vector<EvaluationRequirement::IdentificationDetail>& SingleIdentification::details()
    {
        return details_;
    }


}
