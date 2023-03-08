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

#include "eval/results/dubious/dubiousbase.h"
#include "evaluationmanager.h"


namespace EvaluationRequirementResult
{

/************************************************************************************
 * SingleDubiousBase
 ************************************************************************************/

const std::string SingleDubiousBase::DetailCommentGroupDubious = "CommentsDubious";

const std::string SingleDubiousBase::DetailUTNOrTrackNum   = "UTNOrTrackNum";
const std::string SingleDubiousBase::DetailFirstInside     = "FirstInside";
const std::string SingleDubiousBase::DetailTODBegin        = "TODBegin";
const std::string SingleDubiousBase::DetailTODEnd          = "TODEnd";
const std::string SingleDubiousBase::DetailDuration        = "Duration";
const std::string SingleDubiousBase::DetailNumPosInside    = "NumPosInside";
const std::string SingleDubiousBase::DetailNumPosInsideDub = "NumPosInsideDub";
const std::string SingleDubiousBase::DetailHasModeAC       = "HasModeAC";
const std::string SingleDubiousBase::DetailHasModeS        = "HasModeS";
const std::string SingleDubiousBase::DetailLeftSector      = "LeftSector";
const std::string SingleDubiousBase::DetailIsDubious       = "IsDubious";

/**
*/
SingleDubiousBase::DetailData::DetailData(unsigned int utn_or_track_number, boost::posix_time::ptime ts_begin)
:   utn_or_tracknum(utn_or_track_number)
,   tod_begin      (ts_begin)
,   tod_end        (ts_begin)
{
}

/**
*/
void SingleDubiousBase::DetailData::assignTo(EvaluationDetail& d) const
{
    d.setValue(DetailUTNOrTrackNum, utn_or_tracknum)
     .setValue(DetailFirstInside, first_inside)
     .setValue(DetailTODBegin, tod_begin)
     .setValue(DetailTODEnd, tod_end)
     .setValue(DetailDuration, duration)
     .setValue(DetailNumPosInside, num_pos_inside)
     .setValue(DetailNumPosInsideDub, num_pos_inside_dubious)
     .setValue(DetailHasModeAC, has_mode_ac)
     .setValue(DetailHasModeS, has_mode_s)
     .setValue(DetailLeftSector, left_sector)
     .setValue(DetailIsDubious, is_dubious)
     .addPosition(pos_begin)
     .addPosition(pos_last)
     .setDetails(details);

    SingleDubiousBase::logComments(d, dubious_reasons);
}

/**
*/
unsigned int SingleDubiousBase::DetailData::numDubious() const
{
    unsigned int cnt = 0;

    for (auto& dd : details)
        if (dd.comments().hasComments(DetailCommentGroupDubious))
            ++cnt;

    return cnt;
}

/**
*/
SingleDubiousBase::SingleDubiousBase(const std::string& result_type,
                                     const std::string& result_id, 
                                     std::shared_ptr<EvaluationRequirement::Base> requirement,
                                     const SectorLayer& sector_layer,
                                     unsigned int utn, 
                                     const EvaluationTargetData* target, 
                                     EvaluationManager& eval_man,
                                     const boost::optional<EvaluationDetails>& details,
                                     unsigned int num_updates,
                                     unsigned int num_pos_outside, 
                                     unsigned int num_pos_inside, 
                                     unsigned int num_pos_inside_dubious)
:   Single(result_type, result_id, requirement, sector_layer, utn, target, eval_man, details)
,   num_updates_           (num_updates)
,   num_pos_outside_       (num_pos_outside)
,   num_pos_inside_        (num_pos_inside)
,   num_pos_inside_dubious_(num_pos_inside_dubious)
{
}

/**
*/
SingleDubiousBase::~SingleDubiousBase() = default;

/**
*/
unsigned int SingleDubiousBase::numPosOutside() const
{
    return num_pos_outside_;
}

/**
*/
unsigned int SingleDubiousBase::numPosInside() const
{
    return num_pos_inside_;
}

/**
*/
unsigned int SingleDubiousBase::numPosInsideDubious() const
{
    return num_pos_inside_dubious_;
}

/**
*/
unsigned int SingleDubiousBase::numUpdates() const
{
    return num_updates_;
}

/**
*/
std::string SingleDubiousBase::dubiousReasonsString(const EvaluationDetailComments& comments)
{
    if (!comments.hasComments(DetailCommentGroupDubious))
        return "OK";
    
    std::string str;

    auto cmts = comments.group(DetailCommentGroupDubious);

    for (const auto& c : cmts.value())
    {
        if (str.size())
            str += ", ";

        str += c.first;
        if (c.second.size())
            str += "(" + c.second + ")";
    }
    
    return str;
}

/**
*/
std::unique_ptr<nlohmann::json::object_t> SingleDubiousBase::getTargetErrorsViewable ()
{
    std::unique_ptr<nlohmann::json::object_t> viewable_ptr = 
        eval_man_.getViewableForEvaluation(utn_, req_grp_id_, result_id_);

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

/**
*/
void SingleDubiousBase::logComment(EvaluationDetail& d, const std::string& id, const std::string& comment)
{
    d.comments().comment(DetailCommentGroupDubious, id, comment);
}

/**
*/
void SingleDubiousBase::logComments(EvaluationDetail& d, const EvaluationDetailComments::CommentGroup& group)
{
    d.comments().group(DetailCommentGroupDubious, group);
}

/**
*/
SingleDubiousBase::EvaluationDetails SingleDubiousBase::generateDetails(const std::vector<DetailData>& detail_data)
{
    if (detail_data.empty())
        return {};

    size_t n = detail_data.size();

    EvaluationDetails details(n);

    for (size_t i = 0; i < n; ++i)
        detail_data[ i ].assignTo(details[ i ]);

    return details;
}

/************************************************************************************
 * JoinedDubiousBase
 ************************************************************************************/

/**
*/
JoinedDubiousBase::JoinedDubiousBase(const std::string& result_type,
                                     const std::string& result_id, 
                                     std::shared_ptr<EvaluationRequirement::Base> requirement,
                                     const SectorLayer& sector_layer, 
                                     EvaluationManager& eval_man)
:   Joined(result_type, result_id, requirement, sector_layer, eval_man)
{
}

}
