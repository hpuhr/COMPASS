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

#include "eval/results/extra/tracksingle.h"
#include "eval/results/extra/trackjoined.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"

#include "eval/requirement/base/base.h"
#include "eval/requirement/extra/track.h"

#include "evaluationtargetdata.h"
#include "evaluationmanager.h"

#include "logger.h"
#include "util/timeconv.h"
#include "viewpoint.h"

#include <cassert>

using namespace std;
using namespace Utils;
using namespace nlohmann;

namespace EvaluationRequirementResult
{

/**
*/
SingleExtraTrack::SingleExtraTrack(const std::string& result_id, 
                                   std::shared_ptr<EvaluationRequirement::Base> requirement,
                                   const SectorLayer& sector_layer,
                                   unsigned int utn,
                                   const EvaluationTargetData* target,
                                   EvaluationManager& eval_man,
                                   const EvaluationDetails& details,
                                   bool ignore,
                                   unsigned int num_inside,
                                   unsigned int num_extra,
                                   unsigned int num_ok)
:   SingleProbabilityBase("SingleExtraTrack", result_id, requirement, sector_layer, utn, target, eval_man, details)
,   num_inside_(num_inside)
,   num_extra_ (num_extra)
,   num_ok_    (num_ok)
{
    if (ignore)
        setIgnored();

    updateResult();
}

/**
*/
unsigned int SingleExtraTrack::numInside() const
{
    return num_inside_;
}

/**
*/
unsigned int SingleExtraTrack::numExtra() const
{
    return num_extra_;
}

/**
*/
unsigned int SingleExtraTrack::numOK() const
{
    return num_ok_;
}

/**
*/
std::shared_ptr<Joined> SingleExtraTrack::createEmptyJoined(const std::string& result_id)
{
    return make_shared<JoinedExtraTrack> (result_id, requirement_, sector_layer_, eval_man_);
}


/**
*/
boost::optional<double> SingleExtraTrack::computeResult_impl() const
{
    assert (num_inside_ >= num_extra_ + num_ok_);

    unsigned int num_total = num_extra_ + num_ok_;

    if (num_total == 0)
        return {};

    boost::optional<double> result = (double)num_extra_ / (double)num_total;

    logdbg << "SingleTrack: updateProb: result_id " << result_id_ << " num_extra " << num_extra_ << " num_ok " << num_ok_;
}

/**
*/
bool SingleExtraTrack::hasIssues_impl() const
{
    return num_extra_ > 0;
}

/**
*/
std::vector<std::string> SingleExtraTrack::targetTableHeadersCustom() const
{
    return { "#Tst", "Ign.", "#Check", "#OK", "#Extra" };
}

/**
*/
std::vector<QVariant> SingleExtraTrack::targetTableValuesCustom() const
{
    return { target_->numTstUpdates(), isIgnored(), num_extra_ + num_ok_, num_ok_, num_extra_ };
}

/**
*/
std::vector<Single::TargetInfo> SingleExtraTrack::targetInfos() const
{
    return { TargetInfo("#Tst [1]", "Number of test updates"              , target_->numTstUpdates()),
             TargetInfo("Ign."    , "Ignore target"                       , isIgnored()             ),
             TargetInfo("#Check." , "Number of checked test track updates", num_extra_ + num_ok_    ),
             TargetInfo("#OK."    , "Number of OK test track updates"     , num_ok_                 ),
             TargetInfo("#Extra"  , "Number of extra test track updates"  , num_extra_              ) };
}

/**
*/
std::vector<std::string> SingleExtraTrack::detailHeaders() const
{
    return {"ToD", "Inside", "TN", "Extra", "Comment"};
}

/**
*/
std::vector<QVariant> SingleExtraTrack::detailValues(const EvaluationDetail& detail,
                                                     const EvaluationDetail* parent_detail) const
{
    return { Utils::Time::toString(detail.timestamp()).c_str(),
             detail.getValue(DetailKey::Inside),
             detail.getValue(DetailKey::TrackNum),
             detail.getValue(DetailKey::Extra),
             detail.comments().generalComment().c_str() };
}

/**
*/
bool SingleExtraTrack::detailIsOk(const EvaluationDetail& detail) const
{
    auto is_extra = detail.getValueAs<bool>(DetailKey::Extra);
    assert(is_extra.has_value());

    return !is_extra.value();
}

/**
*/
void SingleExtraTrack::addAnnotationForDetail(nlohmann::json& annotations_json, 
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
std::map<std::string, std::vector<Single::LayerDefinition>> SingleExtraTrack::gridLayers() const
{
    std::map<std::string, std::vector<Single::LayerDefinition>> layer_defs;

    layer_defs[ requirement_->name() ].push_back(getGridLayerDefBinary());

    return layer_defs;
}

/**
*/
void SingleExtraTrack::addValuesToGrid(Grid2D& grid, const std::string& layer) const
{
    if (layer == requirement_->name())
    {
        addValuesToGridBinary(grid, EvaluationRequirementResult::SingleExtraTrack::DetailKey::Extra, true, false);
    }
}

}
