#include "reftrajaccuracyfilterwidget.h"
//#include "stringconv.h"
#include "textfielddoublevalidator.h"
#include "logger.h"
//#include "rangeedit.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>

using namespace std;
//using namespace Utils;

RefTrajAccuracyFilterWidget::RefTrajAccuracyFilterWidget(RefTrajAccuracyFilter& filter)
    : DBFilterWidget(filter), filter_(filter)
{
    min_value_edit_ = new QLineEdit();
    min_value_edit_->setValidator(new TextFieldDoubleValidator(0, 10e6, Precision));
    connect(min_value_edit_, &QLineEdit::textEdited, this, &RefTrajAccuracyFilterWidget::minValueEditedSlot);

    addNameValuePair("Accuracy <=", min_value_edit_);

    update();
}

RefTrajAccuracyFilterWidget::~RefTrajAccuracyFilterWidget()
{
}

void RefTrajAccuracyFilterWidget::update()
{
    DBFilterWidget::update();

    assert (min_value_edit_);

    const QString r0 = QString::number(filter_.minValue(), 'f', Precision);

    min_value_edit_->setText(r0);
}

void RefTrajAccuracyFilterWidget::minValueEditedSlot(const QString& value)
{
    if (!value.size())
    {
        loginf << "RefTrajAccuracyFilterWidget: minValueEditedSlot: skipping empty string";
        return;
    }

    bool ok;

    float value_float = value.toFloat(&ok);
    assert (ok);

    loginf << "RefTrajAccuracyFilterWidget: minValueEditedSlot: '" << value_float << "'";

    filter_.minValue(value_float);
}

