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
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"

#include "eval/requirement/base/base.h"
#include "eval/requirement/dubious/dubioustrack.h"

#include "evaluationtargetdata.h"
#include "evaluationmanager.h"

#include "logger.h"
#include "util/stringconv.h"
#include "util/timeconv.h"
#include "viewpoint.h"

#include <cassert>
#include <algorithm>

using namespace std;
using namespace Utils;
using namespace nlohmann;

namespace EvaluationRequirementResult
{

/**
*/
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
    updateResult();
}

/**
*/
SingleDubiousTrack::~SingleDubiousTrack() = default;

/**
*/
float SingleDubiousTrack::trackDurationAll() const
{
    return track_duration_all_;
}

/**
*/
float SingleDubiousTrack::trackDurationNondub() const
{
    return track_duration_nondub_;
}

/**
*/
float SingleDubiousTrack::trackDurationDubious() const
{
    return track_duration_dubious_;
}

/**
*/
unsigned int SingleDubiousTrack::numTracks() const
{
    return num_tracks_;
}

/**
*/
unsigned int SingleDubiousTrack::numTracksDubious() const
{
    return num_tracks_dubious_;
}

/**
*/
std::shared_ptr<Joined> SingleDubiousTrack::createEmptyJoined(const std::string& result_id)
{
    return make_shared<JoinedDubiousTrack> (result_id, requirement_, sector_layer_, eval_man_);
}

/**
*/
EvaluationRequirement::DubiousTrack* SingleDubiousTrack::req ()
{
    EvaluationRequirement::DubiousTrack* req =
            dynamic_cast<EvaluationRequirement::DubiousTrack*>(requirement_.get());
    assert (req);
    return req;
}

