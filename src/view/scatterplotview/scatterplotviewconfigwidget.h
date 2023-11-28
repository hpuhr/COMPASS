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

#ifndef SCATTERPLOTVIEWCONFIGWIDGET_H_
#define SCATTERPLOTVIEWCONFIGWIDGET_H_

#include "viewconfigwidget.h"
//#include "dbcontent/variable/variable.h"
//#include "appmode.h"

namespace dbContent
{
class VariableOrderedSetWidget;
class VariableSelectionWidget;
}

class ScatterPlotViewWidget;
class ScatterPlotView;

class QCheckBox;
class QLineEdit;

/**
 * @brief Widget with configuration elements for a ScatterPlotView
 *
 */
class ScatterPlotViewConfigWidget : public ViewConfigWidget
{
    Q_OBJECT

public slots:
    void selectedVariableXChangedSlot();
    void selectedVariableYChangedSlot();

    void useConnectionLinesSlot();

public:
    ScatterPlotViewConfigWidget(ScatterPlotViewWidget* view_widget, QWidget* parent = nullptr);
    virtual ~ScatterPlotViewConfigWidget();

    virtual void configChanged() override;

protected:
    void updateSelectedVarX();
    void updateSelectedVarY();

    ScatterPlotView* view_ = nullptr;

    dbContent::VariableSelectionWidget* select_var_x_ {nullptr};
    dbContent::VariableSelectionWidget* select_var_y_ {nullptr};

    QCheckBox* use_connection_lines_ {nullptr};

    virtual void onDisplayChange_impl() override;
};

#endif /* SCATTERPLOTVIEWCONFIGWIDGET_H_ */
