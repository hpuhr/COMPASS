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
 * PositionBase
 ****************************************************************************/

/**
*/
PositionBase::PositionBase() = default;

/**
*/
PositionBase::PositionBase(unsigned int num_pos,
                           unsigned int num_no_ref,
                           unsigned int num_pos_outside,
                           unsigned int num_pos_inside,
                           unsigned int num_passed,
                           unsigned int num_failed)
:   num_pos_        (num_pos)
,   num_no_ref_     (num_no_ref)
,   num_pos_outside_(num_pos_outside)
,   num_pos_inside_ (num_pos_inside)
,   num_passed_     (num_passed)
,   num_failed_     (num_failed)
{
}

/**
*/
unsigned int PositionBase::numPassed() const
{
    return num_passed_;
}

/**
*/
unsigned int PositionBase::numFailed() const
{
    return num_failed_;
}

/**
*/
unsigned int PositionBase::numPosOutside() const
{
    return num_pos_outside_;
}

/**
*/
unsigned int PositionBase::numPosInside() const
{
    return num_pos_inside_;
}

/**
*/
unsigned int PositionBase::numPos() const
{
    return num_pos_;
}

/**
*/
unsigned int PositionBase::numNoRef() const
{
    return num_no_ref_;
}

/**
*/
const ValueAccumulator& PositionBase::accumulator() const
{
    return accumulator_;
}

/****************************************************************************
 * SinglePositionBaseCommon
 ****************************************************************************/

/**
*/
SinglePositionBaseCommon::SinglePositionBaseCommon(unsigned int num_pos,
                                                   unsigned int num_no_ref,
                                                   unsigned int num_pos_outside,
                                                   unsigned int num_pos_inside,
                                                   unsigned int num_passed,
                                                   unsigned int num_failed)
:   PositionBase(num_pos, num_no_ref, num_pos_outside, num_pos_inside, num_passed, num_failed)
{
}

/**
*/
boost::optional<double> SinglePositionBaseCommon::common_computeResult() const
{
    assert (num_no_ref_ <= num_pos_);
    assert (num_pos_ - num_no_ref_ == num_pos_inside_ + num_pos_outside_);

    auto values = getValues();

    assert (values.size() == num_passed_ + num_failed_);

    unsigned int num_distances = values.size();

    assert (num_passed_ <= num_distances);

    if (num_distances > 0)
        accumulator_.accumulate(values, true);

    return computeFinalResultValue();
}

/**
*/
unsigned int SinglePositionBaseCommon::common_numIssues() const
{
    return num_failed_ > 0;
}

/**
*/
bool SinglePositionBaseCommon::common_detailIsOk(const EvaluationDetail& detail) const
{
    auto check_passed = detail.getValueAs<bool>(DetailKey::CheckPassed);
    assert(check_passed.has_value());

    return check_passed.value();
}

/****************************************************************************
 * SinglePositionProbabilityBase
 ****************************************************************************/

/**
*/
SinglePositionProbabilityBase::SinglePositionProbabilityBase(const std::string& result_type,
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
                                                             unsigned int num_failed)
:   SingleProbabilityBase(result_type, result_id, requirement, sector_layer, utn, target, eval_man, details)
,   SinglePositionBaseCommon(num_pos, num_no_ref, num_pos_outside, num_pos_inside, num_passed, num_failed)
{
}

/**
*/
std::vector<double> SinglePositionProbabilityBase::getValues() const
{
    std::vector<double> values;
    values.reserve(getDetails().size());

    auto func = [ & ] (const EvaluationDetail& detail, const EvaluationDetail* parent_detail, int idx0, int idx1)
    {
        auto offset_valid = detail.getValueAs<bool>(ValueValid);
        if (!offset_valid.has_value() || !offset_valid.value())
            return;

        auto offset = detail.getValueAsOrAssert<float>(Value);
        values.push_back(offset);
    };

    iterateDetails(func);

    values.shrink_to_fit();
}

/**
 * Result value for all probability based position requirement results.
 */
boost::optional<double> SinglePositionProbabilityBase::computeFinalResultValue() const
{
    auto total = num_passed_ + num_failed_;

    if (total == 0)
        return {};

    return (double)num_passed_ / (double)total;
}

/**
*/
boost::optional<double> SinglePositionProbabilityBase::computeResult_impl() const
{
    return common_computeResult();
}

/**
*/
unsigned int SinglePositionProbabilityBase::numIssues() const
{
    return common_numIssues();
}

/**
*/
bool SinglePositionProbabilityBase::detailIsOk(const EvaluationDetail& detail) const
{
    return common_detailIsOk(detail);
}

