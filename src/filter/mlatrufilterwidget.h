#ifndef MLATRUFILTERWIDGET_H
#define MLATRUFILTERWIDGET_H

#include "dbfilterwidget.h"
#include "mlatrufilter.h"

class QLabel;
class QLineEdit;

class MLATRUFilterWidget : public DBFilterWidget
{
    Q_OBJECT

  protected slots:
    void valueEditedSlot(const QString& value);

public:
    MLATRUFilterWidget(MLATRUFilter& filter);
    virtual ~MLATRUFilterWidget();

    virtual void update();

protected:
    MLATRUFilter& filter_;

    QLabel* label_{nullptr};
    QLineEdit* value_edit_ {nullptr};
};


#endif // MLATRUFILTERWIDGET_H
