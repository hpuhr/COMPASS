#include "probabilitybaseconfigwidget.h"
#include "probabilitybaseconfig.h"
#include "eval/requirement/base/comparisontypecombobox.h"
#include "logger.h"

#include <QLabel>
#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>

namespace EvaluationRequirement {

ProbabilityBaseConfigWidget::ProbabilityBaseConfigWidget(ProbabilityBaseConfig& cfg)
    : BaseConfigWidget(cfg)
{
    assert (form_layout_);

    // prob
    prob_edit_ = new QLineEdit(QString::number(config().prob()));
    prob_edit_->setValidator(new QDoubleValidator(0.0000001, 1.0, 8, this));
    connect(prob_edit_, &QLineEdit::textEdited,
            this, &ProbabilityBaseConfigWidget::changedProbabilitySlot);

    form_layout_->addRow("Probability [1]", prob_edit_);

    // prob check type
    check_type_box_ = new ComparisonTypeComboBox();
    check_type_box_->setType(config().probCheckType());
    connect(check_type_box_, &ComparisonTypeComboBox::changedTypeSignal,
            this, &ProbabilityBaseConfigWidget::changedTypeSlot);
    form_layout_->addRow("Probability Check Type", check_type_box_);
}

void ProbabilityBaseConfigWidget::changedProbabilitySlot(const QString& value)
{
    loginf << "BaseConfigWidget: changedProbabilitySlot: value " << value.toStdString();

    bool ok;
    float val = value.toFloat(&ok);

    if (ok)
        config().prob(val);
    else
        loginf << "BaseConfigWidget: changedProbabilitySlot: invalid value";
}

void ProbabilityBaseConfigWidget::changedTypeSlot()
{
    assert (check_type_box_);
    logdbg << "BaseConfigWidget: changedProbabilitySlot: value " << check_type_box_->getType();
    config().probCheckType(check_type_box_->getType());
}

ProbabilityBaseConfig& ProbabilityBaseConfigWidget::config()
{
    ProbabilityBaseConfig* config = dynamic_cast<ProbabilityBaseConfig*>(&config_);
    assert (config);

    return *config;
}


} // namespace EvaluationRequirement
