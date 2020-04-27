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

    // tool actions
    {
        toolbar_->addAction(QIcon(Files::getIconFilepath("arrow_to_left.png").c_str()),
                            "Select Previous");
    }

     toolbar_->addSeparator();

     {
         toolbar_->addAction(QIcon(Files::getIconFilepath("arrow_to_right.png").c_str()),
                             "Select Next");
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

    if (text == "Select Previous")
    {
        vp_widget_->selectPreviousSlot();
    }
    else if (text == "Select Next")
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
