#ifndef  EVALUATIONREQUIREMENTBASECONFIGWIDGET_H
#define  EVALUATIONREQUIREMENTBASECONFIGWIDGET_H

#include <QWidget>

class QLineEdit;
class QCheckBox;

class QFormLayout;

namespace EvaluationRequirement
{

class BaseConfig;

class BaseConfigWidget : public QWidget
{
    Q_OBJECT

signals:

public slots:
    void changedNameSlot(const QString& value);
    void changedShortNameSlot(const QString& value);

public:
    BaseConfigWidget(BaseConfig& cfg);

protected:
    BaseConfig& config_;

    QFormLayout* form_layout_ {nullptr};
};

}

#endif //  EVALUATIONREQUIREMENTBASECONFIGWIDGET_H
