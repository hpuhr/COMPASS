#ifndef ACADFILTERWIDGET_H
#define ACADFILTERWIDGET_H

#include "dbfilterwidget.h"
#include "acadfilter.h"

class QLabel;
class QLineEdit;

class ACADFilterWidget : public DBFilterWidget
{

    Q_OBJECT

protected slots:
    void valueEditedSlot(const QString& value);

public:
    ACADFilterWidget(ACADFilter& filter);
    virtual ~ACADFilterWidget();

    virtual void update();

protected:
    ACADFilter& filter_;

    QLabel* label_{nullptr};
    QLineEdit* value_edit_ {nullptr};
};

#endif // ACADFILTERWIDGET_H
