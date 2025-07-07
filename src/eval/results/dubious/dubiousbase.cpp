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
 * DubiousBase
 ************************************************************************************/

/**
*/
DubiousBase::DubiousBase() = default;

/**
*/
DubiousBase::DubiousBase(unsigned int num_updates,
                         unsigned int num_pos_outside,
                         unsigned int num_pos_inside,
                         unsigned int num_pos_inside_dubious)
:   num_updates_           (num_updates)
,   num_pos_outside_       (num_pos_outside)
,   num_pos_inside_        (num_pos_inside)
,   num_pos_inside_dubious_(num_pos_inside_dubious)
{
}

/**
*/
unsigned int DubiousBase::numPosOutside() const
{
    return num_pos_outside_;
}

/**
*/
unsigned int DubiousBase::numPosInside() const
{
    return num_pos_inside_;
}

/**
*/
unsigned int DubiousBase::numPosInsideDubious() const
{
    return num_pos_inside_dubious_;
}

/**
*/
unsigned int DubiousBase::numUpdates() const
{
    return num_updates_;
}

/************************************************************************************
 * SingleDubiousBase::DetailData
 ************************************************************************************/

/**
*/
SingleDubiousBase::DetailData::DetailData(unsigned int utn_or_track_number, 
                                          boost::posix_time::ptime ts_begin)
:   utn_or_tracknum(utn_or_track_number)
,   tod_begin      (ts_begin           )
,   tod_end        (ts_begin           )
{
}

/**
*/
void SingleDubiousBase::DetailData::assignTo(EvaluationDetail& d) const
{
    d.setValue(DetailKey::UTNOrTrackNum, utn_or_tracknum)
     .setValue(DetailKey::FirstInside, first_inside)
     .setValue(DetailKey::TODBegin, tod_begin)
     .setValue(DetailKey::TODEnd, tod_end)
     .setValue(DetailKey::Duration, duration)
     .setValue(DetailKey::NumPosInside, num_pos_inside)
     .setValue(DetailKey::NumPosInsideDub, num_pos_inside_dubious)
     .setValue(DetailKey::HasModeAC, has_mode_ac)
     .setValue(DetailKey::HasModeS, has_mode_s)
     .setValue(DetailKey::LeftSector, left_sector)
     .setValue(DetailKey::IsDubious, is_dubious)
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

/************************************************************************************
 * SingleDubiousBase
 ************************************************************************************/

const std::string SingleDubiousBase::DetailCommentGroupDubious = "CommentsDubious";

/**
*/
SingleDubiousBase::SingleDubiousBase(const std::string& result_type,
                                     const std::string& result_id, 
                                     std::shared_ptr<EvaluationRequirement::Base> requirement,
                                     const SectorLayer& sector_layer,
                                     unsigned int utn, 
                                     const EvaluationTargetData* target, 
                                     EvaluationCalculator& calculator,
                                     const EvaluationDetails& details,
                                     unsigned int num_updates,
                                     unsigned int num_pos_outside, 
                                     unsigned int num_pos_inside, 
                                     unsigned int num_pos_inside_dubious)
:   DubiousBase(num_updates, num_pos_outside, num_pos_inside, num_pos_inside_dubious)
,   SingleProbabilityBase(result_type, result_id, requirement, sector_layer, utn, target, calculator, details)
{
}

/**
*/
SingleDubiousBase::~SingleDubiousBase() = default;

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
                                     EvaluationCalculator& calculator)
:   DubiousBase()
,   JoinedProbabilityBase(result_type, result_id, requirement, sector_layer, calculator)
{
}

}
