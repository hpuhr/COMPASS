#include "evaluationrequirementdetectionconfig.h"
#include "evaluationrequirementdetectionconfigwidget.h"
#include "evaluationrequirementgroup.h"
#include "evaluationrequirement.h"

using namespace std;

EvaluationRequirementDetectionConfig::EvaluationRequirementDetectionConfig(
        const std::string& class_id, const std::string& instance_id,
        EvaluationRequirementGroup& group, EvaluationStandard& standard)
    : EvaluationRequirementConfig(class_id, instance_id, group, standard)
{
    registerParameter("update_interval", &update_interval_s_, 1);
    registerParameter("minimum_probability", &minimum_probability_, 0.99);

    registerParameter("use_max_gap_interval", &use_max_gap_interval_, true);
    registerParameter("max_gap_interval", &max_gap_interval_s_, 30);

    registerParameter("use_miss_tolerance", &use_miss_tolerance_, false);
    registerParameter("miss_tolerance", &miss_tolerance_s_, 0.01);

}

EvaluationRequirementDetectionConfig::~EvaluationRequirementDetectionConfig()
{

}

void EvaluationRequirementDetectionConfig::addGUIElements(QFormLayout* layout)
{
    assert (layout);

    EvaluationRequirementConfig::addGUIElements(layout);
}

EvaluationRequirementDetectionConfigWidget* EvaluationRequirementDetectionConfig::widget()
{
    if (!widget_)
        widget_.reset(new EvaluationRequirementDetectionConfigWidget(*this));

    return widget_.get();
}

std::shared_ptr<EvaluationRequirement> EvaluationRequirementDetectionConfig::createRequirement()
{
    //EvaluationRequirement req (name_, short_name_, group_.name());

    shared_ptr<EvaluationRequirementDetection> req = make_shared<EvaluationRequirementDetection>(
                name_, short_name_, group_.name(), update_interval_s_, minimum_probability_,
                use_max_gap_interval_, max_gap_interval_s_, use_miss_tolerance_, miss_tolerance_s_);

    return req;
}

float EvaluationRequirementDetectionConfig::updateInterval() const
{
    return update_interval_s_;
}

void EvaluationRequirementDetectionConfig::updateInterval(float value)
{
    update_interval_s_ = value;
}

float EvaluationRequirementDetectionConfig::minimumProbability() const
{
    return minimum_probability_;
}

void EvaluationRequirementDetectionConfig::minimumProbability(float value)
{
    minimum_probability_ = value;
}

bool EvaluationRequirementDetectionConfig::useMaxGapInterval() const
{
    return use_max_gap_interval_;
}

void EvaluationRequirementDetectionConfig::useMaxGapInterval(bool value)
{
    use_max_gap_interval_ = value;
}

float EvaluationRequirementDetectionConfig::maxGapInterval() const
{
    return max_gap_interval_s_;
}

void EvaluationRequirementDetectionConfig::maxGapInterval(float value)
{
    max_gap_interval_s_ = value;
}

bool EvaluationRequirementDetectionConfig::useMissTolerance() const
{
    return use_miss_tolerance_;
}

void EvaluationRequirementDetectionConfig::useMissTolerance(bool value)
{
    use_miss_tolerance_ = value;
}

float EvaluationRequirementDetectionConfig::missTolerance() const
{
    return miss_tolerance_s_;
}

void EvaluationRequirementDetectionConfig::missTolerance(float value)
{
    miss_tolerance_s_ = value;
}
