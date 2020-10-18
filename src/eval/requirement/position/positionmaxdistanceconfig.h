#ifndef EVALUATIONREQUIREMENTPOSITIONMAXDISTANCECONFIG_H
#define EVALUATIONREQUIREMENTPOSITIONMAXDISTANCECONFIG_H

#include "configurable.h"
#include "eval/requirement/config.h"
#include "eval/requirement/position/positionmaxdistanceconfigwidget.h"
#include "eval/requirement/position/positionmaxdistance.h"

class Group;
class EvaluationStandard;

namespace EvaluationRequirement
{

class PositionMaxDistanceConfig : public Config
{
public:
    PositionMaxDistanceConfig(const std::string& class_id, const std::string& instance_id,
                              Group& group, EvaluationStandard& standard,
                              EvaluationManager& eval_ma);
    virtual ~PositionMaxDistanceConfig();

    virtual void addGUIElements(QFormLayout* layout) override;
    PositionMaxDistanceConfigWidget* widget() override;
    std::shared_ptr<Base> createRequirement() override;

    float maxDistance() const;
    void maxDistance(float value);

    float maximumProbability() const;
    void maximumProbability(float value);

    float maxRefTimeDiff() const;
    void maxRefTimeDiff(float value);

protected:
    float max_ref_time_diff_ {0};
    float max_distance_ {0};
    float maximum_probability_{0};

    std::unique_ptr<PositionMaxDistanceConfigWidget> widget_;
};

}
#endif // EVALUATIONREQUIREMENTPOSITIONMAXDISTANCECONFIG_H
