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

#include "eval/results/dubious/dubioustargetsingle.h"
#include "eval/results/dubious/dubioustargetjoined.h"
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
:   SingleDubiousBase("SingleDubiousTarget", result_id, requirement, sector_layer, utn, target, eval_man, details, num_updates, num_pos_outside, num_pos_inside, num_pos_inside_dubious)
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
bool SingleDubiousTarget::hasIssues_impl() const
{
    assert (numDetails() == 1);

    return getDetail(0).getValue(DetailKey::IsDubious).toBool();
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

    auto duration_var = Utils::Time::toString(duration.value(),2).c_str();

    return { TargetInfo("#Up [1]"        , "Number of updates"                      , num_updates_                          ),
             TargetInfo("#PosInside [1]" , "Number of updates inside sector"        , num_pos_inside_                       ),
             TargetInfo("#PosOutside [1]", "Number of updates outside sector"       , num_pos_outside_                      ),
             TargetInfo("#DU [1]"        , "Number of dubious updates inside sector", num_pos_inside_dubious_               ),
             TargetInfo("PDU [%]"        , "Probability of dubious update"          , resultValueOptional(p_dubious_update_)),
             TargetInfo("Duration [s]"   , "Duration"                               , duration_var                          ) };
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
bool SingleDubiousTarget::detailIsOk(const EvaluationDetail& detail) const
{
    auto comments = detail.comments().group(DetailCommentGroupDubious);
    bool is_dub   = (comments.has_value() && !comments->empty());

    return !is_dub;
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
        addAnnotationPos(annotations_json, detail.position(0), AnnotationType::TypeHighlight);
    }
    else if (type == TargetAnnotationType::TargetOverview)
    {
        addAnnotationPos(annotations_json, detail.position(0), is_ok ? AnnotationType::TypeOk : AnnotationType::TypeError);
    }
}

/**
*/
std::map<std::string, std::vector<Single::LayerDefinition>> SingleDubiousTarget::gridLayers() const
{
    std::map<std::string, std::vector<Single::LayerDefinition>> layer_defs;

    layer_defs[ requirement_->name() ].push_back(getGridLayerDefBinary());

    return layer_defs;
}

/**
*/
void SingleDubiousTarget::addValuesToGrid(Grid2D& grid, const std::string& layer) const
{
    assert(numDetails() > 0);
    const auto& detail = getDetail(0);

    if (!detail.hasDetails())
        return;

    const auto& details = detail.details();

    if (layer == requirement_->name())
    {
        if (detail.hasDetails())
        {
            auto is_ok = [ & ] (size_t idx)
            {
                auto comments = details[ idx ].comments().group(DetailCommentGroupDubious);
                bool is_dub = (comments.has_value() && !comments->empty());

                return !is_dub;
            };
        
            addValuesToGridBinary(grid, details, is_ok, false);
        }
    }
}

}
