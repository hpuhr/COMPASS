#include "mlatrufilterwidget.h"
#include "mlatrufilter.h"
//#include "stringconv.h"
#include "logger.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

using namespace std;
//using namespace Utils;

MLATRUFilterWidget::MLATRUFilterWidget(MLATRUFilter& filter)
    : DBFilterWidget(filter), filter_(filter)
{
    QHBoxLayout* layout = new QHBoxLayout();

    label_ = new QLabel("MLAT RUs IN");
    layout->addWidget(label_);

    value_edit_ = new QLineEdit();
    //value_edit_->setValidator(new TextFieldDoubleValidator(0, 100000, 0));
    connect(value_edit_, &QLineEdit::textEdited, this, &MLATRUFilterWidget::valueEditedSlot);
    layout->addWidget(value_edit_);

    child_layout_->addLayout(layout);

    update();
}

MLATRUFilterWidget::~MLATRUFilterWidget()
{

}

void MLATRUFilterWidget::update()

{
    DBFilterWidget::update();

    assert (value_edit_);

    value_edit_->setText(filter_.rus().c_str());
}

void MLATRUFilterWidget::valueEditedSlot(const QString& value)
{
    string value_str = value.toStdString();

    loginf << "MLATRUFilterWidget: valueEditedSlot: '" << value_str << "'";

    filter_.rus(value_str);
}
