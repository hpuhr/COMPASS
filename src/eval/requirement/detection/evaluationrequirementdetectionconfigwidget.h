#ifndef EVALUATIONREQUIREMENTDETECTIONCONFIGWIDGET_H
#define EVALUATIONREQUIREMENTDETECTIONCONFIGWIDGET_H

#include <QWidget>

class QLineEdit;
class QCheckBox;

class QFormLayout;

namespace EvaluationRequirement
{
class DetectionConfig;

class DetectionConfigWidget : public QWidget
{
    Q_OBJECT

public slots:
    void updateIntervalEditSlot(QString value);
    void minimumProbEditSlot(QString value);

    void toggleUseMaxGapSlot();
    void maxGapEditSlot(QString value);

    void toggleUseMissToleranceSlot();
    void missToleranceEditSlot(QString value);

public:
    DetectionConfigWidget(DetectionConfig& config);

protected:
    DetectionConfig& config_;

    QFormLayout* form_layout_ {nullptr};

    QLineEdit* update_interval_edit_{nullptr};
    QLineEdit* minimum_prob_edit_{nullptr};

    QCheckBox* use_max_gap_check_{nullptr};
    QLineEdit* max_gap_interval_edit_{nullptr};

    QCheckBox* use_miss_tolerance_check_{nullptr};
    QLineEdit* miss_tolerance_edit_{nullptr};

};

}

#endif // EVALUATIONREQUIREMENTDETECTIONCONFIGWIDGET_H