/**
*/
void SinglePositionProbabilityBase::addAnnotationForDetail(nlohmann::json& annotations_json, 
                                                           const EvaluationDetail& detail, 
                                                           TargetAnnotationType type,
                                                           bool is_ok) const
{
    assert (detail.numPositions() >= 1);

    if (detail.numPositions() == 1) // no ref pos
        return;

    if (type == TargetAnnotationType::Highlight)
    {
        addAnnotationDistance(annotations_json, detail, AnnotationType::TypeHighlight, true, false);
    }
    else if (type == TargetAnnotationType::TargetOverview)
    {
        addAnnotationDistance(annotations_json, detail, is_ok ? AnnotationType::TypeOk : AnnotationType::TypeError, true, false);
    }
}

/**
*/
std::map<std::string, std::vector<Single::LayerDefinition>> SinglePositionProbabilityBase::gridLayers() const
{
    std::map<std::string, std::vector<Single::LayerDefinition>> layer_defs;

    layer_defs[ requirement_->name() ].push_back(getGridLayerDefBinary());

    return layer_defs;
}

/**
*/
void SinglePositionProbabilityBase::addValuesToGrid(Grid2D& grid, const std::string& layer) const
{
    if (layer == requirement_->name())
    {
        addValuesToGridBinary(grid, SinglePositionBaseCommon::DetailKey::CheckPassed);
    }
}

/**
*/
// void SinglePositionBase::addCustomAnnotations(nlohmann::json& json_annotations)
// {
//     if (!json_annotations.is_array())
//         return;

//     HistogramT<double> histogram;

//     HistogramConfig config;
//     config.num_bins = eval_man_.settings().histogram_num_bins;
//     config.type     = HistogramConfig::Type::Range;

//     HistogramInitializer<double> init;
//     init.scan(values_);
//     init.initHistogram(histogram, config);

//     histogram.add(values_);

//     auto hraw = histogram.toRaw();

//     ViewPointGenAnnotation annotation("", true);

//     std::string name = reqGrpId() + ":" + resultId();

//     std::unique_ptr<ViewPointGenFeatureHistogram> feat_h;
//     feat_h.reset(new ViewPointGenFeatureHistogram(hraw, name, QColor(0, 0, 255)));
//     feat_h->setName(name);

//     annotation.addFeature(std::move(feat_h));

//     nlohmann::json feat_json;
//     annotation.toJSON(feat_json);

//     json_annotations.push_back(feat_json);
// }


/****************************************************************************
 * SinglePositionValueBase
 ****************************************************************************/

/**
*/
SinglePositionValueBase::SinglePositionValueBase(const std::string& result_type,
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
                                                 unsigned int num_failed)
:   SingleProbabilityBase(result_type, result_id, requirement, sector_layer, utn, target, eval_man, details)
,   SinglePositionBaseCommon(num_pos, num_no_ref, num_pos_outside, num_pos_inside, num_passed, num_failed)
{
}

/**
*/
std::vector<double> SinglePositionProbabilityBase::getValues() const
{
    std::vector<double> values;
    values.reserve(getDetails().size());

    auto func = [ & ] (const EvaluationDetail& detail, const EvaluationDetail* parent_detail, int idx0, int idx1)
    {
        auto offset_valid = detail.getValueAs<bool>(ValueValid);
        if (!offset_valid.has_value() || !offset_valid.value())
            return;

        auto offset = detail.getValueAsOrAssert<float>(Value);
        values.push_back(offset);
    };

    iterateDetails(func);

    values.shrink_to_fit();
}

/**
*/
boost::optional<double> SinglePositionValueBase::computeResult_impl() const
{
    return common_computeResult();
}

/**
*/
unsigned int SinglePositionValueBase::numIssues() const
{
    return common_numIssues();
}

/**
*/
bool SinglePositionValueBase::detailIsOk(const EvaluationDetail& detail) const
{
    return common_detailIsOk(detail);
}

/**
*/
void SinglePositionValueBase::addAnnotationForDetail(nlohmann::json& annotations_json, 
                                                     const EvaluationDetail& detail, 
                                                     TargetAnnotationType type,
                                                     bool is_ok) const
{
    assert (detail.numPositions() >= 1);

    if (detail.numPositions() == 1) // no ref pos
        return;

    if (type == TargetAnnotationType::Highlight)
    {
        addAnnotationDistance(annotations_json, detail, AnnotationType::TypeHighlight, true, false);
    }
    else if (type == TargetAnnotationType::TargetOverview)
    {
        addAnnotationDistance(annotations_json, detail, is_ok ? AnnotationType::TypeOk : AnnotationType::TypeError, true, false);
    }
}

