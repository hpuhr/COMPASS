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

#include "taskmanagerlogwidget.h"

TaskManagerWidget::TaskManagerWidget(TaskManager& task_manager, QWidget *parent)
    : QWidget(parent), task_manager_(task_manager)
{

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    int frame_width_small = 1;

    QVBoxLayout* main_layout_ = new QVBoxLayout ();

    QHBoxLayout* top_layout = new QHBoxLayout ();

    //    QFrame *left_frame = new QFrame ();
    //    left_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    //    left_frame->setLineWidth(frame_width_small);
    //    left_frame->setLayout(left_layout);

    //    QSizePolicy sp_left(QSizePolicy::Preferred, QSizePolicy::Preferred);
    //    sp_left.setHorizontalStretch(1);
    //    left_frame->setSizePolicy(sp_left);

    // top
    {
        task_list_ = new QListWidget ();
        task_list_->setSelectionBehavior(QAbstractItemView::SelectItems);
        task_list_->setSelectionMode(QAbstractItemView::SingleSelection);
        updateTaskList();
        connect (task_list_, &QListWidget::itemClicked, this, &TaskManagerWidget::taskClicked);

        top_layout->addWidget(task_list_);

        tasks_widget_ = new QStackedWidget ();
        top_layout->addWidget(tasks_widget_);

        for (auto& task_map_it : item_task_mappings_) // select "Open a Database"
        {
            if (task_map_it.first->text() == "Open a Database")
            {
                task_list_->setCurrentItem(task_map_it.first);
                taskClicked (task_map_it.first);
                break;
            }
        }
    }

    main_layout_->addLayout(top_layout);

    log_widget_ = new TaskManagerLogWidget ();

    main_layout_->addWidget(log_widget_);

    setLayout (main_layout_);

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