/**
*/
boost::optional<double> SingleDubiousTrack::computeResult_impl() const
{
    assert (num_updates_ == num_pos_inside_ + num_pos_outside_);
    assert (num_tracks_ >= num_tracks_dubious_);

    p_dubious_update_.reset();

    boost::optional<double> result;

    if (num_tracks_)
    {
        result = (double)num_tracks_dubious_ / (double)num_tracks_;

        for (const auto& detail_it : getDetails())
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
    
    if (num_pos_inside_)
    {
        p_dubious_update_ = (double)num_pos_inside_dubious_ / (double)num_pos_inside_;
    }

    logdbg << "SingleDubiousTrack "       << requirement_->name() << " " << target_->utn_
           << " has_p_dubious_update_ "   << p_dubious_update_.has_value()
           << " num_pos_inside_dubious_ " << num_pos_inside_dubious_
           << " num_pos_inside_ "         << num_pos_inside_
           << " p_dubious_update_ "       << (p_dubious_update_.has_value() ? p_dubious_update_.value() : 0);

    return result;
}

/**
*/
bool SingleDubiousTrack::hasIssues_impl() const
{
    return num_tracks_dubious_ > 0;
}

/**
*/
std::vector<std::string> SingleDubiousTrack::targetTableHeadersCustom() const
{
    return { "#PosInside", "#DU", "PDU", "#T", "#DT", "Reasons" };
}

/**
*/
std::vector<QVariant> SingleDubiousTrack::targetTableValuesCustom() const
{
    //collect reasons
    string reasons;

    for (const auto& detail_it : getDetails())
    {
        if (reasons.size())
            reasons += "\n";

        auto dub_reasons_str = dubiousReasonsString(detail_it.comments());

        reasons += to_string(detail_it.getValue(DetailKey::UTNOrTrackNum).toUInt()) + ":" + dub_reasons_str;
    }

    return { num_pos_inside_,                        // "#IU"
             num_pos_inside_dubious_,                // "#DU"
             resultValueOptional(p_dubious_update_), // "PDU"
             num_tracks_,                            // "#T"
             num_tracks_dubious_,                    // "#DT"
             reasons.c_str() };                      // "Reasons"
}

/**
*/
std::vector<Single::TargetInfo> SingleDubiousTrack::targetInfos() const
{
    QVariant p_dubious_up_var          = resultValueOptional(p_dubious_update_);
    QVariant track_duration_var        = Utils::String::doubleToStringPrecision(track_duration_all_,2).c_str();
    QVariant track_duration_dub_var    = Utils::String::doubleToStringPrecision(track_duration_dubious_,2).c_str();
    QVariant track_duration_nondub_var = Utils::String::doubleToStringPrecision(track_duration_nondub_,2).c_str();
    QVariant dubious_t_avg_var         = num_tracks_dubious_ > 0 ? roundf(track_duration_dubious_/(float)num_tracks_dubious_ * 100.0) / 100.0 : QVariant();

    QVariant p_dubious_t_var, p_nondub_t_var;

    if (track_duration_all_)
    {
        p_dubious_t_var = roundf(track_duration_dubious_ / track_duration_all_ * 10000.0) / 100.0;
        p_nondub_t_var  = roundf(track_duration_nondub_  / track_duration_all_ * 10000.0) / 100.0;
    }

    return { TargetInfo("#Up [1]"                        , "Number of updates"                      , num_updates_             ),
             TargetInfo("#PosInside [1]"                 , "Number of updates inside sector"        , num_pos_inside_          ),
             TargetInfo("#PosOutside [1]"                , "Number of updates outside sector"       , num_pos_outside_         ),
             TargetInfo("#DU [1]"                        , "Number of dubious updates inside sector", num_pos_inside_dubious_  ),
             TargetInfo("PDU [%]"                        , "Probability of dubious update"          , p_dubious_up_var         ),
             TargetInfo("#T [1]"                         , "Number of tracks"                       , num_tracks_              ),
             TargetInfo("#DT [1]"                        , "Number of dubious tracks"               , num_tracks_dubious_      ),
             TargetInfo("Duration [s]"                   , "Duration of all tracks"                 , track_duration_var       ),
             TargetInfo("Duration Dubious [s]"           , "Duration of dubious tracks"             , track_duration_dub_var   ),
             TargetInfo("Duration Non-Dubious [s]"       , "Duration of non-dubious tracks"         , track_duration_nondub_var),
             TargetInfo("Average Duration Dubious [s]"   , "Average duration of dubious tracks"     , dubious_t_avg_var        ),
             TargetInfo("Duration Dubious Ratio [%]"     , "Duration ratio of dubious tracks"       , p_dubious_t_var          ),
             TargetInfo("Duration Non-Dubious Ration [%]", "Duration ratio of non-dubious tracks"   , p_nondub_t_var           ) };
}

/**
*/
std::vector<std::string> SingleDubiousTrack::detailHeaders() const
{
    return {"ToD", "TN", "Dubious Comment"};
}

/**
*/
std::vector<QVariant> SingleDubiousTrack::detailValues(const EvaluationDetail& detail,
                                                       const EvaluationDetail* parent_detail) const
{
    assert(parent_detail);

    return { Utils::Time::toString(detail.timestamp()).c_str(),
             parent_detail->getValue(DetailKey::UTNOrTrackNum),
             dubiousReasonsString(detail.comments()).c_str() }; // "Comment"
}

/**
*/
bool SingleDubiousTrack::detailIsOk(const EvaluationDetail& detail) const
{
    auto comments = detail.comments().group(DetailCommentGroupDubious);
    bool is_dub   = (comments.has_value() && !comments->empty());

    return !is_dub;
}

/**
*/
void SingleDubiousTrack::addAnnotationForDetail(nlohmann::json& annotations_json, 
                                                const EvaluationDetail& detail, 
                                                TargetAnnotationType type,
                                                bool is_ok) const
{
    assert (detail.numPositions() >= 1);

    if (type == TargetAnnotationType::Highlight)
    {
        addAnnotationPos(annotations_json, detail.position(0), AnnotationType::TypeHighlight);
    }
    else if (type == TargetAnnotationType::TargetOverview)
    {
        addAnnotationPos(annotations_json, detail.position(0), is_ok ? AnnotationType::TypeOk : AnnotationType::TypeError);
    }
}

/**
*/
std::map<std::string, std::vector<Single::LayerDefinition>> SingleDubiousTrack::gridLayers() const
{
    std::map<std::string, std::vector<Single::LayerDefinition>> layer_defs;

    layer_defs[ requirement_->name() ].push_back(getGridLayerDefBinary());

    return layer_defs;
}

/**
*/
void SingleDubiousTrack::addValuesToGrid(Grid2D& grid, const std::string& layer) const
{
    const auto& details = getDetails();

    if (layer == requirement_->name())
    {
        for (const auto& d : details)
        {
            if (d.hasDetails())
            {
                const auto& updates = d.details();

                auto is_ok = [ & ] (size_t idx)
                {
                    auto comments = updates[ idx ].comments().group(DetailCommentGroupDubious);
                    bool is_dub = (comments.has_value() && !comments->empty());

                    return !is_dub;
                };
            
                addValuesToGridBinary(grid, updates, is_ok, false);
            }
        }
    }
}

}
