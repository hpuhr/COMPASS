#include "evaluationrequirementdetectionconfigwidget.h"
#include "evaluationrequirementdetectionconfig.h"

#include <QFormLayout>

EvaluationRequirementDetectionConfigWidget::EvaluationRequirementDetectionConfigWidget(
        EvaluationRequirementDetectionConfig& config)
    : QWidget(), config_(config)
{
    form_layout_ = new QFormLayout();

    config_.addGUIElements(form_layout_);

    setLayout(form_layout_);
}
