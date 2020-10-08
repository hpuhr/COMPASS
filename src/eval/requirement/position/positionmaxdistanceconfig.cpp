#include "eval/requirement/position/positionmaxdistanceconfig.h"
#include "eval/requirement/position/positionmaxdistanceconfigwidget.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base.h"

using namespace std;


namespace EvaluationRequirement
{

    PositionMaxDistanceConfig::PositionMaxDistanceConfig(
            const std::string& class_id, const std::string& instance_id,
            Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
    : Config(class_id, instance_id, group, standard, eval_man)
    {
        registerParameter("max_distance", &max_distance_, 50.0);
        registerParameter("maximum_probability", &maximum_probability_, 0.10);
    }

    PositionMaxDistanceConfig::~PositionMaxDistanceConfig()
    {

    }

    void PositionMaxDistanceConfig::addGUIElements(QFormLayout* layout)
    {
        assert (layout);

        Config::addGUIElements(layout);
    }

    PositionMaxDistanceConfigWidget* PositionMaxDistanceConfig::widget()
    {
        if (!widget_)
            widget_.reset(new PositionMaxDistanceConfigWidget(*this));

        return widget_.get();
    }

    std::shared_ptr<Base> PositionMaxDistanceConfig::createRequirement()
    {
        shared_ptr<PositionMaxDistance> req = make_shared<PositionMaxDistance>(
                    name_, short_name_, group_.name(), eval_man_, max_distance_, maximum_probability_);

        return req;
    }

    float PositionMaxDistanceConfig::maxDistance() const
    {
        return max_distance_;
    }

    float PositionMaxDistanceConfig::maximumProbability() const
    {
        return maximum_probability_;
    }

    void PositionMaxDistanceConfig::maxDistance(float value)
    {
        max_distance_ = value;
    }

    void PositionMaxDistanceConfig::maximumProbability(float value)
    {
        maximum_probability_ = value;
    }
}
