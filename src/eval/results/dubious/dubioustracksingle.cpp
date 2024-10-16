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
#include "util/stringconv.h"
#include "util/timeconv.h"
//#include "util/number.h"

#include <cassert>
#include <algorithm>

using namespace std;
using namespace Utils;
using namespace nlohmann;

namespace EvaluationRequirementResult
{

SingleDubiousTrack::SingleDubiousTrack(const std::string& result_id, 
                                       std::shared_ptr<EvaluationRequirement::Base> requirement,
                                       const SectorLayer& sector_layer,
                                       unsigned int utn,
                                       const EvaluationTargetData* target,
                                       EvaluationManager& eval_man,
                                       const EvaluationDetails& details,
                                       unsigned int num_updates,
                                       unsigned int num_pos_outside,
                                       unsigned int num_pos_inside,
                                       unsigned int num_pos_inside_dubious,
                                       unsigned int num_tracks,
                                       unsigned int num_tracks_dubious)
    :   SingleDubiousBase  ("SingleDubiousTrack", result_id, requirement, sector_layer, utn, target, eval_man, details,
                            num_updates, num_pos_outside, num_pos_inside, num_pos_inside_dubious)
    ,   num_tracks_        (num_tracks)
    ,   num_tracks_dubious_(num_tracks_dubious)
{
    update();
}

SingleDubiousTrack::~SingleDubiousTrack() = default;

void SingleDubiousTrack::update()
{
    assert (num_updates_ == num_pos_inside_ + num_pos_outside_);
    assert (num_tracks_ >= num_tracks_dubious_);

    if (num_tracks_)
    {
        p_dubious_track_ = (float)num_tracks_dubious_/(float)num_tracks_;

        result_usable_ = true;

        for (auto& detail_it : getDetails())
        {
            auto duration   = detail_it.getValue(DetailKey::Duration).toDouble();
            auto is_dubious = detail_it.getValue(DetailKey::IsDubious).toBool();

            track_duration_all_ += duration;

            if (is_dubious)
                track_duration_dubious_ += duration;
            else
                track_duration_nondub_ += duration;
        }
    }

    result_usable_ = p_dubious_track_.has_value();

    if (num_pos_inside_)
    {
        p_dubious_update_ = (float)num_pos_inside_dubious_/(float)num_pos_inside_;
    }

    logdbg << "SingleDubiousTrack "       << requirement_->name() << " " << target_->utn_
           << " has_p_dubious_update_ "   << p_dubious_update_.has_value()
           << " num_pos_inside_dubious_ " << num_pos_inside_dubious_
           << " num_pos_inside_ "         << num_pos_inside_
           << " p_dubious_update_ "       << (p_dubious_update_.has_value() ? p_dubious_update_.value() : 0);

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

    if (eval_man_.settings().report_show_adsb_info_)
        addTargetDetailsToTableADSB(tgt_overview_section, target_table_name_);
    else
        addTargetDetailsToTable(tgt_overview_section, target_table_name_);

    if (eval_man_.settings().report_split_results_by_mops_
            || eval_man_.settings().report_split_results_by_aconly_ms_) // add to general sum table
    {
        EvaluationResultsReport::Section& sum_section = root_item->getSection(getRequirementSumSectionID());

        if (eval_man_.settings().report_show_adsb_info_)
            addTargetDetailsToTableADSB(sum_section, target_table_name_);
        else
            addTargetDetailsToTable(sum_section, target_table_name_);
    }
}

void SingleDubiousTrack::addTargetDetailsToTable (EvaluationResultsReport::Section& section, 
                                                  const std::string& table_name)
{
    if (!section.hasTable(table_name))
    {
        Qt::SortOrder order = Qt::AscendingOrder;

        if(req()->probCheckType() == EvaluationRequirement::COMPARISON_TYPE::LESS_THAN
                || req()->probCheckType() == EvaluationRequirement::COMPARISON_TYPE::LESS_THAN_OR_EQUAL)
            order = Qt::DescendingOrder;

        section.addTable(table_name, 15,
                         {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                          "#PosInside", "#DU", "PDU", "#T", "#DT", "Reasons", "PDT"}, true, 14, order);
    }

    EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

    QVariant p_dubious_up_var;

    if (p_dubious_update_.has_value())
        p_dubious_up_var = roundf(p_dubious_update_.value() * 10000.0) / 100.0;

    QVariant p_dubious_track_var;

    if (p_dubious_track_.has_value())
        p_dubious_track_var = roundf(p_dubious_track_.value() * 10000.0) / 100.0;

    string reasons;

    for (auto& detail_it : getDetails())
    {
        if (reasons.size())
            reasons += "\n";

        auto dub_reasons_str = dubiousReasonsString(detail_it.comments());

        reasons += to_string(detail_it.getValue(DetailKey::UTNOrTrackNum).toUInt())+":" + dub_reasons_str;
    }

    target_table.addRow(
                {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
                 target_->acidsStr().c_str(), target_->acadsStr().c_str(),
                 target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(), target_->modeCMaxStr().c_str(),
                 num_pos_inside_, // "#IU"
                 num_pos_inside_dubious_, // "#DU"
                 p_dubious_up_var, // "PDU"
                 num_tracks_, // "#T"
                 num_tracks_dubious_, // "#DT"
                 reasons.c_str(),  // "Reasons"
                 p_dubious_track_var}, // "PDT"
                this, {utn_});
}

void SingleDubiousTrack::addTargetDetailsToTableADSB (EvaluationResultsReport::Section& section, 
                                                      const std::string& table_name)
{
    if (!section.hasTable(table_name))
    {
        Qt::SortOrder order = Qt::AscendingOrder;

        if(req()->probCheckType() == EvaluationRequirement::COMPARISON_TYPE::LESS_THAN
                || req()->probCheckType() == EvaluationRequirement::COMPARISON_TYPE::LESS_THAN_OR_EQUAL)
            order = Qt::DescendingOrder;

        section.addTable(table_name, 16,
                         {"UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
                          "#PosInside", "#DU", "PDU", "#T", "#DT", "Reasons", "PDT",
                          "MOPS"}, true, 14, order);
    }

    EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

    QVariant p_dubious_up_var;

    if (p_dubious_update_.has_value())
        p_dubious_up_var = roundf(p_dubious_update_.value() * 10000.0) / 100.0;

    QVariant p_dubious_var;

    if (p_dubious_track_.has_value())
        p_dubious_var = roundf(p_dubious_track_.value() * 10000.0) / 100.0;

    string reasons;

    for (auto& detail_it : getDetails())
    {
        if (reasons.size())
            reasons += "\n";

        auto dub_reasons_str = dubiousReasonsString(detail_it.comments());

        reasons += to_string(detail_it.getValue(DetailKey::UTNOrTrackNum).toUInt())+":" + dub_reasons_str;
    }

    // "UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max",
    // "#ACOK", "#ACNOK", "PACOK", "#DOK", "#DNOK", "PDOK", "MOPS", "NUCp/NIC", "NACp"

    target_table.addRow(
                {utn_, target_->timeBeginStr().c_str(), target_->timeEndStr().c_str(),
                 target_->acidsStr().c_str(), target_->acadsStr().c_str(),
                 target_->modeACodesStr().c_str(), target_->modeCMinStr().c_str(),
                 target_->modeCMaxStr().c_str(),
                 num_pos_inside_, // "#PosInside"
                 num_pos_inside_dubious_, // "#DU"
                 p_dubious_up_var, // "PDU"
                 num_tracks_, // "#T"
                 num_tracks_dubious_, // "#DT"
                 reasons.c_str(),  // "Reasons"
                 p_dubious_var, // "PDT"
                 target_->mopsVersionStr().c_str()}, // "MOPS"
                this, {utn_});

}

void SingleDubiousTrack::addTargetDetailsToReport(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    root_item->getSection(getTargetSectionID()).perTargetSection(true); // mark utn section per target
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
    utn_req_table.addRow({"#DU [1]", "Number of dubious updates inside sector", num_pos_inside_dubious_}, this);

    QVariant p_dubious_up_var;

    if (p_dubious_update_.has_value())
        p_dubious_up_var = roundf(p_dubious_update_.value() * 10000.0) / 100.0;

    utn_req_table.addRow({"PDU [%]", "Probability of dubious update", p_dubious_up_var}, this);

    utn_req_table.addRow({"#T [1]", "Number of tracks", num_tracks_}, this);
    utn_req_table.addRow({"#DT [1]", "Number of dubious tracks", num_tracks_dubious_},
                         this);

    utn_req_table.addRow({"Duration [s]", "Duration of all tracks",
                          String::doubleToStringPrecision(track_duration_all_,2).c_str()}, this);
    utn_req_table.addRow({"Duration Dubious [s]", "Duration of dubious tracks",
                          String::doubleToStringPrecision(track_duration_dubious_,2).c_str()}, this);

    QVariant dubious_t_avg_var;

    if (num_tracks_dubious_)
        dubious_t_avg_var = roundf(track_duration_dubious_/(float)num_tracks_dubious_ * 100.0) / 100.0;

    utn_req_table.addRow({"Duration Non-Dubious [s]", "Duration of non-dubious tracks",
                          String::doubleToStringPrecision(track_duration_nondub_,2).c_str()}, this);

    utn_req_table.addRow({"Average Duration Dubious [s]", "Average duration of dubious tracks",
                          dubious_t_avg_var}, this);

    QVariant p_dubious_t_var, p_nondub_t_var;

    if (track_duration_all_)
    {
        p_dubious_t_var = roundf(track_duration_dubious_/track_duration_all_ * 10000.0) / 100.0;
        p_nondub_t_var  = roundf(track_duration_nondub_ /track_duration_all_ * 10000.0) / 100.0;
    }

    utn_req_table.addRow({"Duration Dubious Ratio [%]", "Duration ratio of dubious tracks", p_dubious_t_var}, this);
    utn_req_table.addRow({"Duration Non-Dubious Ration [%]", "Duration ratio of non-dubious tracks", p_nondub_t_var}, this);

    // condition
    {
        QVariant p_dubious_var;

        if (p_dubious_track_.has_value())
            p_dubious_track_ = roundf(p_dubious_track_.value() * 10000.0) / 100.0;

        utn_req_table.addRow({"PDT [%]", "Probability of dubious track", p_dubious_var}, this);

        utn_req_table.addRow({"Condition", {}, req->getConditionStr().c_str()}, this);

        string result {"Unknown"};

        if (p_dubious_track_.has_value())
            result = req->getConditionResultStr(p_dubious_track_.value());

        utn_req_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);

        if (result == "Failed")
        {
            root_item->getSection(getTargetSectionID()).perTargetWithIssues(true); // mark utn section as with issue
            utn_req_section.perTargetWithIssues(true);
        }
    }

