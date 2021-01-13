/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "scatterplotviewwidget.h"

#include <QHBoxLayout>
#include <QSettings>
#include <QSplitter>
#include <QTabWidget>

#include "scatterplotview.h"
#include "scatterplotviewconfigwidget.h"
#include "scatterplotviewdatawidget.h"
#include "scatterplotviewdatatoolwidget.h"

/*
 */
ScatterPlotViewWidget::ScatterPlotViewWidget(const std::string& class_id, const std::string& instance_id,
                                     Configurable* config_parent, ScatterPlotView* view,
                                     QWidget* parent)
    : ViewWidget(class_id, instance_id, config_parent, view, parent),
      data_widget_(nullptr),
      config_widget_(nullptr)
{
    //setAutoFillBackground(true);

    QHBoxLayout* hlayout = new QHBoxLayout;
    hlayout->setContentsMargins(0, 0, 0, 0);

    main_splitter_ = new QSplitter();
    main_splitter_->setOrientation(Qt::Horizontal);

    QSettings settings("COMPASS", instanceId().c_str());

    {  // data stuff

        QWidget* data_layout_widget = new QWidget();
        QSizePolicy sp_left(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sp_left.setHorizontalStretch(5);
        data_layout_widget->setSizePolicy(sp_left);
        data_layout_widget->setContentsMargins(0, 0, 0, 0);

        QVBoxLayout* data_layout = new QVBoxLayout;
        data_layout->setContentsMargins(0, 0, 0, 0);

        tool_widget_ = new ScatterPlotViewDataToolWidget(view, this);
        tool_widget_->setContentsMargins(0, 0, 0, 0);
        data_layout->addWidget(tool_widget_);

        data_widget_ = new ScatterPlotViewDataWidget(getView(), view->getDataSource());
        //data_widget_->setAutoFillBackground(true);
        //QSizePolicy sp_left(QSizePolicy::Preferred, QSizePolicy::Preferred);
        //sp_left.setHorizontalStretch(3);
        //data_widget_->setSizePolicy(sp_left);
        data_layout->addWidget(data_widget_);

        //main_splitter_->addWidget(data_widget_);
        data_layout_widget->setLayout(data_layout);
        main_splitter_->addWidget(data_layout_widget);
    }

    {  // config widget
        config_widget_ = new ScatterPlotViewConfigWidget(getView());
        //config_widget_->setAutoFillBackground(true);
        QSizePolicy sp_right(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sp_right.setHorizontalStretch(1);
        config_widget_->setSizePolicy(sp_right);

        // hlayout->addWidget( config_widget_ );
        main_splitter_->addWidget(config_widget_);
    }

    main_splitter_->restoreState(settings.value("mainSplitterSizes").toByteArray());
    hlayout->addWidget(main_splitter_);

    setLayout(hlayout);
    setContentsMargins(0, 0, 0, 0);

    setFocusPolicy(Qt::StrongFocus);

    // connect stuff here
    // connect( config_widget_, SIGNAL(variableChanged()), this, SLOT(variableChangedSlot()) );

    connect(tool_widget_, &ScatterPlotViewDataToolWidget::toolChangedSignal, data_widget_,
            &ScatterPlotViewDataWidget::toolChangedSlot);

    connect(tool_widget_, &ScatterPlotViewDataToolWidget::invertSelectionSignal, data_widget_,
            &ScatterPlotViewDataWidget::invertSelectionSlot);
    connect(tool_widget_, &ScatterPlotViewDataToolWidget::clearSelectionSignal, data_widget_,
            &ScatterPlotViewDataWidget::clearSelectionSlot);

    connect(tool_widget_, &ScatterPlotViewDataToolWidget::zoomToHomeSignal, data_widget_,
            &ScatterPlotViewDataWidget::resetZoomSlot);

}

/*
 */
ScatterPlotViewWidget::~ScatterPlotViewWidget()
{
    QSettings settings("COMPASS", instanceId().c_str());
    settings.setValue("mainSplitterSizes", main_splitter_->saveState());
}

/*
 */
void ScatterPlotViewWidget::updateView() {}

/*
 */
void ScatterPlotViewWidget::toggleConfigWidget()
{
    assert(config_widget_);
    bool vis = config_widget_->isVisible();
    config_widget_->setVisible(!vis);
}

/*
 */
ScatterPlotViewConfigWidget* ScatterPlotViewWidget::configWidget() { return config_widget_; }
