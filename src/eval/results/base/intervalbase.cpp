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

#include "eval/results/base/intervalbase.h"
#include "eval/results/evaluationdetail.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"

#include "eval/requirement/base/base.h"
#include "eval/requirement/base/intervalbase.h"

#include "evaluationtargetdata.h"
#include "evaluationmanager.h"

#include "logger.h"
#include "stringconv.h"
#include "viewpoint.h"
#include "sectorlayer.h"

#include <cassert>

using namespace std;
using namespace Utils;
using namespace nlohmann;

namespace EvaluationRequirementResult
{

/***********************************************************************************************
 * SingleIntervalBase
 ***********************************************************************************************/

/**
*/
SingleIntervalBase::SingleIntervalBase(const std::string& result_type, 
                                       const std::string& result_id, 
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
    :   IntervalBase(sum_uis, missed_uis)
    ,   SingleProbabilityBase(result_type, result_id, requirement, sector_layer, utn, target, eval_man, details)
    ,   ref_periods_(ref_periods)
    ,   ref_updates_(ref_updates)
{
}

/**
*/
boost::optional<double> SingleIntervalBase::computeResult_impl() const
{
    assert (missed_uis_ <= sum_uis_);

    if (sum_uis_ == 0)
        return {};

    logdbg << type() << ": updatePD: utn " << utn_ << " missed_uis " << missed_uis_ << " sum_uis " << sum_uis_;

    return (1.0 - (double)missed_uis_ / (double)sum_uis_);
}

/**
*/
unsigned int SingleIntervalBase::numIssues() const
{
    return missed_uis_;
}

/**
*/
std::vector<std::string> SingleIntervalBase::targetTableHeadersCustom() const
{
    return { "#EUIs", "#MUIs" };
}

/**
*/
std::vector<QVariant> SingleIntervalBase::targetTableValuesCustom() const
{
    return { sum_uis_, missed_uis_ };
}

/**
*/
std::vector<Single::TargetInfo> SingleIntervalBase::targetInfos() const
{
    std::vector<Single::TargetInfo> infos = { { "#EUIs [1]", "Expected Update Intervals", sum_uis_   },
                                              { "#MUIs [1]", "Missed Update Intervals"  , missed_uis_} };
    size_t inside_sectors = 0;

    for (unsigned int cnt=0; cnt < ref_periods_.size(); ++cnt)
    {
        //skip outside sectors
        if (ref_periods_.period(cnt).type() == TimePeriod::Type::OutsideSector)
            continue;

        ++inside_sectors;

        infos.emplace_back(("Reference Period " + std::to_string(cnt)).c_str(), "Time inside sector", ref_periods_.period(cnt).str().c_str());
    }

    if (inside_sectors == 0)
        infos.emplace_back("Reference Period", "Time inside sector", "None");

    return infos;
}

/**
*/
std::vector<std::string> SingleIntervalBase::detailHeaders() const
{
    return {"ToD", "DToD", "Ref.", "#MUIs", "Comment"};
}

/**
*/
std::vector<QVariant> SingleIntervalBase::detailValues(const EvaluationDetail& detail,
                                                       const EvaluationDetail* parent_detail) const
{
    auto     d_tod     = detail.getValue(DetailKey::DiffTOD);
    QVariant d_tod_str = d_tod.isValid() ? QVariant(String::timeStringFromDouble(d_tod.toFloat()).c_str()) : QVariant();

    return { Utils::Time::toString(detail.timestamp()).c_str(),
             d_tod_str,
             detail.getValue(DetailKey::RefExists),
             detail.getValue(DetailKey::MissedUIs),
             detail.comments().generalComment().c_str() };
}

/**
*/
bool SingleIntervalBase::detailIsOk(const EvaluationDetail& detail) const
{
    auto check_failed = detail.getValueAsOrAssert<bool>(EvaluationRequirementResult::SingleIntervalBase::DetailKey::MissOccurred);
    return !check_failed;
}

/**
*/
void SingleIntervalBase::addAnnotationForDetail(nlohmann::json& annotations_json, 
                                           const EvaluationDetail& detail, 
                                           TargetAnnotationType type,
                                           bool is_ok) const
{
    if (detail.numPositions() == 1)
        return;

    assert (detail.numPositions() >= 2);

    if (type == TargetAnnotationType::Highlight)
    {
        addAnnotationDistance(annotations_json, detail, TypeHighlight);
    }
    // else if (type == TargetAnnotationType::Overview)
    // {
    //     auto idx0 = detail.getValueAs<unsigned int>(EvaluationRequirementResult::SingleIntervalBase::DetailKey::RefUpdateStartIndex);
    //     auto idx1 = detail.getValueAs<unsigned int>(EvaluationRequirementResult::SingleIntervalBase::DetailKey::RefUpdateEndIndex);

    //     assert(idx0.has_value() && idx1.has_value());

    //     for (unsigned int idx = idx0.value(); idx <= idx1.value(); ++idx)
    //         addAnnotationPos(annotations_json, detail.position(idx), ok ? TypeOk : TypeError);

    //     for (unsigned int idx = idx0.value() + 1; idx <= idx1.value(); ++idx)
    //         addAnnotationLine(annotations_json, detail.position(idx - 1), detail.position(idx), ok ? TypeOk : TypeError);
    // }
    else if (type == TargetAnnotationType::TargetOverview)
    {
        addAnnotationDistance(annotations_json, detail, is_ok ? TypeOk : TypeError);
    }
}

/**
*/
std::map<std::string, std::vector<Single::LayerDefinition>> SingleIntervalBase::gridLayers() const
{
    std::map<std::string, std::vector<Single::LayerDefinition>> layer_defs;

    layer_defs[ requirement_->name() ].push_back(getGridLayerDefBinary());

    return layer_defs;
}

/**
*/
void SingleIntervalBase::addValuesToGrid(Grid2D& grid, const std::string& layer) const
{
    if (layer == requirement_->name())
    {
        for (auto& detail_it : getDetails())
        {
            auto check_failed = detail_it.getValueAsOrAssert<bool>(
                        EvaluationRequirementResult::SingleIntervalBase::DetailKey::MissOccurred);

            if (detail_it.numPositions() == 1)
                continue;

            assert (detail_it.numPositions() >= 2);

            auto idx0 = detail_it.getValueAs<unsigned int>(EvaluationRequirementResult::SingleIntervalBase::DetailKey::RefUpdateStartIndex).value();
            auto idx1 = detail_it.getValueAs<unsigned int>(EvaluationRequirementResult::SingleIntervalBase::DetailKey::RefUpdateEndIndex).value();

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

/***********************************************************************************************
 * JoinedIntervalBase
 ***********************************************************************************************/

/**
*/
JoinedIntervalBase::JoinedIntervalBase(const std::string& result_type, 
                                       const std::string& result_id, 
                                       std::shared_ptr<EvaluationRequirement::Base> requirement,
                                       const SectorLayer& sector_layer, 
                                       EvaluationManager& eval_man)
:   JoinedProbabilityBase(result_type, result_id, requirement, sector_layer, eval_man)
{
}

/**
*/
unsigned int JoinedIntervalBase::numIssues() const
{
    return missed_uis_;
}

/**
*/
unsigned int JoinedIntervalBase::numUpdates() const
{
    return sum_uis_;
}

/**
*/
void JoinedIntervalBase::clearResults_impl() 
{
    sum_uis_    = 0;
    missed_uis_ = 0;
}

/**
*/
void JoinedIntervalBase::accumulateSingleResult(const std::shared_ptr<Single>& single_result, bool first, bool last)
{
    std::shared_ptr<SingleIntervalBase> single_interval = std::static_pointer_cast<SingleIntervalBase>(single_result);

    sum_uis_    += single_interval->sumUIs();
    missed_uis_ += single_interval->missedUIs();
}

/**
*/
boost::optional<double> JoinedIntervalBase::computeResult_impl() const
{
    loginf << "JoinedIntervalBase: computeResult_impl:" << type()
            << " sum_uis_ " << sum_uis_
            << " missed_uis_ " << missed_uis_;

    assert (missed_uis_ <= sum_uis_);

    if (sum_uis_ == 0)
        return {};

    return 1.0 - (double)missed_uis_ / (double)(sum_uis_);
}

/**
*/
std::vector<Joined::SectorInfo> JoinedIntervalBase::sectorInfos() const
{
    return { { "#Updates/#EUIs [1]", "Total number update intervals"     , sum_uis_    },
             { "#MUIs [1]"         , "Number of missed update intervals" , missed_uis_ } };
}

}
