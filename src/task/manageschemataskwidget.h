#ifndef MANAGESCHEMATASKWIDGET_H
#define MANAGESCHEMATASKWIDGET_H

#include <taskwidget.h>

class ManageSchemaTask;

class ManageSchemaTaskWidget : public TaskWidget
{
    Q_OBJECT

public slots:
    void schemaLockedSlot ();
    void expertModeChangedSlot ();

public:
    ManageSchemaTaskWidget(ManageSchemaTask& task, QWidget *parent=nullptr);

protected:
    ManageSchemaTask& task_;

    //QWidget* dbschema_manager_widget_ {nullptr};
};

#endif // MANAGESCHEMATASKWIDGET_H
