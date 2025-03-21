
#pragma once

#include "dbfilterwidget.h"
#include "mode3afilter.h"

class QLineEdit;

/**
 */
class Mode3AFilterWidget : public DBFilterWidget
{

    Q_OBJECT

protected slots:
    void valueEditedSlot(const QString& value);

public:
    Mode3AFilterWidget(Mode3AFilter& filter);
    virtual ~Mode3AFilterWidget();

    virtual void update();

protected:
    Mode3AFilter& filter_;

    QLineEdit* value_edit_ {nullptr};
};
