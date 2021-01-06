#include "eval/requirement/base/baseconfigwidget.h"
#include "eval/requirement/base/baseconfig.h"
#include "eval/requirement/base/comparisontypecombobox.h"
#include "logger.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>

using namespace std;

namespace EvaluationRequirement
{

BaseConfigWidget::BaseConfigWidget(BaseConfig& cfg)
    : QWidget(), config_(cfg)
{
    form_layout_ = new QFormLayout();

    QLineEdit* name_edit = new QLineEdit (config_.name().c_str());
    connect(name_edit, &QLineEdit::textEdited, this, &BaseConfigWidget::changedNameSlot);

    form_layout_->addRow("Name", name_edit);

    QLineEdit* short_name_edit = new QLineEdit (config_.shortName().c_str());
    connect(short_name_edit, &QLineEdit::textEdited, this, &BaseConfigWidget::changedShortNameSlot);

    form_layout_->addRow("Short Name", short_name_edit);

    // prob

    QLineEdit* minimum_prob_edit_ = new QLineEdit(QString::number(config_.prob()));
    minimum_prob_edit_->setValidator(new QDoubleValidator(0.0001, 1.0, 4, this));
    connect(minimum_prob_edit_, &QLineEdit::textEdited,
            this, &BaseConfigWidget::changedProbabilitySlot);

    form_layout_->addRow("Probability [1]", minimum_prob_edit_);

    // prob check type
    check_type_box_ = new ComparisonTypeComboBox();
    check_type_box_->setType(config_.probCheckType());
    connect(check_type_box_, &ComparisonTypeComboBox::changedTypeSignal, this, &BaseConfigWidget::changedTypeSlot);
    form_layout_->addRow("Probability Check Type", check_type_box_);

    setLayout(form_layout_);
}

BaseConfigWidget::~BaseConfigWidget()
{

}

void BaseConfigWidget::changedNameSlot(const QString& value)
{
    string value_str = value.toStdString();

    loginf << "BaseConfigWidget: changedNameSlot: name '" << value_str << "'";


}

void BaseConfigWidget::changedShortNameSlot(const QString& value)
{
    string value_str = value.toStdString();

    loginf << "BaseConfigWidget: changedShortNameSlot: name '" << value_str << "'";

    config_.shortName(value_str);
}

void BaseConfigWidget::changedProbabilitySlot(const QString& value)
{
    loginf << "BaseConfigWidget: changedProbabilitySlot: value " << value.toStdString();

    bool ok;
    float val = value.toFloat(&ok);

    if (ok)
        config_.prob(val);
    else
        loginf << "BaseConfigWidget: changedProbabilitySlot: invalid value";
}

void BaseConfigWidget::changedTypeSlot()
{
    assert (check_type_box_);
    loginf << "BaseConfigWidget: changedProbabilitySlot: value " << check_type_box_->getType();
    config_.probCheckType(check_type_box_->getType());
}


}
