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

#include <QWidget>

#include "dbovariable.h"

namespace dbContent
{
class DBContentVariableOrderedSetWidget;
class DBContentVariableSelectionWidget;
}

class ScatterPlotView;

class QCheckBox;
class QLineEdit;
class QPushButton;
class QLabel;

/**
 * @brief Widget with configuration elements for a ScatterPlotView
 *
 */
class ScatterPlotViewConfigWidget : public QWidget
{
    Q_OBJECT

  public slots:
    void selectedVariableXChangedSlot();
    void selectedVariableYChangedSlot();

    void reloadRequestedSlot();
    void loadingStartedSlot();

  public:
    ScatterPlotViewConfigWidget(ScatterPlotView* view, QWidget* parent = nullptr);
    virtual ~ScatterPlotViewConfigWidget();

    void setStatus (const std::string& status, bool visible, QColor color = Qt::black);

  protected:
    ScatterPlotView* view_;

    dbContent::DBContentVariableSelectionWidget* select_var_x_ {nullptr};
    dbContent::DBContentVariableSelectionWidget* select_var_y_ {nullptr};

    QLabel* status_label_ {nullptr};
    QPushButton* reload_button_{nullptr};
};

#endif /* SCATTERPLOTVIEWCONFIGWIDGET_H_ */
