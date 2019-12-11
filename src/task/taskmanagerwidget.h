#ifndef TASKMANAGERWIDGET_H
#define TASKMANAGERWIDGET_H

#include <QWidget>

class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class TaskManagerLogWidget;
class TaskManager;

class TaskManagerWidget : public QWidget
{
    Q_OBJECT

signals:

public slots:
    void taskClicked(QListWidgetItem* item);

public:
    explicit TaskManagerWidget(TaskManager& task_manager, QWidget *parent=nullptr);

    void updateTaskList ();

protected:
    TaskManager& task_manager_;

    QListWidget* task_list_ {nullptr};
    QStackedWidget* tasks_widget_ {nullptr};
    TaskManagerLogWidget* log_widget_ {nullptr};
};

#endif // TASKMANAGERWIDGET_H
