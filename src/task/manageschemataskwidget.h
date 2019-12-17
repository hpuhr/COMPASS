#ifndef MANAGESCHEMATASKWIDGET_H
#define MANAGESCHEMATASKWIDGET_H

#include <QWidget>

class ManageSchemaTask;

class ManageSchemaTaskWidget : public QWidget
{
    Q_OBJECT

public slots:
    void schemaLockedSlot ();

public:
    ManageSchemaTaskWidget(ManageSchemaTask& task, QWidget *parent=nullptr);

protected:
    ManageSchemaTask& task_;

    //QWidget* dbschema_manager_widget_ {nullptr};
};

#endif // MANAGESCHEMATASKWIDGET_H
