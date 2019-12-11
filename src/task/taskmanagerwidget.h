#ifndef TASKMANAGERWIDGET_H
#define TASKMANAGERWIDGET_H

#include <QWidget>

class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class QSplitter;
class Task;
class TaskManagerLogWidget;
class TaskManager;

class TaskManagerWidget : public QWidget
{
    Q_OBJECT

signals:

public slots:
    void taskClicked(QListWidgetItem* item);

public:
    explicit TaskManagerWidget(TaskManager& task_manager, QWidget* parent=nullptr);
    ~TaskManagerWidget ();

    void updateTaskList ();

protected:
    TaskManager& task_manager_;

    QListWidget* task_list_ {nullptr};
    QStackedWidget* tasks_widget_ {nullptr};
    TaskManagerLogWidget* log_widget_ {nullptr};

    QSplitter* top_splitter_ {nullptr};
    QSplitter* main_splitter_ {nullptr};

    std::map <QListWidgetItem*, Task*> item_task_mappings_;

    //void closeEvent(QCloseEvent* event);
};

#endif // TASKMANAGERWIDGET_H
