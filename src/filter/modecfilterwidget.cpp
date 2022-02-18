#include "modecfilterwidget.h"
#include "stringconv.h"
#include "textfielddoublevalidator.h"
#include "logger.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>

using namespace std;
using namespace Utils;

ModeCFilterWidget::ModeCFilterWidget(ModeCFilter& filter, const std::string& class_id, const std::string& instance_id)
    : DBFilterWidget(class_id, instance_id, filter), filter_(filter)
{
    QFormLayout* layout = new QFormLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    //layout->setSpacing(0);

    min_value_edit_ = new QLineEdit();
    min_value_edit_->setValidator(new TextFieldDoubleValidator(-10000, 100000, 2));
    connect(min_value_edit_, &QLineEdit::textEdited, this, &ModeCFilterWidget::minValueEditedSlot);

    layout->addRow("Mode C Code >=", min_value_edit_);

    max_value_edit_ = new QLineEdit();
    max_value_edit_->setValidator(new TextFieldDoubleValidator(-10000, 100000, 2));
    connect(max_value_edit_, &QLineEdit::textEdited, this, &ModeCFilterWidget::maxValueEditedSlot);

    layout->addRow("Mode C Code <=", max_value_edit_);

    null_check_ = new QCheckBox();
    connect(null_check_, &QCheckBox::clicked, this, &ModeCFilterWidget::nullWantedChangedSlot);
    layout->addRow("NULL Values", null_check_);

    child_layout_->addLayout(layout);

    update();
}

ModeCFilterWidget::~ModeCFilterWidget()
{

}

void ModeCFilterWidget::update()

{
    DBFilterWidget::update();

    assert (min_value_edit_);
    assert (max_value_edit_);
    assert(null_check_);

    min_value_edit_->setText(QString::number(filter_.minValue()));
    max_value_edit_->setText(QString::number(filter_.maxValue()));
    null_check_->setChecked(filter_.nullWanted());
}

void ModeCFilterWidget::minValueEditedSlot(const QString& value)
{
    bool ok;

    float value_float = value.toFloat(&ok);
    assert (ok);

    loginf << "ModeCFilterWidget: minValueEditedSlot: '" << value_float << "'";

    filter_.minValue(value_float);
}

void ModeCFilterWidget::maxValueEditedSlot(const QString& value)
{
    bool ok;

    float value_float = value.toFloat(&ok);
    assert (ok);

    loginf << "ModeCFilterWidget: maxValueEditedSlot: '" << value_float << "'";

    filter_.maxValue(value_float);
}

void ModeCFilterWidget::nullWantedChangedSlot()
{
     assert(null_check_);
    bool wanted = null_check_->checkState() == Qt::Checked;

    loginf << "ModeCFilterWidget: nullWantedChangedSlot: " << wanted;

    filter_.nullWanted(wanted);
}
