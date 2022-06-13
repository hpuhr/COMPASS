#include "mode3afilterwidget.h"
#include "stringconv.h"
#include "logger.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

using namespace std;
using namespace Utils;

Mode3AFilterWidget::Mode3AFilterWidget(Mode3AFilter& filter)
    : DBFilterWidget(filter), filter_(filter)
{
    QHBoxLayout* layout = new QHBoxLayout();

    label_ = new QLabel("Mode 3/A Codes IN");
    layout->addWidget(label_);

    value_edit_ = new QLineEdit();
    connect(value_edit_, &QLineEdit::textEdited, this, &Mode3AFilterWidget::valueEditedSlot);
    layout->addWidget(value_edit_);

    child_layout_->addLayout(layout);

    update();
}

Mode3AFilterWidget::~Mode3AFilterWidget()
{

}

void Mode3AFilterWidget::update()

{
    DBFilterWidget::update();

    assert (value_edit_);

    value_edit_->setText(filter_.valuesString().c_str());
}

void Mode3AFilterWidget::valueEditedSlot(const QString& value)
{
    string value_str = value.toStdString();

    loginf << "Mode3AFilterWidget: valueEditedSlot: '" << value_str << "'";

    filter_.valuesString(value_str);
}

