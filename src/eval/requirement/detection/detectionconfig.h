#ifndef EVALUATIONREQUIREMENTDETECTIONCONFIG_H
#define EVALUATIONREQUIREMENTDETECTIONCONFIG_H

#include "configurable.h"
#include "eval/requirement/config.h"
#include "eval/requirement/detection/detectionconfigwidget.h"
#include "eval/requirement/detection/detection.h"

#include <QObject>

#include <memory>


class Group;
class EvaluationStandard;

namespace EvaluationRequirement
{

class DetectionConfig : public Config
{
    Q_OBJECT

signals:

public slots:

public:
    DetectionConfig(const std::string& class_id, const std::string& instance_id,
                                         Group& group, EvaluationStandard& standard,
                                         EvaluationManager& eval_man);
    virtual ~DetectionConfig();

    virtual void addGUIElements(QFormLayout* layout) override;
    DetectionConfigWidget* widget() override;
    std::shared_ptr<Base> createRequirement() override;

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

    std::unique_ptr<DetectionConfigWidget> widget_;
};

}

#endif // EVALUATIONREQUIREMENTDETECTIONCONFIG_H
