/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "eval/results/position/alongacrosssingle.h"
#include "eval/results/position/alongacrossjoined.h"
#include "eval/requirement/base.h"
#include "eval/requirement/position/alongacross.h"
#include "evaluationtargetdata.h"
#include "evaluationmanager.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "logger.h"
#include "stringconv.h"
#include "number.h"

#include <cassert>
#include <algorithm>

using namespace std;
using namespace Utils;

namespace EvaluationRequirementResult
{

    SinglePositionAlongAcross::SinglePositionAlongAcross(
            const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            const SectorLayer& sector_layer,
            unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man,
            int num_pos, int num_no_ref, int num_pos_outside, int num_pos_inside, int num_pos_ok, int num_pos_nok,
            double error_min, double error_max, double error_avg,
            std::vector<EvaluationRequirement::PositionAlongAcrossDetail> details)
        : Single("SinglePositionAlongAcross", result_id, requirement, sector_layer, utn, target, eval_man),
          num_pos_(num_pos), num_no_ref_(num_no_ref), num_pos_outside_(num_pos_outside),
          num_pos_inside_(num_pos_inside), num_pos_ok_(num_pos_ok), num_pos_nok_(num_pos_nok),
          error_min_(error_min), error_max_(error_max), error_avg_(error_avg),
          details_(details)
    {
        updatePMinPos();
    }


    void SinglePositionAlongAcross::updatePMinPos()
    {
        assert (num_no_ref_ <= num_pos_);
        assert (num_pos_ - num_no_ref_ == num_pos_inside_ + num_pos_outside_);
        assert (num_pos_inside_ == num_pos_ok_ + num_pos_nok_);

        if (num_pos_ - num_no_ref_ - num_pos_outside_)
        {
            assert (num_pos_ == num_no_ref_ + num_pos_outside_+ num_pos_ok_ + num_pos_nok_);
            p_min_pos_ = (float)num_pos_ok_/(float)(num_pos_ - num_no_ref_ - num_pos_outside_);
            has_p_min_pos_ = true;

            result_usable_ = true;
        }
        else
        {
            p_min_pos_ = 0;
            has_p_min_pos_ = false;

            result_usable_ = false;
        }

        updateUseFromTarget();
    }

//    void SinglePositionAlongAcross::print()
//    {
//        std::shared_ptr<EvaluationRequirement::PositionAlongAcross> req =
//                std::static_pointer_cast<EvaluationRequirement::PositionAlongAcross>(requirement_);
//        assert (req);

//        if (num_pos_)
//            loginf << "SinglePositionAlongAcross: print: req. name " << req->name()
//                   << " utn " << utn_
//                   << " pd " << String::percentToString(100.0 * p_min_pos_)
//                   << " passed " << (p_min_pos_ >= req->minimumProbability());
//        else
//            loginf << "SinglePositionAlongAcross: print: req. name " << req->name()
//                   << " utn " << utn_ << " has no data";
//    }

    void SinglePositionAlongAcross::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        logdbg << "SinglePositionAlongAcross " <<  requirement_->name() <<": addToReport";

        // add target to requirements->group->req
        addTargetToOverviewTable(root_item);

        // add requirement results to targets->utn->requirements->group->req
        addTargetDetailsToReport(root_item);

