#include "taskmanagerwidget.h"
#include "taskmanager.h"
#include "global.h"
#include "logger.h"

#include <QListWidget>
#include <QStackedWidget>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QSplitter>
#include <QSettings>

#include "taskmanagerlogwidget.h"

TaskManagerWidget::TaskManagerWidget(TaskManager& task_manager, QWidget *parent)
    : QWidget(parent), task_manager_(task_manager)
{
    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    int frame_width_small = 1;

    main_splitter_ = new QSplitter();
    main_splitter_->setOrientation(Qt::Vertical);

    QVBoxLayout* main_layout_ = new QVBoxLayout ();

    QSettings settings ("ATSDB", "TaskManagerWidget");

    // top
    {
        QWidget* top_container = new QWidget;
        QVBoxLayout* top_container_layout = new QVBoxLayout;

        top_splitter_ = new QSplitter();

        task_list_ = new QListWidget ();
        task_list_->setSelectionBehavior(QAbstractItemView::SelectItems);
        task_list_->setSelectionMode(QAbstractItemView::SingleSelection);
        updateTaskList();
        connect (task_list_, &QListWidget::itemClicked, this, &TaskManagerWidget::taskClicked);

        top_splitter_->addWidget(task_list_);

        tasks_widget_ = new QStackedWidget ();

        for (auto& task_map_it : item_task_mappings_) // select "Open a Database"
        {
            if (task_map_it.first->text() == "Open a Database")
            {
                task_list_->setCurrentItem(task_map_it.first);
                taskClicked (task_map_it.first);
                break;
            }
        }
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
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        item->setCheckState(Qt::Unchecked); // AND initialize check state

        item_task_mappings_[item] = current_task;
    }

//#if USE_JASTERIX
//    QListWidgetItem* import_asterix_item = new QListWidgetItem("Import ASTERIX Recording", task_list_); // icon,
//    import_asterix_item->setFlags(import_asterix_item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
//    import_asterix_item->setCheckState(Qt::Unchecked); // AND initialize check state
//#endif

//    QListWidgetItem* import_json_item = new QListWidgetItem("Import JSON data", task_list_); // icon,
//    import_json_item->setFlags(import_asterix_item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
//    import_json_item->setCheckState(Qt::Unchecked); // AND initialize check state

//    QListWidgetItem* calc_radar_pos_item = new QListWidgetItem("Calculate Radar Plot Positions", task_list_); // icon,
//    calc_radar_pos_item->setFlags(import_asterix_item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
//    calc_radar_pos_item->setCheckState(Qt::Unchecked); // AND initialize check state

//    QListWidgetItem* calc_artas_assoc_item = new QListWidgetItem("Calculate ARTAS Associations", task_list_); // icon,
//    calc_artas_assoc_item->setFlags(import_asterix_item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
//    calc_artas_assoc_item->setCheckState(Qt::Unchecked); // AND initialize check state

    //        QListWidgetItem* item = new QListWidgetItem(icon, "item", listWidget);
    //        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
    //        item->setCheckState(Qt::Unchecked); // AND initialize check state
}

void TaskManagerWidget::taskClicked(QListWidgetItem* item)
{
    assert (item);
    loginf << "TaskManagerWidget: taskClicked: item '" << item->text().toStdString() << "'";

    assert (item_task_mappings_.count(item));
    Task* current_task = item_task_mappings_.at(item);
    assert (current_task);

    if (tasks_widget_->indexOf(current_task->widget()) == -1) // add of not added previously
        tasks_widget_->addWidget(current_task->widget());

    tasks_widget_->setCurrentWidget(current_task->widget());
}
