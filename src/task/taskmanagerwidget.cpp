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

#include "taskmanagerwidget.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSettings>
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "files.h"
#include "global.h"
#include "logger.h"
#include "taskmanager.h"
#include "taskmanagerlogwidget.h"
#include "taskwidget.h"

using namespace Utils;

TaskManagerWidget::TaskManagerWidget(TaskManager& task_manager, QWidget* parent)
    : QWidget(parent), task_manager_(task_manager)
{
    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    main_splitter_ = new QSplitter();
    main_splitter_->setOrientation(Qt::Vertical);
    // main_splitter_->setStyleSheet("QSplitter::handle {background: gray;} ");
    // main_splitter_->setHandleWidth(7);

    //    QSplitterHandle *handle = main_splitter_->handle(1);
    //    QVBoxLayout *layout = new QVBoxLayout(handle);
    //    layout->setSpacing(0);
    //    layout->setMargin(0);

    //    QFrame *line = new QFrame(handle);
    //    line->setFrameShape(QFrame::HLine);
    //    line->setFrameShadow(QFrame::Sunken);
    //    layout->addWidget(line);

    QVBoxLayout* main_layout_ = new QVBoxLayout();

    QSettings settings("ATSDB", "TaskManagerWidget");

    //setAutoFillBackground(true);

    // top
    {
        QWidget* top_container = new QWidget;
        QVBoxLayout* top_container_layout = new QVBoxLayout;

        top_splitter_ = new QSplitter();

        QWidget* task_stuff_container = new QWidget;
        QVBoxLayout* task_stuff_container_layout = new QVBoxLayout;

        task_list_ = new QListWidget();
        task_list_->setSelectionBehavior(QAbstractItemView::SelectItems);
        task_list_->setSelectionMode(QAbstractItemView::SingleSelection);
        updateTaskList();
        updateTaskStates();
        connect(task_list_, &QListWidget::itemClicked, this, &TaskManagerWidget::taskClickedSlot);

        task_stuff_container_layout->addWidget(task_list_);

        expert_check_ = new QCheckBox("Expert Mode");
        expert_check_->setChecked(task_manager_.expertMode());
        connect(expert_check_, &QCheckBox::clicked, this,
                &TaskManagerWidget::expertModeToggledSlot);
        task_stuff_container_layout->addWidget(expert_check_);

        // task_stuff_container_layout->addStretch();

        run_current_task_button_ = new QPushButton("Run Current Task");
        run_current_task_button_->setDisabled(true);
        QObject::connect(run_current_task_button_, &QPushButton::clicked, this,
                         &TaskManagerWidget::runCurrentTaskSlot);
        task_stuff_container_layout->addWidget(run_current_task_button_);

        start_button_ = new QPushButton("Start");
        start_button_->setDisabled(true);
        QObject::connect(start_button_, &QPushButton::clicked, this, &TaskManagerWidget::startSlot);
        task_stuff_container_layout->addWidget(start_button_);

        task_stuff_container->setLayout(task_stuff_container_layout);

        top_splitter_->addWidget(task_stuff_container);

        tasks_widget_ = new QStackedWidget();

        selectNextTask();
        top_splitter_->addWidget(tasks_widget_);
        top_splitter_->restoreState(settings.value("topSplitterSizes").toByteArray());

        top_container_layout->addWidget(top_splitter_);
        top_container->setLayout(top_container_layout);

        main_splitter_->addWidget(top_container);
    }

    log_widget_ = new TaskManagerLogWidget();

    main_splitter_->addWidget(log_widget_);
    main_splitter_->restoreState(settings.value("mainSplitterSizes").toByteArray());

    main_layout_->addWidget(main_splitter_);

    setLayout(main_layout_);
}

TaskManagerWidget::~TaskManagerWidget()
{
    logdbg << "TaskManagerWidget: destructor";

    QSettings settings("ATSDB", "TaskManagerWidget");
    settings.setValue("topSplitterSizes", top_splitter_->saveState());
    settings.setValue("mainSplitterSizes", main_splitter_->saveState());
}

void TaskManagerWidget::updateTaskList()
{
    assert(task_list_);

    task_list_->clear();
    item_task_mappings_.clear();

    std::vector<std::string> task_list = task_manager_.taskList();
    std::map<std::string, Task*> tasks = task_manager_.tasks();

    Task* current_task;

    for (auto& task_it : task_list)
    {
        assert(tasks.count(task_it));
        current_task = tasks.at(task_it);
        assert(current_task);

        QListWidgetItem* item =
            new QListWidgetItem(current_task->guiName().c_str(), task_list_);  // icon,
        if (current_task->tooltip().size())
            item->setToolTip(current_task->tooltip().c_str());
        // item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        // item->setCheckState(Qt::Unchecked); // AND initialize check state

        item_task_mappings_[item] = current_task;
    }
}

