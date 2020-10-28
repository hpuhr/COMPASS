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
        registerParameter("max_ref_time_diff", &max_ref_time_diff_, 4.0);
        registerParameter("max_distance", &max_distance_, 50.0);
        registerParameter("minimum_probability", &minimum_probability_, 0.9);
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
                    name_, short_name_, group_.name(), eval_man_,
                    max_ref_time_diff_, max_distance_, minimum_probability_);

        return req;
    }

    float PositionMaxDistanceConfig::maxDistance() const
    {
        return max_distance_;
    }

    float PositionMaxDistanceConfig::minimumProbability() const
    {
        return minimum_probability_;
    }

    void PositionMaxDistanceConfig::maxDistance(float value)
    {
        max_distance_ = value;
    }

    void PositionMaxDistanceConfig::minimumProbability(float value)
    {
        minimum_probability_ = value;
    }
    
    float PositionMaxDistanceConfig::maxRefTimeDiff() const
    {
        return max_ref_time_diff_;
    }
    
    void PositionMaxDistanceConfig::maxRefTimeDiff(float value)
    {
        max_ref_time_diff_ = value;
    }
}
