#ifndef EVALUATIONREQUIREMENTDETECTIONCONFIG_H
#define EVALUATIONREQUIREMENTDETECTIONCONFIG_H

#include "configurable.h"
#include "evaluationrequirementconfig.h"
#include "evaluationrequirementdetectionconfigwidget.h"
#include "evaluationrequirementdetection.h"

#include <QObject>

#include <memory>

class EvaluationRequirementGroup;
class EvaluationStandard;

class EvaluationRequirementDetectionConfig : public EvaluationRequirementConfig
{
    Q_OBJECT

signals:

public slots:

public:
    EvaluationRequirementDetectionConfig(const std::string& class_id, const std::string& instance_id,
                                         EvaluationRequirementGroup& group, EvaluationStandard& standard,
                                         EvaluationManager& eval_man);
    virtual ~EvaluationRequirementDetectionConfig();

    virtual void addGUIElements(QFormLayout* layout) override;
    EvaluationRequirementDetectionConfigWidget* widget() override;
    std::shared_ptr<EvaluationRequirement> createRequirement() override;

    float updateInterval() const;
    void updateInterval(float value);

    float minimumProbability() const;
    void minimumProbability(float value);

    bool useMaxGapInterval() const;
    void useMaxGapInterval(bool value);

    float maxGapInterval() const;
    void maxGapInterval(float value);

    bool useMissTolerance() const;
    void useMissTolerance(bool value);

    float missTolerance() const;
    void missTolerance(float value);

protected:
    float update_interval_s_{0};

    float minimum_probability_{0};

    bool use_max_gap_interval_{true};
    float max_gap_interval_s_{0};

    bool use_miss_tolerance_{false};
    float miss_tolerance_s_{0};

    std::unique_ptr<EvaluationRequirementDetectionConfigWidget> widget_;
};

#endif // EVALUATIONREQUIREMENTDETECTIONCONFIG_H
