#include "evaluationrequirementdetectionconfig.h"
#include "evaluationrequirementdetectionconfigwidget.h"
#include "evaluationrequirementgroup.h"
#include "evaluationrequirement.h"

using namespace std;

namespace EvaluationRequirement
{

DetectionConfig::DetectionConfig(
        const std::string& class_id, const std::string& instance_id,
        Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
    : Config(class_id, instance_id, group, standard, eval_man)
{
    registerParameter("update_interval", &update_interval_s_, 1);
    registerParameter("minimum_probability", &minimum_probability_, 0.99);

    registerParameter("use_max_gap_interval", &use_max_gap_interval_, true);
    registerParameter("max_gap_interval", &max_gap_interval_s_, 30);

    registerParameter("use_miss_tolerance", &use_miss_tolerance_, false);
    registerParameter("miss_tolerance", &miss_tolerance_s_, 0.01);

}

DetectionConfig::~DetectionConfig()
{

}

void DetectionConfig::addGUIElements(QFormLayout* layout)
{
    assert (layout);

    Config::addGUIElements(layout);
}

DetectionConfigWidget* DetectionConfig::widget()
{
    if (!widget_)
        widget_.reset(new DetectionConfigWidget(*this));

    return widget_.get();
}

std::shared_ptr<Base> DetectionConfig::createRequirement()
{
    shared_ptr<Detection> req = make_shared<Detection>(
                name_, short_name_, group_.name(), eval_man_, update_interval_s_, minimum_probability_,
                use_max_gap_interval_, max_gap_interval_s_, use_miss_tolerance_, miss_tolerance_s_);

    return req;
}

float DetectionConfig::updateInterval() const
{
    return update_interval_s_;
}

void DetectionConfig::updateInterval(float value)
{
    update_interval_s_ = value;
}

float DetectionConfig::minimumProbability() const
{
    return minimum_probability_;
}

void DetectionConfig::minimumProbability(float value)
{
    minimum_probability_ = value;
}

bool DetectionConfig::useMaxGapInterval() const
{
    return use_max_gap_interval_;
}

void DetectionConfig::useMaxGapInterval(bool value)
{
    use_max_gap_interval_ = value;
}

float DetectionConfig::maxGapInterval() const
{
    return max_gap_interval_s_;
}

void DetectionConfig::maxGapInterval(float value)
{
    max_gap_interval_s_ = value;
}

bool DetectionConfig::useMissTolerance() const
{
    return use_miss_tolerance_;
}

void DetectionConfig::useMissTolerance(bool value)
{
    use_miss_tolerance_ = value;
}

float DetectionConfig::missTolerance() const
{
    return miss_tolerance_s_;
}

void DetectionConfig::missTolerance(float value)
{
    miss_tolerance_s_ = value;
}

}
