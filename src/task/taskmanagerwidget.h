#ifndef TASKMANAGERWIDGET_H
#define TASKMANAGERWIDGET_H

#include <QWidget>

class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class QSplitter;
class QPushButton;
class QCheckBox;

class Task;
class TaskManagerLogWidget;
class TaskManager;

class TaskManagerWidget : public QWidget
{
    Q_OBJECT

signals:

public slots:
    void taskClickedSlot(QListWidgetItem* item);
    void expertModeToggledSlot ();
    void startSlot ();

public:
    explicit TaskManagerWidget(TaskManager& task_manager, QWidget* parent=nullptr);
    ~TaskManagerWidget ();

    void updateTaskList ();
    void updateTaskStates ();
    void selectNextTask ();

    TaskManagerLogWidget* logWidget() { assert (log_widget_); return log_widget_; }

protected:
    TaskManager& task_manager_;

    QListWidget* task_list_ {nullptr};
    QCheckBox* expert_check_ {nullptr};
    QPushButton* start_button_ {nullptr};

    QStackedWidget* tasks_widget_ {nullptr};
    TaskManagerLogWidget* log_widget_ {nullptr};

    QSplitter* top_splitter_ {nullptr};
    QSplitter* main_splitter_ {nullptr};

    std::map <QListWidgetItem*, Task*> item_task_mappings_;
};

#endif // TASKMANAGERWIDGET_H
