#ifndef EVALUATIONREQUIREMENTPOSITIONMAXDISTANCECONFIGWIDGET_H
#define EVALUATIONREQUIREMENTPOSITIONMAXDISTANCECONFIGWIDGET_H


#include <QWidget>

class QLineEdit;
class QCheckBox;

class QFormLayout;

namespace EvaluationRequirement
{
    class PositionMaxDistanceConfig;

    class PositionMaxDistanceConfigWidget : public QWidget
    {
        Q_OBJECT

    public slots:
        void maxRefTimeDiffEditSlot(QString value);
        void maxDistanceEditSlot(QString value);
        void maximumProbEditSlot(QString value);

    public:
        PositionMaxDistanceConfigWidget(PositionMaxDistanceConfig& config);

    protected:
        PositionMaxDistanceConfig& config_;

        QLineEdit* max_ref_time_diff_edit_{nullptr};
        QLineEdit* max_distance_edit_{nullptr};
        QLineEdit* maximum_prob_edit_{nullptr};

        QFormLayout* form_layout_ {nullptr};
    };

}

#endif // EVALUATIONREQUIREMENTPOSITIONMAXDISTANCECONFIGWIDGET_H
