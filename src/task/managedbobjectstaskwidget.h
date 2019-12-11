#ifndef MANAGEDBOBJECTSTASKWIDGET_H
#define MANAGEDBOBJECTSTASKWIDGET_H

#include <QWidget>

class ManageDBObjectsTask;
class DBObjectManagerWidget;

class ManageDBObjectsTaskWidget : public QWidget
{
public:
    ManageDBObjectsTaskWidget(ManageDBObjectsTask& task, QWidget *parent=nullptr);

protected:
    ManageDBObjectsTask& task_;

    DBObjectManagerWidget* object_manager_widget_ {nullptr};
};

#endif // MANAGEDBOBJECTSTASKWIDGET_H
