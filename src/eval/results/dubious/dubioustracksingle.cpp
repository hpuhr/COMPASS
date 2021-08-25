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

#include "eval/results/dubious/dubioustracksingle.h"
#include "eval/results/dubious/dubioustrackjoined.h"
#include "eval/requirement/base/base.h"
#include "eval/requirement/dubious/dubioustrack.h"
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

    SingleDubiousTrack::SingleDubiousTrack(
            const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            const SectorLayer& sector_layer,
            unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man,
            unsigned int num_updates,
            unsigned int num_pos_outside, unsigned int num_pos_inside, unsigned int num_tracks,
            unsigned int num_tracks_dubious, std::string dubious_reason,
            EvaluationRequirement::DubiousTrackDetail detail)
        : Single("SingleDubiousTrack", result_id, requirement, sector_layer, utn, target, eval_man),
          num_updates_(num_updates), num_pos_outside_(num_pos_outside),
          num_pos_inside_(num_pos_inside), num_tracks_(num_tracks),
          num_tracks_dubious_(num_tracks_dubious), dubious_reason_(dubious_reason), detail_(detail)
    {
        update();
    }


    void SingleDubiousTrack::update()
    {
        assert (num_updates_ == num_pos_inside_ + num_pos_outside_);
        assert (num_tracks_ >= num_tracks_dubious_);

        //assert (values_.size() == num_comp_failed_+num_comp_passed_);

        //unsigned int num_speeds = values_.size();

        if (num_tracks_)
        {

            p_dubious_ = (float)num_tracks_dubious_/(float)num_tracks_;
            has_p_dubious_ = true;

            result_usable_ = true;
        }
        else
        {
            has_p_dubious_ = false;
            p_dubious_ = 0;

            result_usable_ = false;
        }

        updateUseFromTarget();
    }

    void SingleDubiousTrack::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        logdbg << "SingleDubiousTrack " <<  requirement_->name() <<": addToReport";

        // add target to requirements->group->req
        addTargetToOverviewTable(root_item);

        // add requirement results to targets->utn->requirements->group->req
        addTargetDetailsToReport(root_item);

        // TODO add requirement description, methods
    }

    void SingleDubiousTrack::addTargetToOverviewTable(shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        EvaluationResultsReport::Section& tgt_overview_section = getRequirementSection(root_item);

        if (eval_man_.resultsGenerator().showAdsbInfo())
            addTargetDetailsToTableADSB(tgt_overview_section, target_table_name_);
        else
            addTargetDetailsToTable(tgt_overview_section, target_table_name_);

        if (eval_man_.resultsGenerator().splitResultsByMOPS()) // add to general sum table
        {
            EvaluationResultsReport::Section& sum_section = root_item->getSection(getRequirementSumSectionID());

            if (eval_man_.resultsGenerator().showAdsbInfo())
                addTargetDetailsToTableADSB(sum_section, target_table_name_);
            else
                addTargetDetailsToTable(sum_section, target_table_name_);
        }
    }

    void SingleDubiousTrack::addTargetDetailsToTable (
            EvaluationResultsReport::Section& section, const std::string& table_name)
    {
        if (!section.hasTable(table_name))
        {
            Qt::SortOrder order = Qt::AscendingOrder;

            if(req()->probCheckType() == EvaluationRequirement::COMPARISON_TYPE::LESS_THAN
                    || req()->probCheckType() == EvaluationRequirement::COMPARISON_TYPE::LESS_THAN_OR_EQUAL)
                order = Qt::DescendingOrder;


            section.addTable(table_name, 11,
            {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
             "#T", "#DT", "PDT"}, true, 10, order);
        }

        EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

        QVariant p_dubious_var;

        if (has_p_dubious_)
            p_dubious_var = roundf(p_dubious_ * 10000.0) / 100.0;

        target_table.addRow(
        {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
         target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
         target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(), target_->modeCMaxStr().c_str(),
         num_tracks_, // "#T"
         num_tracks_dubious_, // "#DT"
         p_dubious_var}, // "PDT"
                    this, {utn_});
    }

    void SingleDubiousTrack::addTargetDetailsToTableADSB (
            EvaluationResultsReport::Section& section, const std::string& table_name)
    {
        if (!section.hasTable(table_name))
        {
            Qt::SortOrder order = Qt::AscendingOrder;

            if(req()->probCheckType() == EvaluationRequirement::COMPARISON_TYPE::LESS_THAN
                    || req()->probCheckType() == EvaluationRequirement::COMPARISON_TYPE::LESS_THAN_OR_EQUAL)
                order = Qt::DescendingOrder;

            section.addTable(table_name, 14,
            {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
             "#T", "#DT", "PDT",
             "MOPS", "NUCp/NIC", "NACp"}, true, 10, order);
        }

        EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

        QVariant p_dubious_var;

        if (has_p_dubious_)
            p_dubious_var = roundf(p_dubious_ * 10000.0) / 100.0;

        // "UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
        // "#ACOK", "#ACNOK", "PACOK", "#DOK", "#DNOK", "PDOK", "MOPS", "NUCp/NIC", "NACp"

        target_table.addRow(
        {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
         target_->callsignsStr().c_str(), target_->targetAddressesStr().c_str(),
         target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(),
         target_->modeCMaxStr().c_str(),
         num_tracks_, // "#T"
         num_tracks_dubious_, // "#DT"
         p_dubious_var, // "PDT"
         target_->mopsVersionsStr().c_str(), // "MOPS"
         target_->nucpNicStr().c_str(), // "NUCp/NIC"
         target_->nacpStr().c_str()}, // "NACp"
                    this, {utn_});

    }

    void SingleDubiousTrack::addTargetDetailsToReport(shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        EvaluationResultsReport::Section& utn_req_section = root_item->getSection(getTargetRequirementSectionID());

        if (!utn_req_section.hasTable("details_overview_table"))
            utn_req_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"}, false);

        std::shared_ptr<EvaluationRequirement::DubiousTrack> req =
                std::static_pointer_cast<EvaluationRequirement::DubiousTrack>(requirement_);
        assert (req);

        EvaluationResultsReport::SectionContentTable& utn_req_table =
                utn_req_section.getTable("details_overview_table");

        addCommonDetails(root_item);

        // "UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
        // "#ACOK", "#ACNOK", "PACOK", "#DOK", "#DNOK", "PDOK"

        utn_req_table.addRow({"Use", "To be used in results", use_}, this);
        utn_req_table.addRow({"#Up [1]", "Number of updates", num_updates_}, this);
        utn_req_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
        utn_req_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);

        utn_req_table.addRow({"#T [1]", "Number of tracks", num_tracks_}, this);
        utn_req_table.addRow({"#DT [1]", "Number of dubious tracks", num_tracks_dubious_},
                             this);

        // condition
        {
            QVariant p_dubious_var;

            if (has_p_dubious_)
                p_dubious_ = roundf(p_dubious_ * 10000.0) / 100.0;

            utn_req_table.addRow({"PDT [%]", "Probability of dubious track", p_dubious_var}, this);

            utn_req_table.addRow({"Condition", {}, req->getConditionStr().c_str()}, this);

            string result {"Unknown"};

            if (has_p_dubious_)
                result = req->getResultConditionStr(p_dubious_);

            utn_req_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);
        }

        if (has_p_dubious_ && p_dubious_ != 0.0) // TODO
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

    void SingleDubiousTrack::reportDetails(EvaluationResultsReport::Section& utn_req_section)
    {
//        if (!utn_req_section.hasTable(tr_details_table_name_))
//            utn_req_section.addTable(tr_details_table_name_, 8,
//            {"ToD", "NoRef", "PosInside", "Distance", "CP", "#CF", "#CP", "Comment"});

//        EvaluationResultsReport::SectionContentTable& utn_req_details_table =
//                utn_req_section.getTable(tr_details_table_name_);

//        unsigned int detail_cnt = 0;

//        for (auto& rq_det_it : details_)
//        {
//            utn_req_details_table.addRow(
//            {String::timeStringFromDouble(rq_det_it.tod_).c_str(),
//             !rq_det_it.has_ref_pos_, rq_det_it.pos_inside_,
//             rq_det_it.offset_,  // "Distance"
//             rq_det_it.check_passed_, // CP"
//             rq_det_it.num_check_failed_, // "#CF",
//             rq_det_it.num_check_passed_, // "#CP"
//             rq_det_it.comment_.c_str()}, // "Comment"
//                        this, detail_cnt);

//            ++detail_cnt;
//        }
    }

    bool SingleDubiousTrack::hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
            return true;
