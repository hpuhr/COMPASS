#include "scatterplotviewdatatoolwidget.h"
#include "files.h"
#include "logger.h"
#include "scatterplotview.h"

#include <QApplication>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QToolButton>

using namespace Utils;

ScatterPlotViewDataToolWidget::ScatterPlotViewDataToolWidget(ScatterPlotView* view, QWidget* parent)
    : QWidget(parent), view_(view)
{
    setMaximumHeight(40);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    toolbar_ = new QToolBar("Tools");

    // tool actions
    {
        QAction* nav_action_ =
            toolbar_->addAction(QIcon(Files::getIconFilepath("navigate.png").c_str()), "Navigate");
        navigate_button_ = dynamic_cast<QToolButton*>(toolbar_->widgetForAction(nav_action_));
        assert(navigate_button_);
        navigate_button_->setCheckable(true);
        navigate_button_->setChecked(true);
    }

    {
        QAction* select_action = toolbar_->addAction(
            QIcon(Files::getIconFilepath("select_action.png").c_str()), "Select");
        select_button_ = dynamic_cast<QToolButton*>(toolbar_->widgetForAction(select_action));
        assert(select_button_);
        select_button_->setCheckable(true);
    }

    {
        QAction* zoom_action_ =
            toolbar_->addAction(QIcon(Files::getIconFilepath("zoom_select_action.png").c_str()), "Zoom to Rectangle");
        zoom_rect_button_ = dynamic_cast<QToolButton*>(toolbar_->widgetForAction(zoom_action_));
        assert(zoom_rect_button_);
        zoom_rect_button_->setCheckable(true);
    }

    QWidget* empty = new QWidget();
    empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolbar_->addWidget(empty);

    {
//        toolbar_->addAction(QIcon(Files::getIconFilepath("select_color.png").c_str()),
//                            "Edit Selection Color");
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

    connect(toolbar_, &QToolBar::actionTriggered, this, &ScatterPlotViewDataToolWidget::actionTriggeredSlot);

    layout->addWidget(toolbar_);
}


ScatterPlotViewDataToolWidget::~ScatterPlotViewDataToolWidget()
{

}

void ScatterPlotViewDataToolWidget::actionTriggeredSlot(QAction* action)
{
    std::string text = action->text().toStdString();

    if (text == "Navigate")
    {
        unselectAllTools();
        navigate_button_->setChecked(true);
        emit toolChangedSignal(SP_NAVIGATE_TOOL, Qt::OpenHandCursor);
    }
    else if (text == "Zoom to Rectangle")
    {
        unselectAllTools();
        zoom_rect_button_->setChecked(true);
        emit toolChangedSignal(SP_ZOOM_RECT_TOOL, Qt::CrossCursor);
    }
//    else if (text == "Label")
//    {
//        unselectAllTools();
//        label_button_->setChecked(true);
//        emit toolChangedSignal(LABEL_TOOL, Qt::PointingHandCursor);
//    }
//    else if (text == "Label Multiple")
//    {
//        unselectAllTools();
//        label_multiple_button_->setChecked(true);
//        emit toolChangedSignal(LABEL_MULTIPLE_TOOL, Qt::CrossCursor);
//    }
//    else if (text == "Measure")
//    {
//        unselectAllTools();
//        measure_button_->setChecked(true);
//        emit toolChangedSignal(MEASURE_TOOL, Qt::ArrowCursor);
//    }
    else if (text == "Select")
    {
        unselectAllTools();
        select_button_->setChecked(true);
        emit toolChangedSignal(SP_SELECT_TOOL, Qt::CrossCursor);
    }

//    else if (text == "Toogle Time Filter")
//    {
//        assert(time_filter_button_);
//        bool filter = !view_->getTimeFilter();
//        time_filter_button_->setChecked(filter);
//        emit timeFilterEnabledSignal(filter);
//        view_->setTimeFilter(filter);
//    }
//    else if (text == "Toogle Depth Check")
//    {
//        assert(depth_check_button_);
//        bool use = !view_->useDepthCheck();
//        depth_check_button_->setChecked(use);
//        view_->useDepthCheck(use);
//    }
//    else if (text == "Edit Label Color")
//    {
//        setLabelBackgroundColorSlot();
//    }
//    else if (text == "Save View Point")
//    {
//        emit saveViewPointSignal();
//    }
//    else if (text == "Edit Label Content")
//    {
//        labelMenuSlot();
//    }
//    else if (text == "Delete All Labels")
//    {
//        emit clearLabelsSignal();
//    }
//    else if (text == "Delete All Measurements")
//    {
//        emit clearMeasurementsSignal();
//    }
//    else if (text == "Edit Selection Color")
//    {
//        setSelectionColorSlot();
//    }
    else if (text == "Invert Selection")
    {
        emit invertSelectionSignal();
    }
    else if (text == "Delete Selection")
    {
        emit clearSelectionSignal();
    }
//    else if (text == "Overlay Text Color Invert")
//    {
//        overlayColorInvertSlot();
//    }
//    else if (text == "Switch Map Dimensions")
//    {
//        emit switchDimensionsSignal();
//    }
    else if (text == "Zoom to Home")
    {
        emit zoomToHomeSignal();
    }
    else
        logerr << "ScatterPlotViewDataToolWidget: actionTriggeredSlot: unknown action '" << text << "'";
}

void ScatterPlotViewDataToolWidget::unselectAllTools()
{
    navigate_button_->setChecked(false);
    zoom_rect_button_->setChecked(false);
    //label_button_->setChecked(false);
    //measure_button_->setChecked(false);
    select_button_->setChecked(false);
    //label_multiple_button_->setChecked(false);
}

void ScatterPlotViewDataToolWidget::loadingStartedSlot()
{
    loginf << "ScatterPlotViewDataToolWidget: loadingStartedSlot";

    assert(toolbar_);
    toolbar_->setDisabled(true);
}

void ScatterPlotViewDataToolWidget::loadingDoneSlot()
{
    assert(toolbar_);
    toolbar_->setDisabled(false);
}