    if (p_dubious_track_.has_value() && p_dubious_track_.value() != 0.0) // TODO
    {
        utn_req_section.addFigure("target_errors_overview", "Target Errors Overview",
                                  [this](void) { return this->getTargetErrorsViewable(); });
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
    if (!utn_req_section.hasTable(tr_details_table_name_))
        utn_req_section.addTable(tr_details_table_name_, 3,
                                 {"ToD", "TN", "Dubious Comment"});

    EvaluationResultsReport::SectionContentTable& utn_req_details_table =
            utn_req_section.getTable(tr_details_table_name_);

    utn_req_details_table.setCreateOnDemand(
                [this, &utn_req_details_table](void)
    {

        unsigned int detail_update_cnt = 0;

        //iterate over details
        for (auto& rq_det_it : getDetails())
        {
            if (!rq_det_it.hasDetails())
                continue;

            //iterate over updates
            for (auto& update : rq_det_it.details())
            {
                utn_req_details_table.addRow(
                            { Time::toString(update.timestamp()).c_str(),
                              rq_det_it.getValue(DetailKey::UTNOrTrackNum),
                              dubiousReasonsString(update.comments()).c_str() }, // "Comment"
                            this, {detail_update_cnt});

                ++detail_update_cnt;
            }
        }});
}

bool SingleDubiousTrack::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else if (table.name() == tr_details_table_name_ && annotation.isValid()) //  && annotation.toUInt() < details_.size()
        return true;
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
    else if (table.name() == tr_details_table_name_ && annotation.isValid())
    {
        unsigned int detail_update_cnt = annotation.toUInt();

        logdbg << "SingleDubiousTrack: viewableData: detail_update_cnt " << detail_update_cnt;

        std::unique_ptr<nlohmann::json::object_t> viewable_ptr
                = eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_);
        assert (viewable_ptr);

