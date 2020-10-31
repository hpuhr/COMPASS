#ifndef UTNFILTERWIDGET_H
#define UTNFILTERWIDGET_H

#include "dbfilterwidget.h"
#include "utnfilter.h"

class QLabel;
class QLineEdit;

class UTNFilterWidget  : public DBFilterWidget
{
    Q_OBJECT

  protected slots:
    void valueEditedSlot(const QString& value);

public:
    UTNFilterWidget(UTNFilter& filter, const std::string& class_id, const std::string& instance_id);
    virtual ~UTNFilterWidget();

    virtual void update();

protected:
    UTNFilter& filter_;

    QLabel* label_{nullptr};
    QLineEdit* value_edit_ {nullptr};
};

#endif // UTNFILTERWIDGET_H
