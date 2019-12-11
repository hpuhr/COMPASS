#include "taskmanagerwidget.h"
#include "taskmanager.h"

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
        task_list_->setSelectionBehavior( QAbstractItemView::SelectItems );
        task_list_->setSelectionMode( QAbstractItemView::SingleSelection );
        //connect (object_parser_list_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectedObjectParserSlot()));
        // updateTasklist

        top_layout->addWidget(task_list_);

        tasks_widget_ = new QStackedWidget ();
        top_layout->addWidget(tasks_widget_);
    }

    main_layout_->addLayout(top_layout);

    log_widget_ = new TaskManagerLogWidget ();

    main_layout_->addWidget(log_widget_);

    setLayout (main_layout_);

}