        //        unsigned int detail_cnt = 0;
        //        unsigned int per_detail_update_cnt = detail_update_cnt;

        //        const auto& details = getDetails();
        //        assert (details.size());

        //        while (per_detail_update_cnt >= details.at(detail_cnt).numDetails())
        //        {
        //            per_detail_update_cnt -= details.at(detail_cnt).numDetails();
        //            ++detail_cnt;

        //            assert (detail_cnt < details.size());
        //        }

        //        logdbg << "SingleDubiousTrack: viewableData: FINAL detail_cnt " << detail_cnt
        //               << " update detail size " << details.at(detail_cnt).numDetails()
        //               << " per_detail_update_cnt " << per_detail_update_cnt;

        //        assert (detail_cnt < details.size());
        //        assert (per_detail_update_cnt < details.at(detail_cnt).numDetails());

        //        const auto& detail        = details.at(detail_cnt);
        //        const auto& update_detail = detail.details().at(per_detail_update_cnt);

        //        assert(update_detail.numPositions() >= 1);

        //        (*viewable_ptr)[ViewPoint::VP_POS_LAT_KEY    ] = update_detail.position(0).latitude_;
        //        (*viewable_ptr)[ViewPoint::VP_POS_LON_KEY    ] = update_detail.position(0).longitude_;
        //        (*viewable_ptr)[ViewPoint::VP_POS_WIN_LAT_KEY] = eval_man_.settings().result_detail_zoom_;
        //        (*viewable_ptr)[ViewPoint::VP_POS_WIN_LON_KEY] = eval_man_.settings().result_detail_zoom_;
        //        (*viewable_ptr)[ViewPoint::VP_TIMESTAMP_KEY  ] = Time::toString(update_detail.timestamp());

