#ifndef REFTRAJACCURCYFILTERWIDGET_H
#define REFTRAJACCURCYFILTERWIDGET_H

#include "dbfilterwidget.h"
#include "reftrajaccuracyfilter.h"

class QCheckBox;
class QLineEdit;

class RangeEditFloat;

class RefTrajAccuracyFilterWidget : public DBFilterWidget
{

    Q_OBJECT

protected slots:
    void minValueEditedSlot(const QString& value);

public:
    RefTrajAccuracyFilterWidget(RefTrajAccuracyFilter& filter);
    virtual ~RefTrajAccuracyFilterWidget();

    virtual void update();

protected:
    static const int Precision = 2;

    RefTrajAccuracyFilter& filter_;

    QLineEdit* min_value_edit_ {nullptr};
};

#endif // REFTRAJACCURCYFILTERWIDGET_H