        // TODO add requirement description, methods
    }

    void SinglePositionAlongAcross::addTargetToOverviewTable(shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        EvaluationResultsReport::Section& tgt_overview_section = getRequirementSection(root_item);

        if (eval_man_.showAdsbInfo())
        {
            if (!tgt_overview_section.hasTable(target_table_name_))
                tgt_overview_section.addTable(target_table_name_, 17,
                {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                 "#POK", "#PNOK", "POK", "EMin", "EMax", "EAvg", "MOPS", "NUCp/NIC", "NACp"}, true, 10);

            addTargetDetailsToTableADSB(tgt_overview_section.getTable(target_table_name_));
        }
        else
        {
            if (!tgt_overview_section.hasTable(target_table_name_))
                tgt_overview_section.addTable(target_table_name_, 14,
                {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                 "#POK", "#PNOK", "POK", "EMin", "EMax", "EAvg"}, true, 10);

            addTargetDetailsToTable(tgt_overview_section.getTable(target_table_name_));
        }

        if (eval_man_.splitResultsByMOPS()) // add to general sum table
        {
            EvaluationResultsReport::Section& sum_section = root_item->getSection(getRequirementSumSectionID());

            if (eval_man_.showAdsbInfo())
            {
                if (!sum_section.hasTable(target_table_name_))
                    sum_section.addTable(target_table_name_, 17,
                    {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                     "#POK", "#PNOK", "POK", "EMin", "EMax", "EAvg", "MOPS", "NUCp/NIC", "NACp"}, true, 10);

                addTargetDetailsToTableADSB(sum_section.getTable(target_table_name_));
            }
            else
            {
                if (!sum_section.hasTable(target_table_name_))
                    sum_section.addTable(target_table_name_, 14,
                    {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                     "#POK", "#PNOK", "POK", "EMin", "EMax", "EAvg"}, true, 10);

                addTargetDetailsToTable(sum_section.getTable(target_table_name_));
            }
        }
    }

    void SinglePositionAlongAcross::addTargetDetailsToTable (EvaluationResultsReport::SectionContentTable& target_table)
    {
        QVariant pd_var;

        if (has_p_min_pos_)
            pd_var = roundf(p_min_pos_ * 10000.0) / 100.0;

        //string utn_req_section_heading = getTargetRequirementSectionID();

        if (has_p_min_pos_)
            target_table.addRow(
            {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
             target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
             target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(),
             target_->modeCMaxStr().c_str(), num_pos_ok_, num_pos_nok_, pd_var,
             Number::round(error_min_,2), Number::round(error_max_,2),
             Number::round(error_avg_,2)}, this, {utn_});
        else
            target_table.addRow(
            {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
             target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
             target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(),
             target_->modeCMaxStr().c_str(), num_pos_ok_, num_pos_nok_, pd_var,
             {},{},{}}, this, {utn_});
    }

    void SinglePositionAlongAcross::addTargetDetailsToTableADSB (
            EvaluationResultsReport::SectionContentTable& target_table)
    {
        QVariant pd_var;

        if (has_p_min_pos_)
            pd_var = roundf(p_min_pos_ * 10000.0) / 100.0;

        //string utn_req_section_heading = getTargetRequirementSectionID();

        if (has_p_min_pos_)
            target_table.addRow(
            {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
             target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
             target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(),
             target_->modeCMaxStr().c_str(), num_pos_ok_, num_pos_nok_, pd_var,
             Number::round(error_min_,2), Number::round(error_max_,2),
             Number::round(error_avg_,2), target_->mopsVersionsStr().c_str(),
             target_->nucpNicStr().c_str(), target_->nacpStr().c_str()}, this, {utn_});
        else
            target_table.addRow(
            {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
             target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
             target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(),
             target_->modeCMaxStr().c_str(), num_pos_ok_, num_pos_nok_, pd_var,
             {},{},{}, target_->mopsVersionsStr().c_str(),
             target_->nucpNicStr().c_str(), target_->nacpStr().c_str()}, this, {utn_});
    }

    void SinglePositionAlongAcross::addTargetDetailsToReport(shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        QVariant pd_var;

        if (has_p_min_pos_)
            pd_var = roundf(p_min_pos_ * 10000.0) / 100.0;

        EvaluationResultsReport::Section& utn_req_section = root_item->getSection(getTargetRequirementSectionID());

        if (!utn_req_section.hasTable("details_overview_table"))
            utn_req_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"}, false);

        EvaluationResultsReport::SectionContentTable& utn_req_table =
                utn_req_section.getTable("details_overview_table");

        addCommonDetails(root_item);

        utn_req_table.addRow({"Use", "To be used in results", use_}, this);
        utn_req_table.addRow({"#Pos [1]", "Number of updates", num_pos_}, this);
        utn_req_table.addRow({"#NoRef [1]", "Number of updates w/o reference positions", num_no_ref_}, this);
        utn_req_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
        utn_req_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);
        utn_req_table.addRow({"#POK [1]", "Number of updates with acceptable distance", num_pos_ok_}, this);
        utn_req_table.addRow({"#PNOK [1]", "Number of updates with unacceptable distance ", num_pos_nok_}, this);
        utn_req_table.addRow({"POK [%]", "Probability of acceptable position", pd_var}, this);
        utn_req_table.addRow({"EMin [m]", "Distance Error minimum", error_min_}, this);
        utn_req_table.addRow({"EMax [m]", "Distance Error maxmimum", error_max_}, this);
        utn_req_table.addRow({"EAvg [m]", "Distance Error average", error_avg_}, this);

        // condition
        std::shared_ptr<EvaluationRequirement::PositionAlongAcross> req =
                std::static_pointer_cast<EvaluationRequirement::PositionAlongAcross>(requirement_);
        assert (req);

        string condition = ">= "+String::percentToString(req->minimumProbability() * 100.0);

        utn_req_table.addRow({"Condition", "", condition.c_str()}, this);

        string result {"Unknown"};

        if (has_p_min_pos_)
            result = p_min_pos_ >= req->minimumProbability() ? "Passed" : "Failed";

        utn_req_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);

        if (has_p_min_pos_ && p_min_pos_ != 1.0)
        {
            utn_req_section.addFigure("target_errors_overview", "Target Errors Overview",
                                      getTargetErrorsViewable());
        }
        else
        {
            utn_req_section.addText("target_errors_overview_no_figure");
            utn_req_section.getText("target_errors_overview_no_figure").addText(
                        "No target errors found, therefore no figure was generated.");
        }

        // add further details
        reportDetails(utn_req_section);
    }

    void SinglePositionAlongAcross::reportDetails(EvaluationResultsReport::Section& utn_req_section)
    {
        if (!utn_req_section.hasTable(tr_details_table_name_))
            utn_req_section.addTable(tr_details_table_name_, 12,
            {"ToD", "NoRef", "PosInside", "Distance", "PosOK", "#Pos", "#NoRef",
             "#PosInside", "#PosOutside", "#PosOK", "#PosNOK", "Comment"});

        EvaluationResultsReport::SectionContentTable& utn_req_details_table =
                utn_req_section.getTable(tr_details_table_name_);

        unsigned int detail_cnt = 0;

        for (auto& rq_det_it : details_)
        {
            utn_req_details_table.addRow(
            {String::timeStringFromDouble(rq_det_it.tod_).c_str(),
             !rq_det_it.has_ref_pos_, rq_det_it.pos_inside_, rq_det_it.distance_, rq_det_it.pos_ok_,
             rq_det_it.num_pos_, rq_det_it.num_no_ref_,
             rq_det_it.num_inside_, rq_det_it.num_outside_, rq_det_it.num_pos_ok_, rq_det_it.num_pos_nok_,
             rq_det_it.comment_.c_str()},
                        this, detail_cnt);

            ++detail_cnt;
        }
    }

    bool SinglePositionAlongAcross::hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
            return true;
        else if (table.name() == tr_details_table_name_ && annotation.isValid() && annotation.toUInt() < details_.size())
            return true;
        else
            return false;
    }

    std::unique_ptr<nlohmann::json::object_t> SinglePositionAlongAcross::viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {

        assert (hasViewableData(table, annotation));

        if (table.name() == target_table_name_)
        {
            return getTargetErrorsViewable();
        }
        else if (table.name() == tr_details_table_name_ && annotation.isValid())
        {
            unsigned int detail_cnt = annotation.toUInt();

            loginf << "SinglePositionAlongAcross: viewableData: detail_cnt " << detail_cnt;

            std::unique_ptr<nlohmann::json::object_t> viewable_ptr
                    = eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_);
            assert (viewable_ptr);

            const EvaluationRequirement::PositionAlongAcrossDetail& detail = details_.at(detail_cnt);

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

    std::unique_ptr<nlohmann::json::object_t> SinglePositionAlongAcross::getTargetErrorsViewable ()
    {
        std::unique_ptr<nlohmann::json::object_t> viewable_ptr = eval_man_.getViewableForEvaluation(
                    utn_, req_grp_id_, result_id_);

        bool has_pos = false;
        double lat_min, lat_max, lon_min, lon_max;

        for (auto& detail_it : details_)
        {
            if (detail_it.pos_ok_)
                continue;

            if (has_pos)
            {
                lat_min = min(lat_min, detail_it.tst_pos_.latitude_);
                lat_max = max(lat_max, detail_it.tst_pos_.latitude_);

                lon_min = min(lon_min, detail_it.tst_pos_.longitude_);
                lon_max = max(lon_max, detail_it.tst_pos_.longitude_);
            }
            else // tst pos always set
            {
                lat_min = detail_it.tst_pos_.latitude_;
                lat_max = detail_it.tst_pos_.latitude_;

                lon_min = detail_it.tst_pos_.longitude_;
                lon_max = detail_it.tst_pos_.longitude_;

                has_pos = true;
            }

            if (detail_it.has_ref_pos_)
            {
                lat_min = min(lat_min, detail_it.ref_pos_.latitude_);
                lat_max = max(lat_max, detail_it.ref_pos_.latitude_);

                lon_min = min(lon_min, detail_it.ref_pos_.longitude_);
                lon_max = max(lon_max, detail_it.ref_pos_.longitude_);
            }
        }

        if (has_pos)
        {
            (*viewable_ptr)["position_latitude"] = (lat_max+lat_min)/2.0;
            (*viewable_ptr)["position_longitude"] = (lon_max+lon_min)/2.0;;

            double lat_w = 1.1*(lat_max-lat_min)/2.0;
            double lon_w = 1.1*(lon_max-lon_min)/2.0;

            if (lat_w < 0.02)
                lat_w = 0.02;

            if (lon_w < 0.02)
                lon_w = 0.02;

            (*viewable_ptr)["position_window_latitude"] = lat_w;
            (*viewable_ptr)["position_window_longitude"] = lon_w;
        }

        return viewable_ptr;
    }

    bool SinglePositionAlongAcross::hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
            return true;
        else
            return false;;
    }

    std::string SinglePositionAlongAcross::reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        assert (hasReference(table, annotation));

        return "Report:Results:"+getTargetRequirementSectionID();
    }

    double SinglePositionAlongAcross::errorMin() const
    {
        return error_min_;
    }

    double SinglePositionAlongAcross::errorMax() const
    {
        return error_max_;
    }

    double SinglePositionAlongAcross::errorAvg() const
    {
        return error_avg_;
    }

    int SinglePositionAlongAcross::numPosOutside() const
    {
        return num_pos_outside_;
    }

    int SinglePositionAlongAcross::numPosInside() const
    {
        return num_pos_inside_;
    }

    std::shared_ptr<Joined> SinglePositionAlongAcross::createEmptyJoined(const std::string& result_id)
    {
        return make_shared<JoinedPositionAlongAcross> (result_id, requirement_, sector_layer_, eval_man_);
    }

    int SinglePositionAlongAcross::numPos() const
    {
        return num_pos_;
    }

    int SinglePositionAlongAcross::numNoRef() const
    {
        return num_no_ref_;
    }

    int SinglePositionAlongAcross::numPosOk() const
    {
        return num_pos_ok_;
    }

    int SinglePositionAlongAcross::numPosNOk() const
    {
        return num_pos_nok_;
    }

    std::vector<EvaluationRequirement::PositionAlongAcrossDetail>& SinglePositionAlongAcross::details()
    {
        return details_;
    }


}
