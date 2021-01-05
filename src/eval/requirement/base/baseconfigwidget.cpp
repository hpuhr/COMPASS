#include "eval/requirement/base/baseconfigwidget.h"
#include "eval/requirement/base/baseconfig.h"
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


}
