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

#include "eval/results/position/positionbase.h"
#include "eval/requirement/base/base.h"
#include "evaluationmanager.h"

#include "histograminitializer.h"
#include "histogram.h"
#include "viewpointgenerator.h"

#include "logger.h"

using namespace std;
using namespace Utils;

namespace EvaluationRequirementResult
{

/****************************************************************************
 * SinglePositionBase
 ****************************************************************************/

/**
*/
SinglePositionBase::SinglePositionBase(const std::string& result_type,
                                       const std::string& result_id, 
                                       std::shared_ptr<EvaluationRequirement::Base> requirement,
                                       const SectorLayer& sector_layer,
                                       unsigned int utn, 
                                       const EvaluationTargetData* target, 
                                       EvaluationManager& eval_man,
                                       const EvaluationDetails& details,
                                       unsigned int num_pos, 
                                       unsigned int num_no_ref,
                                       unsigned int num_pos_outside, 
                                       unsigned int num_pos_inside,
                                       unsigned int num_passed, 
                                       unsigned int num_failed,
                                       vector<double> values)
:   Single("SinglePositionAcross", result_id, requirement, sector_layer, utn, target, eval_man, details)
,   num_pos_        (num_pos)
,   num_no_ref_     (num_no_ref)
,   num_pos_outside_(num_pos_outside)
,   num_pos_inside_ (num_pos_inside)
,   num_passed_     (num_passed)
,   num_failed_     (num_failed)
,   values_         (values)
{
}

/**
*/
unsigned int SinglePositionBase::numPassed() const
{
    return num_passed_;
}

/**
*/
unsigned int SinglePositionBase::numFailed() const
{
    return num_failed_;
}

/**
*/
unsigned int SinglePositionBase::numPosOutside() const
{
    return num_pos_outside_;
}

/**
*/
unsigned int SinglePositionBase::numPosInside() const
{
    return num_pos_inside_;
}

/**
*/
unsigned int SinglePositionBase::numPos() const
{
    return num_pos_;
}

/**
*/
unsigned int SinglePositionBase::numNoRef() const
{
    return num_no_ref_;
}

/**
*/
const vector<double>& SinglePositionBase::values() const
{
    return values_;
}

/**
*/
void SinglePositionBase::addCustomAnnotations(nlohmann::json& json_annotations)
{
    if (!json_annotations.is_array())
        return;

    HistogramT<double> histogram;

    HistogramConfig config;
    config.num_bins = eval_man_.settings().histogram_num_bins;
    config.type     = HistogramConfig::Type::Range;

    HistogramInitializer<double> init;
    init.scan(values_);
    init.initHistogram(histogram, config);

    histogram.add(values_);

    auto hraw = histogram.toRaw();

    ViewPointGenAnnotation annotation("", true);

    std::string name = reqGrpId() + ":" + resultId();

    std::unique_ptr<ViewPointGenFeatureHistogram> feat_h;
    feat_h.reset(new ViewPointGenFeatureHistogram(hraw, name, QColor(0, 0, 255)));
    feat_h->setName(name);

    annotation.addFeature(std::move(feat_h));

    nlohmann::json feat_json;
    annotation.toJSON(feat_json);

    json_annotations.push_back(feat_json);
}

/****************************************************************************
 * JoinedPositionBase
 ****************************************************************************/

/**
*/
JoinedPositionBase::JoinedPositionBase(const std::string& result_type,
                                       const std::string& result_id, 
                                       std::shared_ptr<EvaluationRequirement::Base> requirement,
                                       const SectorLayer& sector_layer, 
                                       EvaluationManager& eval_man)
: Joined(result_type, result_id, requirement, sector_layer, eval_man)
{
}

vector<double> JoinedPositionBase::values() const
{
    vector<double> values;

    for (auto& result_it : results_)
    {
        SinglePositionBase* single_result = dynamic_cast<SinglePositionBase*>(result_it.get());
        assert (single_result);

        if (!single_result->use())
            continue;

        values.insert(values.end(), single_result->values().begin(), single_result->values().end());
    }

    return values;
}

void JoinedPositionBase::updateToChanges_impl()
{
    loginf << "JoinedPositionBase: updateToChanges_impl";

    num_pos_         = 0;
    num_no_ref_      = 0;
    num_pos_outside_ = 0;
    num_pos_inside_  = 0;
    num_failed_      = 0;
    num_passed_      = 0;

    for (auto& result_it : results_)
    {
        std::shared_ptr<SinglePositionBase> single_result =
                std::static_pointer_cast<SinglePositionBase>(result_it);
        assert (single_result);

        single_result->setInterestFactor(0);

        if (!single_result->use())
            continue;

        num_pos_         += single_result->numPos();
        num_no_ref_      += single_result->numNoRef();
        num_pos_outside_ += single_result->numPosOutside();
        num_pos_inside_  += single_result->numPosInside();
        num_passed_      += single_result->numPassed();
        num_failed_      += single_result->numFailed();
    }

}

/**
*/
void JoinedPositionBase::addCustomAnnotations(nlohmann::json& json_annotations)
{
    if (!json_annotations.is_array())
        return;

    HistogramT<double> histogram;

    HistogramConfig config;
    config.num_bins = eval_man_.settings().histogram_num_bins;
    config.type     = HistogramConfig::Type::Range;

    auto v = values();

    HistogramInitializer<double> init;
    init.scan(v);
    init.initHistogram(histogram, config);

    histogram.add(v);

    auto hraw = histogram.toRaw();

    ViewPointGenAnnotation annotation("", true);

    std::string name = reqGrpId() + ":" + resultId();

    std::unique_ptr<ViewPointGenFeatureHistogram> feat_h;
    feat_h.reset(new ViewPointGenFeatureHistogram(hraw, name, QColor(0, 0, 255)));
    feat_h->setName(name);

    annotation.addFeature(std::move(feat_h));

    nlohmann::json feat_json;
    annotation.toJSON(feat_json);

    json_annotations.push_back(feat_json);
}

}
