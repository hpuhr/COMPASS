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

#include "eval/results/dubious/dubioustarget.h"

#include "eval/results/base/featuredefinitions.h"

#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"

#include "eval/requirement/base/base.h"
#include "eval/requirement/dubious/dubioustarget.h"

#include "evaluationtargetdata.h"
#include "evaluationmanager.h"

#include "logger.h"
#include "util/timeconv.h"
#include "viewpoint.h"

#include <cassert>
#include <algorithm>

using namespace std;
using namespace Utils;
using namespace nlohmann;

namespace EvaluationRequirementResult
{

/**********************************************************************************************
 * SingleDubiousTarget
 **********************************************************************************************/

/**
*/
SingleDubiousTarget::SingleDubiousTarget(const std::string& result_id,
                                         std::shared_ptr<EvaluationRequirement::Base> requirement,
                                         const SectorLayer& sector_layer,
                                         unsigned int utn,
                                         const EvaluationTargetData* target,
                                         EvaluationManager& eval_man,
                                         const EvaluationDetails& details,
                                         unsigned int num_updates,
                                         unsigned int num_pos_outside,
                                         unsigned int num_pos_inside,
                                         unsigned int num_pos_inside_dubious)
:   SingleDubiousBase("SingleDubiousTarget", result_id, requirement, sector_layer, utn, target, 
                      eval_man, details, num_updates, num_pos_outside, num_pos_inside, num_pos_inside_dubious)
{
    updateResult();
}

/**
*/
std::shared_ptr<Joined> SingleDubiousTarget::createEmptyJoined(const std::string& result_id)
{
    return make_shared<JoinedDubiousTarget> (result_id, requirement_, sector_layer_, eval_man_);
}

/**
*/
EvaluationRequirement::DubiousTarget* SingleDubiousTarget::req ()
{
    EvaluationRequirement::DubiousTarget* req =
            dynamic_cast<EvaluationRequirement::DubiousTarget*>(requirement_.get());
    assert (req);
    return req;
}

/**
*/
boost::optional<double> SingleDubiousTarget::computeResult_impl() const
{
    assert (num_updates_ == num_pos_inside_ + num_pos_outside_);
    //assert (values_.size() == num_comp_failed_ + num_comp_passed_);
    assert (numDetails() == 1);

    p_dubious_update_.reset();

    const auto& detail = getDetail(0);

    boost::optional<double> result = (double)detail.getValue(DetailKey::IsDubious).toBool();

    if (num_pos_inside_)
        p_dubious_update_ = (double)num_pos_inside_dubious_ / (double)num_pos_inside_;

    logdbg << "SingleDubiousTarget "      << requirement_->name() << " " << target_->utn_
           << " has_p_dubious_update_ "   << p_dubious_update_.has_value()
           << " num_pos_inside_dubious_ " << num_pos_inside_dubious_
           << " num_pos_inside_ "         << num_pos_inside_
           << " p_dubious_update_ "       << (p_dubious_update_.has_value() ? p_dubious_update_.value() : 0);

    return result;
}

/**
*/
unsigned int SingleDubiousTarget::numIssues() const
{
    return num_pos_inside_dubious_;
}

/**
*/
std::vector<std::string> SingleDubiousTarget::targetTableHeadersCustom() const
{
    return { "#PosInside", "#DU", "PDU", "Reasons" };
}

/**
*/
std::vector<QVariant> SingleDubiousTarget::targetTableValuesCustom() const
{
    assert(numDetails() > 0);
    std::string dub_string = dubiousReasonsString(getDetail(0).comments());

    return { num_pos_inside_,                        // "#PosInside"
             num_pos_inside_dubious_,                // "#DU"
             resultValueOptional(p_dubious_update_), // "PDU"
             dub_string.c_str() };                   // "Reasons"
}

/**
*/
std::vector<Single::TargetInfo> SingleDubiousTarget::targetInfos() const
{
    assert(numDetails() > 0);

    auto duration = getDetail(0).getValueAs<boost::posix_time::time_duration>(DetailKey::Duration);
    assert(duration.has_value());

    auto duration_var         = formatValue(Time::partialSeconds(duration.value()));
    auto p_dubious_update_var = SingleProbabilityBase::formatProbabilityOptional(p_dubious_update_);

    return { TargetInfo("#Up [1]"        , "Number of updates"                      , num_updates_              ),
             TargetInfo("#PosInside [1]" , "Number of updates inside sector"        , num_pos_inside_           ),
             TargetInfo("#PosOutside [1]", "Number of updates outside sector"       , num_pos_outside_          ),
             TargetInfo("#DU [1]"        , "Number of dubious updates inside sector", num_pos_inside_dubious_   ),
             TargetInfo("PDU [%]"        , "Probability of dubious update"          , p_dubious_update_var      ),
             TargetInfo("Duration [s]"   , "Duration"                               , duration_var              ) };
}

/**
*/
std::vector<std::string> SingleDubiousTarget::detailHeaders() const
{
    return { "ToD", "UTN", "Dubious Comment" };
}

/**
*/
std::vector<QVariant> SingleDubiousTarget::detailValues(const EvaluationDetail& detail,
                                                        const EvaluationDetail* parent_detail) const
{
    assert(parent_detail);

    const std::string dub_string = dubiousReasonsString(parent_detail->comments());

    return { Time::toString(detail.timestamp()).c_str(),
             parent_detail->getValue(DetailKey::UTNOrTrackNum),
             dub_string.c_str() };
}

/**
*/
bool SingleDubiousTarget::detailIsOkStatic(const EvaluationDetail& detail)
{
    auto comments = detail.comments().group(DetailCommentGroupDubious);
    bool is_dub   = (comments.has_value() && !comments->empty());

    return !is_dub;
}

/**
*/
bool SingleDubiousTarget::detailIsOk(const EvaluationDetail& detail) const
{
    return SingleDubiousTarget::detailIsOkStatic(detail);
}

/**
*/
void SingleDubiousTarget::addAnnotationForDetail(nlohmann::json& annotations_json, 
                                                 const EvaluationDetail& detail, 
                                                 TargetAnnotationType type,
                                                 bool is_ok) const
{
    assert (detail.numPositions() >= 1);

    if (type == TargetAnnotationType::Highlight)
    {
        addAnnotationPos(annotations_json, detail.position(0), AnnotationArrayType::TypeHighlight);
    }
    else if (type == TargetAnnotationType::TargetOverview)
    {
        addAnnotationPos(annotations_json, detail.position(0), is_ok ? AnnotationArrayType::TypeOk : AnnotationArrayType::TypeError);
    }
}

/**********************************************************************************************
 * JoinedDubiousTarget
 **********************************************************************************************/

/**
*/
JoinedDubiousTarget::JoinedDubiousTarget(const std::string& result_id, 
                                         std::shared_ptr<EvaluationRequirement::Base> requirement,
                                         const SectorLayer& sector_layer, 
                                         EvaluationManager& eval_man)
:   JoinedDubiousBase("JoinedDubiousTarget", result_id, requirement, sector_layer, eval_man)
{
}

/**
*/
unsigned int JoinedDubiousTarget::numIssues() const
{
    return num_pos_inside_dubious_;
}

/**
*/
unsigned int JoinedDubiousTarget::numUpdates() const
{
    return num_updates_;
}

/**
*/
void JoinedDubiousTarget::clearResults_impl() 
{
    num_updates_ = 0;
    num_pos_outside_ = 0;
    num_pos_inside_ = 0;
    num_pos_inside_dubious_ = 0;

    num_utns_ = 0;
    num_utns_dubious_ = 0;

    duration_all_ = 0;
    duration_nondub_ = 0;
    duration_dubious_ = 0;
}

/**
*/
void JoinedDubiousTarget::accumulateSingleResult(const std::shared_ptr<Single>& single_result, bool first, bool last)
{
    std::shared_ptr<SingleDubiousTarget> single = std::static_pointer_cast<SingleDubiousTarget>(single_result);

    num_updates_            += single->numUpdates();
    num_pos_outside_        += single->numPosOutside();
    num_pos_inside_         += single->numPosInside();
    num_pos_inside_dubious_ += single->numPosInsideDubious();

    num_utns_ += 1;

    assert (single->numDetails() >= 1);

    const auto& detail = single->getDetail(0);

    auto is_dubious = detail.getValueAs<bool>(SingleDubiousTarget::DetailKey::IsDubious);
    assert(is_dubious.has_value());

    auto duration = detail.getValueAs<boost::posix_time::time_duration>(SingleDubiousTarget::DetailKey::Duration);
    assert(duration.has_value());

    if (is_dubious.value())
        num_utns_dubious_ += 1;

    duration_all_ += Time::partialSeconds(duration.value());

    if (is_dubious.value())
        duration_dubious_ += Time::partialSeconds(duration.value());
    else
        duration_nondub_ += Time::partialSeconds(duration.value());
}

/**
*/
boost::optional<double> JoinedDubiousTarget::computeResult_impl() const
{
    loginf << "JoinedDubiousTarget: computeResult_impl:"
            << " num_updates " << num_updates_
            << " num_utns " << num_utns_
            << " num_utns_dubious " << num_utns_dubious_
            << " num_pos_inside " << num_pos_inside_
            << " num_pos_inside_dubious " << num_pos_inside_dubious_;

    assert (num_updates_ == num_pos_inside_ + num_pos_outside_);
    assert (num_utns_ >= num_utns_dubious_);

    p_dubious_update_.reset();

    if (num_pos_inside_)
        p_dubious_update_ = (double)num_pos_inside_dubious_ / (double)num_pos_inside_;

    if (num_utns_ == 0)
        return {};

    return (double)num_utns_dubious_ / (double)num_utns_;
}

/**
*/
std::vector<Joined::SectorInfo> JoinedDubiousTarget::sectorInfos() const
{
    auto p_dubious_update_var = SingleProbabilityBase::formatProbabilityOptional(p_dubious_update_);

    std::vector<Joined::SectorInfo> infos = 
        { { "#Up [1]"             , "Number of updates"                      , num_updates_                   },
          { "#PosInside [1]"      , "Number of updates inside sector"        , num_pos_inside_                },
          { "#PosOutside [1]"     , "Number of updates outside sector"       , num_pos_outside_               },
          { "#DU [1]"             , "Number of dubious updates inside sector", num_pos_inside_dubious_        },
          { "PDU [%]"             , "Probability of dubious update"          , p_dubious_update_var           },
          { "#T [1]"              , "Number of targets"                      , num_utns_                      },
          { "#DT [1]"             , "Number of dubious targets"              , num_utns_dubious_              },
          { "Duration [s]"        , "Duration of all targets"                , formatValue(duration_all_)     },
          { "Duration Dubious [s]", "Duration of dubious targets"            , formatValue(duration_dubious_) } };

    QVariant dubious_t_avg_var;

    if (num_utns_dubious_)
        dubious_t_avg_var = roundf(duration_dubious_/(float)num_utns_dubious_ * 100.0) / 100.0;

    infos.push_back({"Duration Non-Dubious [s]", "Duration of non-dubious targets", formatValue(duration_nondub_)});
    infos.push_back({"Average Duration Dubious [s]", "Average duration of dubious targets", dubious_t_avg_var});

    QVariant p_dubious_t_var, p_nondub_t_var;

    if (duration_all_)
    {
        p_dubious_t_var = SingleProbabilityBase::formatProbability(duration_dubious_ / duration_all_);
        p_nondub_t_var  = SingleProbabilityBase::formatProbability(duration_nondub_  / duration_all_);
    }

    infos.push_back({"Duration Ratio Dubious [%]", "Duration ratio of dubious targets", p_dubious_t_var});
    infos.push_back({"Duration Ratio Non-Dubious [%]", "Duration ratio of non-dubious targets", p_nondub_t_var});

    return infos;
}

/**
*/
FeatureDefinitions JoinedDubiousTarget::getCustomAnnotationDefinitions() const
{
    FeatureDefinitions defs;

    //dubious state to binary value
    // auto getValue = [ = ] (const EvaluationDetail& detail)
    // {
    //     bool ok = SingleDubiousTarget::detailIsOkStatic(detail);
    //     return ok ? 1.0 : 0.0;
    // };

    // return AnnotationDefinitions().addBinaryGrid("", 
    //                                              requirement_->name(),
    //                                              DetailValueSource(getValue),
    //                                              GridAddDetailMode::AddEvtPosition,
    //                                              false,
    //                                              Qt::green,
    //                                              Qt::red);

    return defs;
}

}