void TaskManagerWidget::updateTaskStates()
{
    bool expert_mode = task_manager_.expertMode();

    bool expert_mode_missing;
    bool prerequisites_missing;

    bool all_required_done = true;

    for (auto& item_it : item_task_mappings_)
    {
        expert_mode_missing = !expert_mode && item_it.second->expertOnly();
        prerequisites_missing = !item_it.second->checkPrerequisites();

        // item->setFlags(item->flags() & ~Qt::ItemIsSelectable); Qt::ItemIsEnabled

        loginf << "TaskManagerWidget: updateTaskStates: item "
               << item_it.first->text().toStdString() << " expert missing " << expert_mode_missing
               << " prereq. missing " << prerequisites_missing << " recommended "
               << item_it.second->isRecommended() << " required " << item_it.second->isRequired()
               << " done " << item_it.second->done();

        if (!expert_mode_missing && !prerequisites_missing)  // can be run
        {
            item_it.first->setFlags(item_it.first->flags() | Qt::ItemIsEnabled);

            if (item_it.second->done())
                item_it.first->setIcon(QIcon(Files::getIconFilepath("done.png").c_str()));
            else if (!item_it.second->isRecommended())
                item_it.first->setIcon(
                    QIcon(Files::getIconFilepath("not_recommended.png").c_str()));
            else
                item_it.first->setIcon(QIcon(Files::getIconFilepath("todo.png").c_str()));
        }
        else
        {
            item_it.first->setFlags(item_it.first->flags() & ~Qt::ItemIsEnabled);

            if (item_it.second->done())
                item_it.first->setIcon(QIcon(Files::getIconFilepath("done.png").c_str()));
            else if (expert_mode_missing)
                item_it.first->setIcon(QIcon(Files::getIconFilepath("not_todo.png").c_str()));
            else
                item_it.first->setIcon(QIcon(Files::getIconFilepath("todo_maybe.png").c_str()));
        }

        if (item_it.second->isRequired() && !item_it.second->done())
            all_required_done = false;
    }

    loginf << "TaskManagerWidget: updateTaskStates: all required done " << all_required_done;

    if (start_button_)
    {
        if (all_required_done)
        {
            QFont font_bold;
            font_bold.setBold(true);

            start_button_->setDisabled(false);
            start_button_->setFont(font_bold);
        }
        else
        {
            start_button_->setDisabled(true);
            start_button_->setFont(QFont());
        }
    }
}

void TaskManagerWidget::selectNextTask()
{
    QListWidgetItem* current_item = task_list_->currentItem();

    if (current_item && (current_item->flags() & Qt::ItemIsEnabled) &&
        item_task_mappings_.at(current_item)
            ->isRecommended())  // stay on current item uf still recommended
    {
        taskClickedSlot(current_item);
        return;
    }

    for (auto& task_map_it : item_task_mappings_)
    {
        if ((task_map_it.first->flags() & Qt::ItemIsEnabled) && task_map_it.second->isRecommended())
        {
            task_list_->setCurrentItem(task_map_it.first);
            taskClickedSlot(task_map_it.first);
            return;
        }
    }

    taskClickedSlot(task_list_->currentItem());  // update to change buttons
}

std::string TaskManagerWidget::getCurrentTaskName()
{
    QListWidgetItem* current_item = task_list_->currentItem();
    assert(current_item);

    assert(item_task_mappings_.count(current_item));
    return item_task_mappings_.at(current_item)->name();
}

bool TaskManagerWidget::isStartPossible()
{
    assert(start_button_);
    return start_button_->isEnabled();
}

void TaskManagerWidget::taskClickedSlot(QListWidgetItem* item)
{
    assert(item);
    loginf << "TaskManagerWidget: taskClickedSlot: item '" << item->text().toStdString() << "'";

    if (!(item->flags() & Qt::ItemIsEnabled))  // do nothing if not enabled
        return;

    assert(item_task_mappings_.count(item));
    Task* current_task = item_task_mappings_.at(item);
    assert(current_task);

    setCurrentTask(*current_task);
}

void TaskManagerWidget::setCurrentTask(Task& task)
{
    loginf << "TaskManagerWidget: setCurrentTask: task '" << task.name() << "' can run "
           << task.canRun() << " done " << task.done() << " is recommended "
           << task.isRecommended();

    // check if selected in list
    assert(task_list_);

    QList<QListWidgetItem*> items = task_list_->findItems(task.guiName().c_str(), Qt::MatchExactly);
    assert(items.size() == 1);
    QListWidgetItem* task_item = items.at(0);
    if (task_list_->currentItem() != task_item)
        task_list_->setCurrentItem(task_item);

    // set widget
    if (tasks_widget_->indexOf(task.widget()) == -1)  // add of not added previously
        tasks_widget_->addWidget(task.widget());

    tasks_widget_->setCurrentWidget(task.widget());

    // update run current task button
    if (task.canRun())
    {
        QFont font_bold;
        font_bold.setBold(true);

        run_current_task_button_->setEnabled(true);

        if (!task.done() && task.isRecommended())
            run_current_task_button_->setFont(font_bold);
        else
            run_current_task_button_->setFont(QFont());
    }
    else
    {
        run_current_task_button_->setEnabled(false);
        run_current_task_button_->setFont(QFont());
    }
}

void TaskManagerWidget::expertModeToggledSlot()
{
    loginf << "TaskManagerWidget: expertModeToggledSlot";

    assert(expert_check_);
    task_manager_.expertMode(expert_check_->checkState() == Qt::Checked);
}

void TaskManagerWidget::runCurrentTaskSlot()
{
    loginf << "TaskManagerWidget: runCurrentTaskSlot: task " << getCurrentTaskName();

    task_manager_.runTask(getCurrentTaskName());
}

void TaskManagerWidget::runTask(Task& task)
{
    loginf << "TaskManagerWidget: runTask: name '" << task.name() << "'";
    setCurrentTask(task);
    task_manager_.runTask(task.name());
}


void TaskManagerWidget::startSlot()
{
    loginf << "TaskManagerWidget: startSlot";

    emit task_manager_.startInspectionSignal();
}
