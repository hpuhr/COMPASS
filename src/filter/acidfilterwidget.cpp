#include "acidfilterwidget.h"
//#include "stringconv.h"
#include "logger.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

using namespace std;
//using namespace Utils;

ACIDFilterWidget::ACIDFilterWidget(ACIDFilter& filter)
    : DBFilterWidget(filter), filter_(filter)
{
    QHBoxLayout* layout = new QHBoxLayout();

    value_edit_ = new QLineEdit();
    connect(value_edit_, &QLineEdit::textEdited, this, &ACIDFilterWidget::valueEditedSlot);

    addNameValuePair("ACIDs IN", value_edit_);

    update();
}

ACIDFilterWidget::~ACIDFilterWidget() = default;

void ACIDFilterWidget::update()
{
    DBFilterWidget::update();

    assert (value_edit_);

    value_edit_->setText(filter_.valuesString().c_str());
}

void ACIDFilterWidget::valueEditedSlot(const QString& value)
{
    string value_str = value.toStdString();

    loginf << "ACIDFilterWidget: valueEditedSlot: '" << value_str << "'";

    filter_.valuesString(value_str);
}
