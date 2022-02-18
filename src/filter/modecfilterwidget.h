#ifndef MODECFILTERWIDGET_H
#define MODECFILTERWIDGET_H

#include "dbfilterwidget.h"
#include "modecfilter.h"

class QCheckBox;
class QLineEdit;

class ModeCFilterWidget : public DBFilterWidget
{

    Q_OBJECT

protected slots:
    void minValueEditedSlot(const QString& value);
    void maxValueEditedSlot(const QString& value);
    void nullWantedChangedSlot();

public:
    ModeCFilterWidget(ModeCFilter& filter, const std::string& class_id, const std::string& instance_id);
    virtual ~ModeCFilterWidget();

    virtual void update();

protected:
    ModeCFilter& filter_;

    QLineEdit* min_value_edit_ {nullptr};
    QLineEdit* max_value_edit_ {nullptr};
    QCheckBox* null_check_ {nullptr};
};

#endif // MODECFILTERWIDGET_H