//        else if (table.name() == tr_details_table_name_ && annotation.isValid() && annotation.toUInt() < details_.size())
//            return true;
        else
            return false;
    }

    std::unique_ptr<nlohmann::json::object_t> SingleDubiousTrack::viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {

        assert (hasViewableData(table, annotation));

        if (table.name() == target_table_name_)
        {
            return getTargetErrorsViewable();
        }
//        else if (table.name() == tr_details_table_name_ && annotation.isValid())
//        {
//            unsigned int detail_cnt = annotation.toUInt();

//            loginf << "SingleDubiousTrack: viewableData: detail_cnt " << detail_cnt;

//            std::unique_ptr<nlohmann::json::object_t> viewable_ptr
//                    = eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_);
//            assert (viewable_ptr);

//            const EvaluationRequirement::DubiousTrackDetail& detail = details_.at(detail_cnt);

//            (*viewable_ptr)["position_latitude"] = detail.tst_pos_.latitude_;
//            (*viewable_ptr)["position_longitude"] = detail.tst_pos_.longitude_;
//            (*viewable_ptr)["position_window_latitude"] = eval_man_.resultDetailZoom();
//            (*viewable_ptr)["position_window_longitude"] = eval_man_.resultDetailZoom();
//            (*viewable_ptr)["time"] = detail.tod_;

//            if (!detail.check_passed_)
//                (*viewable_ptr)["evaluation_results"]["highlight_details"] = vector<unsigned int>{detail_cnt};

//            return viewable_ptr;
//        }
        else
            return nullptr;
    }

    std::unique_ptr<nlohmann::json::object_t> SingleDubiousTrack::getTargetErrorsViewable ()
    {
        std::unique_ptr<nlohmann::json::object_t> viewable_ptr = eval_man_.getViewableForEvaluation(
                    utn_, req_grp_id_, result_id_);

//        bool has_pos = false;
//        double lat_min, lat_max, lon_min, lon_max;

//        bool failed_values_of_interest = req()->failedValuesOfInterest();

//        for (auto& detail_it : details_)
//        {
//            if ((failed_values_of_interest && detail_it.check_passed_)
//                    || (!failed_values_of_interest && !detail_it.check_passed_))
//                continue;

//            if (has_pos)
//            {
//                lat_min = min(lat_min, detail_it.tst_pos_.latitude_);
//                lat_max = max(lat_max, detail_it.tst_pos_.latitude_);

//                lon_min = min(lon_min, detail_it.tst_pos_.longitude_);
//                lon_max = max(lon_max, detail_it.tst_pos_.longitude_);
//            }
//            else // tst pos always set
//            {
//                lat_min = detail_it.tst_pos_.latitude_;
//                lat_max = detail_it.tst_pos_.latitude_;

//                lon_min = detail_it.tst_pos_.longitude_;
//                lon_max = detail_it.tst_pos_.longitude_;

//                has_pos = true;
//            }

//            if (detail_it.has_ref_pos_)
//            {
//                lat_min = min(lat_min, detail_it.ref_pos_.latitude_);
//                lat_max = max(lat_max, detail_it.ref_pos_.latitude_);

//                lon_min = min(lon_min, detail_it.ref_pos_.longitude_);
//                lon_max = max(lon_max, detail_it.ref_pos_.longitude_);
//            }
//        }

//        if (has_pos)
//        {
//            (*viewable_ptr)["speed_latitude"] = (lat_max+lat_min)/2.0;
//            (*viewable_ptr)["speed_longitude"] = (lon_max+lon_min)/2.0;;

//            double lat_w = 1.1*(lat_max-lat_min)/2.0;
//            double lon_w = 1.1*(lon_max-lon_min)/2.0;

//            if (lat_w < eval_man_.resultDetailZoom())
//                lat_w = eval_man_.resultDetailZoom();

//            if (lon_w < eval_man_.resultDetailZoom())
//                lon_w = eval_man_.resultDetailZoom();

//            (*viewable_ptr)["speed_window_latitude"] = lat_w;
//            (*viewable_ptr)["speed_window_longitude"] = lon_w;
//        }

        return viewable_ptr;
    }

    bool SingleDubiousTrack::hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
            return true;
        else
            return false;;
    }

    std::string SingleDubiousTrack::reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        assert (hasReference(table, annotation));

        return "Report:Results:"+getTargetRequirementSectionID();
    }

    unsigned int SingleDubiousTrack::numPosOutside() const
    {
        return num_pos_outside_;
    }

    unsigned int SingleDubiousTrack::numPosInside() const
    {
        return num_pos_inside_;
    }

    std::shared_ptr<Joined> SingleDubiousTrack::createEmptyJoined(const std::string& result_id)
    {
        return make_shared<JoinedDubiousTrack> (result_id, requirement_, sector_layer_, eval_man_);
    }

    unsigned int SingleDubiousTrack::numUpdates() const
    {
        return num_updates_;
    }

//    std::vector<EvaluationRequirement::DubiousTrackDetail>& SingleDubiousTrack::details()
//    {
//        return details_;
//    }

    EvaluationRequirement::DubiousTrack* SingleDubiousTrack::req ()
    {
        EvaluationRequirement::DubiousTrack* req =
                dynamic_cast<EvaluationRequirement::DubiousTrack*>(requirement_.get());
        assert (req);
        return req;
    }

    EvaluationRequirement::DubiousTrackDetail SingleDubiousTrack::detail() const
    {
        return detail_;
    }

    unsigned int SingleDubiousTrack::numTracks() const
    {
        return num_tracks_;
    }

    unsigned int SingleDubiousTrack::numTracksDubious() const
    {
        return num_tracks_dubious_;
    }

}
