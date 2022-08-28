#include "histogramviewdatatoolwidget.h"
#include "files.h"
#include "logger.h"
#include "histogramview.h"

#include <QApplication>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QToolButton>

using namespace Utils;
HistogramViewDataToolWidget::HistogramViewDataToolWidget(HistogramView* view, QWidget* parent)
    : QWidget(parent), view_(view)
{
    setMaximumHeight(40);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    toolbar_ = new QToolBar("Tools");

    // tool actions
    {
        QAction* select_action = toolbar_->addAction(
            QIcon(Files::getIconFilepath("interaction.png").c_str()), "Select");
        select_button_ = dynamic_cast<QToolButton*>(toolbar_->widgetForAction(select_action));
        assert(select_button_);
        select_button_->setCheckable(true);
        select_button_->setChecked(true);
    }

    QWidget* empty = new QWidget();
    empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolbar_->addWidget(empty);

    {
        toolbar_->addAction(QIcon(Files::getIconFilepath("select_invert.png").c_str()),
                            "Invert Selection");
        toolbar_->addAction(QIcon(Files::getIconFilepath("select_delete.png").c_str()),
                            "Delete Selection");
    }

    QWidget* empty2 = new QWidget();
    empty2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolbar_->addWidget(empty2);

    {
        toolbar_->addAction(QIcon(Files::getIconFilepath("zoom_home.png").c_str()),
                            "Zoom to Home");
    }

    connect(toolbar_, &QToolBar::actionTriggered, this, &HistogramViewDataToolWidget::actionTriggeredSlot);

    layout->addWidget(toolbar_);
}

HistogramViewDataToolWidget::~HistogramViewDataToolWidget()
{

}

void HistogramViewDataToolWidget::actionTriggeredSlot(QAction* action)
{
    std::string text = action->text().toStdString();

    if (text == "Select")
    {
        unselectAllTools();
        select_button_->setChecked(true);
        //emit toolChangedSignal(SP_SELECT_TOOL, Qt::CrossCursor);
    }
    else if (text == "Invert Selection")
    {
        emit invertSelectionSignal();
    }
    else if (text == "Delete Selection")
    {
        emit clearSelectionSignal();
    }
    else if (text == "Zoom to Home")
    {
        emit zoomToHomeSignal();
    }
    else
        logerr << "HistogramViewDataToolWidget: actionTriggeredSlot: unknown action '" << text << "'";
}

void HistogramViewDataToolWidget::unselectAllTools()
{
    select_button_->setChecked(false);
}

void HistogramViewDataToolWidget::loadingStartedSlot()
{
    loginf << "HistogramViewDataToolWidget: loadingStartedSlot";

    assert(toolbar_);
    toolbar_->setDisabled(true);
}

void HistogramViewDataToolWidget::loadingDoneSlot()
{
    assert(toolbar_);
    toolbar_->setDisabled(false);
}
