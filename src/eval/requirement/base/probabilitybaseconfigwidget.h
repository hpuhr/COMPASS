#ifndef EVALUATIONREQUIREMENT_PROBABILITYBASECONFIGWIDGET_H
#define EVALUATIONREQUIREMENT_PROBABILITYBASECONFIGWIDGET_H

#include "eval/requirement/base/baseconfigwidget.h"

namespace EvaluationRequirement {

class ProbabilityBaseConfig;

class ProbabilityBaseConfigWidget : public BaseConfigWidget
{
    Q_OBJECT

signals:

public slots:
    void changedProbabilitySlot(const QString& value);
    void changedTypeSlot();

public:
    ProbabilityBaseConfigWidget(ProbabilityBaseConfig& cfg);

    virtual ~ProbabilityBaseConfigWidget() {}

protected:
    QLineEdit* prob_edit_ {nullptr};
    ComparisonTypeComboBox* check_type_box_ {nullptr};

    ProbabilityBaseConfig& config();
};

} // namespace EvaluationRequirement

#endif // EVALUATIONREQUIREMENT_PROBABILITYBASECONFIGWIDGET_H
