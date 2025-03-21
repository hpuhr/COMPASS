#include "mode3afilterwidget.h"
//#include "stringconv.h"
#include "logger.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

using namespace std;
//using namespace Utils;

Mode3AFilterWidget::Mode3AFilterWidget(Mode3AFilter& filter)
    : DBFilterWidget(filter), filter_(filter)
{
    value_edit_ = new QLineEdit();
    connect(value_edit_, &QLineEdit::textEdited, this, &Mode3AFilterWidget::valueEditedSlot);

    addNameValuePair("Mode 3/A Codes IN", value_edit_);

    update();
}

Mode3AFilterWidget::~Mode3AFilterWidget() = default;

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

