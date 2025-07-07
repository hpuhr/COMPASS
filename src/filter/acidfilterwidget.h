
#pragma once

#include "dbfilterwidget.h"
#include "acidfilter.h"

class QLineEdit;

/**
 */
class ACIDFilterWidget : public DBFilterWidget
{
    Q_OBJECT

protected slots:
    void valueEditedSlot(const QString& value);

public:
    ACIDFilterWidget(ACIDFilter& filter);
    virtual ~ACIDFilterWidget();

    virtual void update();

protected:
    ACIDFilter& filter_;

    QLineEdit* value_edit_ {nullptr};
};
