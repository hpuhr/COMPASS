#include "viewpointstoolwidget.h"
#include "viewpointswidget.h"

#include <QApplication>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QToolButton>

#include "files.h"
#include "logger.h"

using namespace Utils;

ViewPointsToolWidget::ViewPointsToolWidget(ViewPointsWidget* vp_widget, QWidget* parent)
: QWidget(parent), vp_widget_(vp_widget)
{
    assert (vp_widget_);

    setMaximumHeight(40);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    toolbar_ = new QToolBar("Tools");

    // previous
    {
        toolbar_->addAction(QIcon(Files::getIconFilepath("arrow_to_left.png").c_str()),
                            "Select Previous [P]");
    }

    toolbar_->addSeparator();

    // selected

    {
        toolbar_->addAction(QIcon(Files::getIconFilepath("not_recommended.png").c_str()),
                            "Set Selected Status Open [O]");

        toolbar_->addAction(QIcon(Files::getIconFilepath("not_todo.png").c_str()),
                            "Set Selected Status Closed [C]");

        toolbar_->addAction(QIcon(Files::getIconFilepath("todo.png").c_str()),
                            "Set Selected Status ToDo [T]");
    }


    toolbar_->addSeparator();

     // next
     {
         toolbar_->addAction(QIcon(Files::getIconFilepath("arrow_to_right.png").c_str()),
                             "Select Next [N]");
     }

     connect(toolbar_, &QToolBar::actionTriggered, this, &ViewPointsToolWidget::actionTriggeredSlot);

     layout->addWidget(toolbar_);
}

//void ViewPointsToolWidget::toolChangedSignal(ViewPointsTool selected, QCursor cursor)
//{

//}

void ViewPointsToolWidget::actionTriggeredSlot(QAction* action)
{
    std::string text = action->text().toStdString();

    if (text == "Select Previous [P]")
    {
        vp_widget_->selectPreviousSlot();
    }
    else if (text == "Set Selected Status Open [O]")
    {
        vp_widget_->setSelectedOpenSlot();
    }
    else if (text == "Set Selected Status Closed [C]")
    {
        vp_widget_->setSelectedClosedSlot();
    }
    else if (text == "Set Selected Status ToDo [T]")
    {
        vp_widget_->setSelectedTodoSlot();
    }
    else if (text == "Select Next [N]")
    {
        vp_widget_->selectNextSlot();
    }
}

//void ViewPointsToolWidget::loadingStartedSlot()
//{
//    assert(toolbar_);
//    toolbar_->setDisabled(true);
//}

//void ViewPointsToolWidget::loadingDoneSlot()
//{
//    assert(toolbar_);
//    toolbar_->setDisabled(false);
//}
