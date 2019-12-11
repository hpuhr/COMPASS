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
};

#endif // MANAGESCHEMATASKWIDGET_H