/**
*/
std::map<std::string, std::vector<Single::LayerDefinition>> SinglePositionValueBase::gridLayers() const
{
    std::map<std::string, std::vector<Single::LayerDefinition>> layer_defs;

    layer_defs[ requirement_->name() ].push_back(getGridLayerDefBinary());

    return layer_defs;
}

/**
*/
void SinglePositionValueBase::addValuesToGrid(Grid2D& grid, const std::string& layer) const
{
    if (layer == requirement_->name())
    {
        addValuesToGridBinary(grid, SinglePositionBaseCommon::DetailKey::CheckPassed);
    }
}

/****************************************************************************
 * JoinedPositionBase
 ****************************************************************************/

/**
*/
JoinedPositionBase::JoinedPositionBase(const std::string& csv_header)
:   csv_header_(csv_header)
{
}

/**
*/
unsigned int JoinedPositionBase::common_numIssues() const
{
    return num_failed_;
}

/**
*/
unsigned int JoinedPositionBase::common_numUpdates() const
{
    return num_passed_ + num_failed_;
}

/**
*/
void JoinedPositionBase::common_clearResults()
{
    num_pos_         = 0;
    num_no_ref_      = 0;
    num_pos_outside_ = 0;
    num_pos_inside_  = 0;
    num_failed_      = 0;
    num_passed_      = 0;

    accumulator_.reset();
}

/**
*/
void JoinedPositionBase::common_accumulateSingleResult(const PositionBase& single_result,
                                                       const std::vector<double>& values)
{
    num_pos_         += single_result.numPos();
    num_no_ref_      += single_result.numNoRef();
    num_pos_outside_ += single_result.numPosOutside();
    num_pos_inside_  += single_result.numPosInside();
    num_passed_      += single_result.numPassed();
    num_failed_      += single_result.numFailed();

    accumulator_.accumulate(values);
}

/**
*/
boost::optional<double> JoinedPositionBase::common_computeResult() const
{
    assert (num_no_ref_ <= num_pos_);
    assert (num_pos_ - num_no_ref_ == num_pos_inside_ + num_pos_outside_);

    // nothing to do

    return computeFinalResultValue();
}

/**
*/
bool JoinedPositionBase::common_exportAsCSV(std::ofstream& strm) const
{
    strm << csv_header_ << "\n";

    auto values = getValues();

    for (auto v : values)
        strm << v << "\n";

    if (!strm)
        return false;

    return true;
}

/****************************************************************************
 * JoinedPositionProbabilityBase
 ****************************************************************************/

/**
*/
JoinedPositionProbabilityBase::JoinedPositionProbabilityBase(const std::string& result_type,
                                                             const std::string& result_id, 
                                                             std::shared_ptr<EvaluationRequirement::Base> requirement,
                                                             const SectorLayer& sector_layer, 
                                                             EvaluationManager& eval_man,
                                                             const std::string& csv_header)
:   JoinedPositionBase(csv_header)
,   JoinedProbabilityBase(result_type, result_id, requirement, sector_layer, eval_man)
{
}

/**
*/
std::vector<double> JoinedPositionProbabilityBase::getValues() const
{
    std::vector<double> values;

    auto func = [ & ] (const std::shared_ptr<Single>& result)
    {
        std::shared_ptr<SinglePositionProbabilityBase> res = std::static_pointer_cast<SinglePositionProbabilityBase>(result);
        auto v = res->getValues();

        values.insert(values.end(), v.begin(), v.end());
    };

    iterateSingleResults({}, func, {});

    return values;
}

/**
 * Result value for all probability based position requirement results.
 */
boost::optional<double> SinglePositionProbabilityBase::computeFinalResultValue() const
{
    auto total = num_passed_ + num_failed_;

    if (total == 0)
        return {};

    return (double)num_passed_ / (double)total;
}

/**
*/
unsigned int JoinedPositionProbabilityBase::numIssues() const
{
    return common_numIssues();
}

/**
*/
unsigned int JoinedPositionProbabilityBase::numUpdates() const
{
    return common_numUpdates();
}

/**
*/
void JoinedPositionProbabilityBase::clearResults_impl() 
{
    common_clearResults();
}

/**
*/
void JoinedPositionProbabilityBase::accumulateSingleResult(const std::shared_ptr<Single>& single_result, bool first, bool last)
{
    std::shared_ptr<SinglePositionProbabilityBase> single = std::static_pointer_cast<SinglePositionProbabilityBase>(single_result);

    common_accumulateSingleResult(*single, single->getValues());
}

/**
*/
boost::optional<double> JoinedPositionProbabilityBase::computeResult_impl() const
{
    loginf << "JoinedPositionProbabilityBase: computeResult_impl:" << type()
            << " num_pos " << num_pos_
            << " num_no_ref " << num_no_ref_
            << " num_failed " << num_failed_
            << " num_passed " << num_passed_;

    return common_computeResult();
}

