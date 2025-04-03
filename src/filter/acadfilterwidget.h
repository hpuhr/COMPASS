
#pragma once

#include "dbfilterwidget.h"
#include "acadfilter.h"

class QLineEdit;

/**
 */
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

    QLineEdit* value_edit_ {nullptr};
};
