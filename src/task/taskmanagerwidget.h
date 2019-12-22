/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TASKMANAGERWIDGET_H
#define TASKMANAGERWIDGET_H

#include <QWidget>
#include <cassert>

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
    void runCurrentTaskSlot();
    void startSlot ();

public:
    explicit TaskManagerWidget(TaskManager& task_manager, QWidget* parent=nullptr);
    ~TaskManagerWidget ();

    void updateTaskList ();
    void updateTaskStates ();
    void selectNextTask ();

    std::string getCurrentTask ();

    TaskManagerLogWidget* logWidget() { assert (log_widget_); return log_widget_; }

protected:
    TaskManager& task_manager_;

    QListWidget* task_list_ {nullptr};
    QCheckBox* expert_check_ {nullptr};
    QPushButton* run_current_task_button_ {nullptr};
    QPushButton* start_button_ {nullptr};

    QStackedWidget* tasks_widget_ {nullptr};
    TaskManagerLogWidget* log_widget_ {nullptr};

    QSplitter* top_splitter_ {nullptr};
    QSplitter* main_splitter_ {nullptr};

    std::map <QListWidgetItem*, Task*> item_task_mappings_;
};

#endif // TASKMANAGERWIDGET_H
