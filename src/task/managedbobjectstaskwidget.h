#ifndef MANAGEDBOBJECTSTASKWIDGET_H
#define MANAGEDBOBJECTSTASKWIDGET_H

#include <taskwidget.h>

class ManageDBObjectsTask;
class DBObjectManagerWidget;

class ManageDBObjectsTaskWidget : public TaskWidget
{
    Q_OBJECT

public slots:
    void expertModeChangedSlot ();

public:
    ManageDBObjectsTaskWidget(ManageDBObjectsTask& task, QWidget *parent=nullptr);

protected:
    ManageDBObjectsTask& task_;

    DBObjectManagerWidget* object_manager_widget_ {nullptr};
};

#endif // MANAGEDBOBJECTSTASKWIDGET_H
