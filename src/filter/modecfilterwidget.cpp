#include "modecfilterwidget.h"
#include "stringconv.h"
#include "textfielddoublevalidator.h"
#include "logger.h"
#include "rangeedit.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>

using namespace std;
using namespace Utils;

ModeCFilterWidget::ModeCFilterWidget(ModeCFilter& filter)
    : DBFilterWidget(filter), filter_(filter)
{
    QFormLayout* layout = new QFormLayout();

    const float ModeCMin    = -10000.0f;
    const float ModeCMax    =  100000.0f;
    const int   SliderSteps = 1000;

    min_value_edit_ = new QLineEdit();
    min_value_edit_->setValidator(new TextFieldDoubleValidator(ModeCMin, ModeCMax, Precision));
    connect(min_value_edit_, &QLineEdit::textEdited, this, &ModeCFilterWidget::minValueEditedSlot);

    layout->addRow("Mode C Code >=", min_value_edit_);

    max_value_edit_ = new QLineEdit();
    max_value_edit_->setValidator(new TextFieldDoubleValidator(ModeCMin, ModeCMax, Precision));
    connect(max_value_edit_, &QLineEdit::textEdited, this, &ModeCFilterWidget::maxValueEditedSlot);

    layout->addRow("Mode C Code <=", max_value_edit_);

    const QString limit0 = QString::number(ModeCMin, 'f', Precision);
    const QString limit1 = QString::number(ModeCMax, 'f', Precision);

    range_edit_ = new RangeEditFloat(SliderSteps, Precision);
    range_edit_->setLimits(limit0, limit1);
    range_edit_->connectToFields(min_value_edit_, max_value_edit_);
    layout->addRow("", range_edit_);

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

    const QString r0 = QString::number(filter_.minValue(), 'f', Precision);
    const QString r1 = QString::number(filter_.maxValue(), 'f', Precision);

    min_value_edit_->setText(r0);
    max_value_edit_->setText(r1);

    null_check_->setChecked(filter_.nullWanted());
}

void ModeCFilterWidget::minValueEditedSlot(const QString& value)
{
    if (!value.size())
    {
        loginf << "ModeCFilterWidget: minValueEditedSlot: skipping empty string";
        return;
    }

    bool ok;

    float value_float = value.toFloat(&ok);
    assert (ok);

    loginf << "ModeCFilterWidget: minValueEditedSlot: '" << value_float << "'";

    filter_.minValue(value_float);
}

void ModeCFilterWidget::maxValueEditedSlot(const QString& value)
{
    if (!value.size())
    {
        loginf << "ModeCFilterWidget: maxValueEditedSlot: skipping empty string";
        return;
    }

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
