#ifndef  EVALUATIONREQUIREMENTBASECONFIGWIDGET_H
#define  EVALUATIONREQUIREMENTBASECONFIGWIDGET_H

#include <QWidget>

class QLineEdit;
class QCheckBox;

class QFormLayout;

namespace EvaluationRequirement
{

class BaseConfig;
class CheckTypeComboBox;

class BaseConfigWidget : public QWidget
{
    Q_OBJECT

signals:

public slots:
    void changedNameSlot(const QString& value);
    void changedShortNameSlot(const QString& value);
    void changedProbabilitySlot(const QString& value);
    void changedTypeSlot();

public:
    BaseConfigWidget(BaseConfig& cfg);
    virtual ~BaseConfigWidget();

protected:
    BaseConfig& config_;

    QFormLayout* form_layout_ {nullptr};

    CheckTypeComboBox* check_type_box_ {nullptr};
};

}

#endif //  EVALUATIONREQUIREMENTBASECONFIGWIDGET_H
