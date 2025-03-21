
#pragma once

#include "dbfilterwidget.h"
#include "mlatrufilter.h"

class QLineEdit;

/**
 */
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

    QLineEdit* value_edit_ {nullptr};
};
