#ifndef TASKMANAGERWIDGET_H
#define TASKMANAGERWIDGET_H

#include <QWidget>

class QListWidget;
class QStackedWidget;
class TaskManagerLogWidget;
class TaskManager;

class TaskManagerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TaskManagerWidget(TaskManager& task_manager, QWidget *parent=nullptr);

signals:

public slots:

protected:
    TaskManager& task_manager_;

    QListWidget* task_list_ {nullptr};
    QStackedWidget* tasks_widget_ {nullptr};
    TaskManagerLogWidget* log_widget_ {nullptr};
};

#endif // TASKMANAGERWIDGET_H
