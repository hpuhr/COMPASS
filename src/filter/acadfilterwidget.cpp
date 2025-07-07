#include "acadfilterwidget.h"
//#include "stringconv.h"
#include "logger.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

using namespace std;
//using namespace Utils;

ACADFilterWidget::ACADFilterWidget(ACADFilter& filter)
    : DBFilterWidget(filter), filter_(filter)
{
    value_edit_ = new QLineEdit();

    addNameValuePair("ACADs IN", value_edit_);

    update();

    connect(value_edit_, &QLineEdit::textEdited, this, &ACADFilterWidget::valueEditedSlot);
}

ACADFilterWidget::~ACADFilterWidget() = default;

void ACADFilterWidget::update()

{
    DBFilterWidget::update();

    assert (value_edit_);

    value_edit_->setText(filter_.valuesString().c_str());
}

void ACADFilterWidget::valueEditedSlot(const QString& value)
{
    string value_str = value.toStdString();

    loginf << "ACADFilterWidget: valueEditedSlot: '" << value_str << "'";

    filter_.valuesString(value_str);
}