        //        if (update_detail.comments().hasComments(DetailCommentGroupDubious))
        //            (*viewable_ptr)[ViewPoint::VP_EVAL_KEY][ViewPoint::VP_EVAL_HIGHDET_KEY] = vector<unsigned int>{detail_update_cnt};

        return viewable_ptr;
    }
    else
        return nullptr;
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

std::shared_ptr<Joined> SingleDubiousTrack::createEmptyJoined(const std::string& result_id)
{
    return make_shared<JoinedDubiousTrack> (result_id, requirement_, sector_layer_, eval_man_);
}

EvaluationRequirement::DubiousTrack* SingleDubiousTrack::req ()
{
    EvaluationRequirement::DubiousTrack* req =
            dynamic_cast<EvaluationRequirement::DubiousTrack*>(requirement_.get());
    assert (req);
    return req;
}

void SingleDubiousTrack::addAnnotations(nlohmann::json::object_t& viewable, bool overview, bool add_ok)
{
     //addAnnotationFeatures(viewable, overview); // TODO rework

    json& error_line_coordinates  = annotationLineCoords(viewable, TypeError, overview);
    json& error_point_coordinates = annotationPointCoords(viewable, TypeError, overview);
    json& ok_line_coordinates     = annotationLineCoords(viewable, TypeOk, overview);
    json& ok_point_coordinates    = annotationPointCoords(viewable, TypeOk, overview);
}

float SingleDubiousTrack::trackDurationAll() const
{
    return track_duration_all_;
}

float SingleDubiousTrack::trackDurationNondub() const
{
    return track_duration_nondub_;
}

float SingleDubiousTrack::trackDurationDubious() const
{
    return track_duration_dubious_;
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
