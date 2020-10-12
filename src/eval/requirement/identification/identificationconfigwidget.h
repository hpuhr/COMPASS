#ifndef EVALUATIONREQUIREMENTIDENTIFICATIONCONFIGWIDGET_H
#define EVALUATIONREQUIREMENTIDENTIFICATIONCONFIGWIDGET_H

#include <QWidget>

class QLineEdit;
class QCheckBox;

class QFormLayout;

namespace EvaluationRequirement
{
    class IdentificationConfig;

    class IdentificationConfigWidget : public QWidget
    {
        Q_OBJECT

    public slots:
        void minimumProbEditSlot(QString value);

    public:
        IdentificationConfigWidget(IdentificationConfig& config);

    protected:
        IdentificationConfig& config_;

        QFormLayout* form_layout_ {nullptr};

        QLineEdit* minimum_prob_edit_{nullptr};
    };

}

#endif // EVALUATIONREQUIREMENTIDENTIFICATIONCONFIGWIDGET_H
