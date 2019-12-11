#ifndef MANAGESCHEMATASKWIDGET_H
#define MANAGESCHEMATASKWIDGET_H

#include <QWidget>

class ManageSchemaTask;

class ManageSchemaTaskWidget : public QWidget
{
public:
    ManageSchemaTaskWidget(ManageSchemaTask& task, QWidget *parent=nullptr);

protected:
    ManageSchemaTask& task_;

    QWidget* dbschema_manager_widget_ {nullptr};
};

#endif // MANAGESCHEMATASKWIDGET_H
