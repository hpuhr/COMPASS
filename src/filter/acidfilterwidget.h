#ifndef ACIDFILTERWIDGET_H
#define ACIDFILTERWIDGET_H

#include "dbfilterwidget.h"
#include "acidfilter.h"

class QLabel;
class QLineEdit;

class ACIDFilterWidget : public DBFilterWidget
{

    Q_OBJECT

protected slots:
    void valueEditedSlot(const QString& value);

public:
    ACIDFilterWidget(ACIDFilter& filter, const std::string& class_id, const std::string& instance_id);
    virtual ~ACIDFilterWidget();

    virtual void update();

protected:
    ACIDFilter& filter_;

    QLabel* label_{nullptr};
    QLineEdit* value_edit_ {nullptr};
};


#endif // ACIDFILTERWIDGET_H