/**
*/
bool JoinedPositionProbabilityBase::exportAsCSV(std::ofstream& strm) const
{
    return common_exportAsCSV(strm);
}

/**
*/
// void JoinedPositionBase::addCustomAnnotations(nlohmann::json& json_annotations)
// {
//     if (!json_annotations.is_array())
//         return;

//     HistogramT<double> histogram;

//     HistogramConfig config;
//     config.num_bins = eval_man_.settings().histogram_num_bins;
//     config.type     = HistogramConfig::Type::Range;

//     auto v = values();

//     HistogramInitializer<double> init;
//     init.scan(v);
//     init.initHistogram(histogram, config);

//     histogram.add(v);

//     auto hraw = histogram.toRaw();

//     ViewPointGenAnnotation annotation("", true);

//     std::string name = reqGrpId() + ":" + resultId();

//     std::unique_ptr<ViewPointGenFeatureHistogram> feat_h;
//     feat_h.reset(new ViewPointGenFeatureHistogram(hraw, name, QColor(0, 0, 255)));
//     feat_h->setName(name);

//     annotation.addFeature(std::move(feat_h));

//     nlohmann::json feat_json;
//     annotation.toJSON(feat_json);

//     json_annotations.push_back(feat_json);
// }

/****************************************************************************
 * JoinedPositionValueBase
 ****************************************************************************/

/**
*/
JoinedPositionValueBase::JoinedPositionValueBase(const std::string& result_type,
                                                 const std::string& result_id, 
                                                 std::shared_ptr<EvaluationRequirement::Base> requirement,
                                                 const SectorLayer& sector_layer, 
                                                 EvaluationManager& eval_man,
                                                 const std::string& csv_header)
:   JoinedPositionBase(csv_header)
,   Joined(result_type, result_id, requirement, sector_layer, eval_man)
{
}

/**
*/
std::vector<double> JoinedPositionValueBase::getValues() const
{
    std::vector<double> values;

    auto func = [ & ] (const std::shared_ptr<Single>& result)
    {
        std::shared_ptr<SinglePositionValueBase> res = std::static_pointer_cast<SinglePositionValueBase>(result);
        auto v = res->getValues();

        values.insert(values.end(), v.begin(), v.end());
    };

    iterateSingleResults({}, func, {});

    return values;
}

/**
*/
unsigned int JoinedPositionValueBase::numIssues() const
{
    return common_numIssues();
}

/**
*/
unsigned int JoinedPositionValueBase::numUpdates() const
{
    return common_numUpdates();
}

/**
*/
void JoinedPositionValueBase::clearResults_impl() 
{
    common_clearResults();
}

/**
*/
void JoinedPositionValueBase::accumulateSingleResult(const std::shared_ptr<Single>& single_result, bool first, bool last)
{
    std::shared_ptr<SinglePositionValueBase> single = std::static_pointer_cast<SinglePositionValueBase>(single_result);

    common_accumulateSingleResult(*single, single->getValues());
}

/**
*/
boost::optional<double> JoinedPositionValueBase::computeResult_impl() const
{
    loginf << "JoinedPositionValueBase: computeResult_impl:" << type()
            << " num_pos " << num_pos_
            << " num_no_ref " << num_no_ref_
            << " num_failed " << num_failed_
            << " num_passed " << num_passed_;

    return common_computeResult();
}

/**
*/
bool JoinedPositionValueBase::exportAsCSV(std::ofstream& strm) const
{
    return common_exportAsCSV(strm);
}

/**
*/
// void JoinedPositionBase::addCustomAnnotations(nlohmann::json& json_annotations)
// {
//     if (!json_annotations.is_array())
//         return;

//     HistogramT<double> histogram;

//     HistogramConfig config;
//     config.num_bins = eval_man_.settings().histogram_num_bins;
//     config.type     = HistogramConfig::Type::Range;

//     auto v = values();

//     HistogramInitializer<double> init;
//     init.scan(v);
//     init.initHistogram(histogram, config);

//     histogram.add(v);

//     auto hraw = histogram.toRaw();

//     ViewPointGenAnnotation annotation("", true);

//     std::string name = reqGrpId() + ":" + resultId();

//     std::unique_ptr<ViewPointGenFeatureHistogram> feat_h;
//     feat_h.reset(new ViewPointGenFeatureHistogram(hraw, name, QColor(0, 0, 255)));
//     feat_h->setName(name);

//     annotation.addFeature(std::move(feat_h));

//     nlohmann::json feat_json;
//     annotation.toJSON(feat_json);

//     json_annotations.push_back(feat_json);
// }

}
