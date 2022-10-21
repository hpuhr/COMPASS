#ifndef MODECFILTERWIDGET_H
#define MODECFILTERWIDGET_H

#include "dbfilterwidget.h"
#include "modecfilter.h"

class QCheckBox;
class QLineEdit;

class RangeEditFloat;

class ModeCFilterWidget : public DBFilterWidget
{

    Q_OBJECT

protected slots:
    void minValueEditedSlot(const QString& value);
    void maxValueEditedSlot(const QString& value);
    void nullWantedChangedSlot();

public:
    ModeCFilterWidget(ModeCFilter& filter);
    virtual ~ModeCFilterWidget();

    virtual void update();

protected:
    static const int Precision = 2;

    ModeCFilter& filter_;

    QLineEdit* min_value_edit_ {nullptr};
    QLineEdit* max_value_edit_ {nullptr};
    QCheckBox* null_check_ {nullptr};
    RangeEditFloat* range_edit_ {nullptr};
};

#endif // MODECFILTERWIDGET_H
