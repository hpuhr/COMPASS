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

#include "eval/results/detection/single.h"
#include "eval/results/detection/joined.h"
#include "eval/results/evaluationdetail.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"

#include "eval/requirement/base/base.h"
#include "eval/requirement/detection/detection.h"

#include "evaluationtargetdata.h"
#include "evaluationmanager.h"

#include "logger.h"
#include "stringconv.h"
#include "viewpoint.h"

#include <cassert>

using namespace std;
using namespace Utils;
using namespace nlohmann;

namespace EvaluationRequirementResult
{
 
/**
*/
SingleDetection::SingleDetection(const std::string& result_id, 
                                 std::shared_ptr<EvaluationRequirement::Base> requirement,
                                 const SectorLayer& sector_layer,
                                 unsigned int utn,
                                 const EvaluationTargetData* target,
                                 EvaluationManager& eval_man,
                                 const EvaluationDetails& details,
                                 int sum_uis,
                                 int missed_uis,
                                 TimePeriodCollection ref_periods,
                                 const std::vector<dbContent::TargetPosition>& ref_updates)
    :   SingleProbabilityBase("SingleDetection", result_id, requirement, sector_layer, utn, target, eval_man, details)
    ,   sum_uis_    (sum_uis)
    ,   missed_uis_ (missed_uis)
    ,   ref_periods_(ref_periods)
    ,   ref_updates_(ref_updates)
{
    computeResult();
}

/**
*/
std::shared_ptr<Joined> SingleDetection::createEmptyJoined(const std::string& result_id)
{
    return make_shared<JoinedDetection>(result_id, requirement_, sector_layer_, eval_man_);
}

/**
*/
unsigned int SingleDetection::sumUIs() const
{
    return sum_uis_;
}

/**
*/
unsigned int SingleDetection::missedUIs() const
{
    return missed_uis_;
}

/**
*/
boost::optional<double> SingleDetection::computeResult_impl() const
{
    if (!sum_uis_)
        return {};

    logdbg << "SingleDetection: updatePD: utn " << utn_ << " missed_uis " << missed_uis_ << " sum_uis " << sum_uis_;

    assert (missed_uis_ <= sum_uis_);

    std::shared_ptr<EvaluationRequirement::Detection> req =
            std::static_pointer_cast<EvaluationRequirement::Detection>(requirement_);
    assert (req);

    return (1.0 - ((double)missed_uis_/(double)(sum_uis_)));
}

/**
*/
bool SingleDetection::hasIssues_impl() const
{
    return (missed_uis_ > 0);
}

/**
*/
std::vector<std::string> SingleDetection::targetTableHeadersCustom() const
{
    return { "#EUIs", "#MUIs" };
}

/**
*/
std::vector<QVariant> SingleDetection::targetTableValuesCustom() const
{
    return { sum_uis_, missed_uis_ };
}

/**
*/
std::vector<Single::TargetInfo> SingleDetection::targetInfos() const
{
    std::shared_ptr<EvaluationRequirement::Detection> req = std::static_pointer_cast<EvaluationRequirement::Detection>(requirement_);
    assert (req);

    std::vector<TargetInfo> infos = { TargetInfo("#EUIs [1]", "Expected Update Intervals", sum_uis_   ),
                                      TargetInfo("#MUIs [1]", "Missed Update Intervals"  , missed_uis_) };

    for (unsigned int cnt=0; cnt < ref_periods_.size(); ++cnt)
        infos.emplace_back(("Reference Period " + std::to_string(cnt)).c_str(), "Time inside sector", ref_periods_.period(cnt).str().c_str());

    if (!ref_periods_.size())
        infos.emplace_back("Reference Period", "Time inside sector", "None");

    infos.emplace_back("Must hold for any target ", "", req->holdForAnyTarget());
}

/**
*/
std::vector<std::string> SingleDetection::detailHeaders() const
{
    return { "ToD", "DToD", "Ref.", "MUI", "Comment" };
}

/**
*/
std::vector<QVariant> SingleDetection::detailValues(const EvaluationDetail& detail,
                                                    const EvaluationDetail* parent_detail) const
{
    auto d_tod = detail.getValue(DetailKey::DiffTOD);

    return { Utils::Time::toString(detail.timestamp()).c_str(),
             d_tod.isValid() ? QVariant(Utils::String::timeStringFromDouble(d_tod.toFloat()).c_str()) : QVariant(),
             detail.getValue(DetailKey::RefExists),
             detail.getValue(DetailKey::MissedUIs),
             detail.comments().generalComment().c_str() };
}

/**
*/
bool SingleDetection::detailIsOk(const EvaluationDetail& detail) const
{
    return !detail.getValue(DetailKey::MissOccurred).toBool();
}

/**
*/
void SingleDetection::addAnnotationForDetail(nlohmann::json& annotations_json, 
                                             const EvaluationDetail& detail, 
                                             TargetAnnotationType type,
                                             bool is_ok) const
{
    assert (detail.numPositions() >= 1);

    if (type == TargetAnnotationType::Highlight)
    {
        addAnnotationDistance(annotations_json, detail, AnnotationType::TypeHighlight, true, false);
    }
    else if (type == TargetAnnotationType::TargetOverview)
    {
        if (detail.numPositions() >= 2)
            addAnnotationDistance(annotations_json, detail, is_ok ? AnnotationType::TypeOk : AnnotationType::TypeError);
    }
}

/**
*/
std::map<std::string, std::vector<Single::LayerDefinition>> SingleDetection::gridLayers() const
{
    std::map<std::string, std::vector<Single::LayerDefinition>> layer_defs;

    layer_defs[ requirement_->name() ].push_back(getGridLayerDefBinary());

    return layer_defs;
}

/**
*/
void SingleDetection::addValuesToGrid(Grid2D& grid, const std::string& layer) const
{
    if (layer == requirement_->name())
    {
        for (auto& detail_it : getDetails())
        {
            auto check_failed = detail_it.getValueAsOrAssert<bool>(
                        EvaluationRequirementResult::SingleDetection::DetailKey::MissOccurred);

            if (detail_it.numPositions() == 1)
                continue;

            assert (detail_it.numPositions() >= 2);

            auto idx0 = detail_it.getValueAs<unsigned int>(EvaluationRequirementResult::SingleDetection::DetailKey::RefUpdateStartIndex).value();
            auto idx1 = detail_it.getValueAs<unsigned int>(EvaluationRequirementResult::SingleDetection::DetailKey::RefUpdateEndIndex).value();

            size_t n = idx1 - idx0 + 1;

            auto pos_getter = [ & ] (double& x, double& y, size_t idx) 
            { 
                x =  ref_updates_[ idx0 + idx ].longitude_;
                y =  ref_updates_[ idx0 + idx ].latitude_;
            };

            grid.addPoly(pos_getter, n, check_failed ? 1.0 : 0.0);
        }
    }
}

}
