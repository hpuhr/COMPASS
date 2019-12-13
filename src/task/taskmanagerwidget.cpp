#include "taskmanagerwidget.h"
#include "taskmanager.h"
#include "global.h"
#include "logger.h"
#include "files.h"

#include <QListWidget>
#include <QStackedWidget>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QSplitter>
#include <QSettings>
#include <QCheckBox>
#include <QPushButton>

#include "taskmanagerlogwidget.h"

using namespace Utils;

TaskManagerWidget::TaskManagerWidget(TaskManager& task_manager, QWidget *parent)
    : QWidget(parent), task_manager_(task_manager)
{
    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    main_splitter_ = new QSplitter();
    main_splitter_->setOrientation(Qt::Vertical);

    QVBoxLayout* main_layout_ = new QVBoxLayout ();

    QSettings settings ("ATSDB", "TaskManagerWidget");

    setAutoFillBackground(true);

    // top
    {
        QWidget* top_container = new QWidget;
        QVBoxLayout* top_container_layout = new QVBoxLayout;

        top_splitter_ = new QSplitter();

        QWidget* task_stuff_container = new QWidget;
        QVBoxLayout* task_stuff_container_layout = new QVBoxLayout;

        task_list_ = new QListWidget ();
        task_list_->setSelectionBehavior(QAbstractItemView::SelectItems);
        task_list_->setSelectionMode(QAbstractItemView::SingleSelection);
        updateTaskList();
        updateTaskStates();
        connect (task_list_, &QListWidget::itemClicked, this, &TaskManagerWidget::taskClickedSlot);

        task_stuff_container_layout->addWidget(task_list_);

        expert_check_ = new QCheckBox("Expert Mode");
        connect (expert_check_, &QCheckBox::clicked, this, &TaskManagerWidget::expertModeToggledSlot);
        task_stuff_container_layout->addWidget(expert_check_);

        //task_stuff_container_layout->addStretch();

        run_current_task_button_ = new QPushButton ("Run Current Task");
        run_current_task_button_->setDisabled(true);
        QObject::connect(run_current_task_button_, &QPushButton::clicked, this, &TaskManagerWidget::runCurrentTaskSlot);
        task_stuff_container_layout->addWidget(run_current_task_button_);

        start_button_ = new QPushButton ("Start");
        start_button_->setDisabled(true);
        QObject::connect(start_button_, &QPushButton::clicked, this, &TaskManagerWidget::startSlot);
        task_stuff_container_layout->addWidget(start_button_);

        task_stuff_container->setLayout(task_stuff_container_layout);

        top_splitter_->addWidget(task_stuff_container);

        tasks_widget_ = new QStackedWidget ();

        selectNextTask();
        top_splitter_->addWidget(tasks_widget_);
        top_splitter_->restoreState(settings.value("topSplitterSizes").toByteArray());

        top_container_layout->addWidget(top_splitter_);
        top_container->setLayout(top_container_layout);

        main_splitter_->addWidget(top_container);
    }

    log_widget_ = new TaskManagerLogWidget ();

    main_splitter_->addWidget(log_widget_);
    main_splitter_->restoreState(settings.value("mainSplitterSizes").toByteArray());

    main_layout_->addWidget(main_splitter_);

    setLayout (main_layout_);

}

TaskManagerWidget::~TaskManagerWidget ()
{
    loginf  << "TaskManagerWidget: destructor";

    QSettings settings ("ATSDB", "TaskManagerWidget");
    settings.setValue("topSplitterSizes", top_splitter_->saveState());
    settings.setValue("mainSplitterSizes", main_splitter_->saveState());
}

void TaskManagerWidget::updateTaskList ()
{
    assert (task_list_);

    task_list_->clear();
    item_task_mappings_.clear();

    std::vector<std::string> task_list = task_manager_.taskList();
    std::map <std::string, Task*> tasks = task_manager_.tasks();

    Task* current_task;

    for (auto& task_it : task_list)
    {
        assert (tasks.count(task_it));
        current_task = tasks.at(task_it);
        assert (current_task);

        QListWidgetItem* item = new QListWidgetItem(current_task->guiName().c_str(), task_list_); // icon,
        //item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        //item->setCheckState(Qt::Unchecked); // AND initialize check state

        item_task_mappings_[item] = current_task;
    }
}

void TaskManagerWidget::updateTaskStates ()
{
    bool expert_mode = task_manager_.expertMode();

    bool expert_mode_missing;
    bool prerequisites_missing;

    bool all_required_done = true;

    for (auto& item_it : item_task_mappings_)
    {
        expert_mode_missing = !expert_mode && item_it.second->expertOnly();
        prerequisites_missing = !item_it.second->checkPrerequisites();

        //item->setFlags(item->flags() & ~Qt::ItemIsSelectable); Qt::ItemIsEnabled

        loginf << "TaskManagerWidget: updateTaskStates: item " << item_it.first->text().toStdString()
               << " expert missing " << expert_mode_missing << " prereq. missing " << prerequisites_missing
               << " recommended " << item_it.second->isRecommended()
               << " required " << item_it.second->isRequired() << " done " << item_it.second->done();

        if (!expert_mode_missing && !prerequisites_missing) // can be run
        {
            item_it.first->setFlags(item_it.first->flags() | Qt::ItemIsEnabled);

            if (item_it.second->done())
                item_it.first->setIcon(QIcon(Files::getIconFilepath("done.png").c_str()));
            else if (!item_it.second->isRecommended())
                item_it.first->setIcon(QIcon(Files::getIconFilepath("not_recommended.png").c_str()));
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

    if (start_button_ && all_required_done)
        start_button_->setDisabled(false);
}

void TaskManagerWidget::selectNextTask ()
{
    for (auto& task_map_it : item_task_mappings_)
    {
        if ((task_map_it.first->flags() & Qt::ItemIsEnabled) && task_map_it.second->isRecommended())
        {
            task_list_->setCurrentItem(task_map_it.first);
            taskClickedSlot (task_map_it.first);
            break;
        }
    }
}

std::string TaskManagerWidget::getCurrentTask ()
{
    QListWidgetItem* current_item = task_list_->currentItem();
    assert (current_item);

    assert (item_task_mappings_.count(current_item));
    return item_task_mappings_.at(current_item)->name();
}

void TaskManagerWidget::taskClickedSlot(QListWidgetItem* item)
{
    assert (item);
    loginf << "TaskManagerWidget: taskClickedSlot: item '" << item->text().toStdString() << "'";

    if (!(item->flags() & Qt::ItemIsEnabled)) // do nothing if not enabled
        return;

    assert (item_task_mappings_.count(item));
    Task* current_task = item_task_mappings_.at(item);
    assert (current_task);

    if (tasks_widget_->indexOf(current_task->widget()) == -1) // add of not added previously
        tasks_widget_->addWidget(current_task->widget());

    tasks_widget_->setCurrentWidget(current_task->widget());

    run_current_task_button_->setEnabled(current_task->checkPrerequisites());
}

void TaskManagerWidget::expertModeToggledSlot ()
{
    loginf << "TaskManagerWidget: expertModeToggledSlot";

    assert (expert_check_);
    task_manager_.expertMode(expert_check_->checkState() == Qt::Checked);
}

void TaskManagerWidget::runCurrentTaskSlot()
{
    loginf << "TaskManagerWidget: runCurrentTaskSlot";

    task_manager_.runTask(getCurrentTask());
}

void TaskManagerWidget::startSlot ()
{
    loginf << "TaskManagerWidget: startSlot";

    emit task_manager_.startInspectionSignal();
}
