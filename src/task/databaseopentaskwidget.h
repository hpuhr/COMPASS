#ifndef DATABASEOPENTASKWIDGET_H
#define DATABASEOPENTASKWIDGET_H

#include <QWidget>

class DatabaseOpenTask;

class DatabaseOpenTaskWidget : public QWidget
{
public:
    DatabaseOpenTaskWidget(DatabaseOpenTask& task, QWidget *parent=nullptr);

protected:
    DatabaseOpenTask& task_;

    QWidget* dbinterface_widget_ {nullptr};
};

#endif // DATABASEOPENTASKWIDGET_H
