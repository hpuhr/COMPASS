#include "utnfilterwidget.h"
#include "utnfilter.h"
#include "stringconv.h"
//#include "textfielddoublevalidator.h"
#include "logger.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

using namespace std;
using namespace Utils;

UTNFilterWidget::UTNFilterWidget(UTNFilter& filter, const std::string& class_id, const std::string& instance_id)
    : DBFilterWidget(class_id, instance_id, filter), filter_(filter)
{
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    label_ = new QLabel("UTNs IN");
    layout->addWidget(label_);

    value_edit_ = new QLineEdit();
    //value_edit_->setValidator(new TextFieldDoubleValidator(0, 100000, 0));
    connect(value_edit_, &QLineEdit::textEdited, this, &UTNFilterWidget::valueEditedSlot);
    layout->addWidget(value_edit_);

    child_layout_->addLayout(layout);

    update();
}

UTNFilterWidget::~UTNFilterWidget()
{

}

void UTNFilterWidget::update()

{
    DBFilterWidget::update();

    assert (value_edit_);

    value_edit_->setText(filter_.utns().c_str());
}

void UTNFilterWidget::valueEditedSlot(const QString& value)
{
    string value_str = value.toStdString();

    loginf << "UTNFilterWidget: valueEditedSlot: '" << value_str << "'";

    filter_.utns(value_str);
}
