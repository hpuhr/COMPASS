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

#ifndef HISTOGRAMVIEWCONFIGWIDGET_H_
#define HISTOGRAMVIEWCONFIGWIDGET_H_

#include "viewconfigwidget.h"
//#include "dbcontent/variable/variable.h"
//#include "appmode.h"

class HistogramView;
class HistogramViewWidget;

namespace dbContent {
class VariableOrderedSetWidget;
class VariableSelectionWidget;
}

class QCheckBox;
class QRadioButton;
class QLineEdit;
class QPushButton;
class QLabel;
class GroupBox;

/**
 * @brief Widget with configuration elements for a HistogramView
 *
 */
class HistogramViewConfigWidget : public ViewConfigWidget
{
    Q_OBJECT

public slots:
    void dataSourceToggled();
    void selectedVariableChangedSlot();

    void toggleLogScale();

//    void exportSlot();
//    void exportDoneSlot(bool cancelled);

public:
    HistogramViewConfigWidget(HistogramViewWidget* view_widget, QWidget* parent = nullptr);
    virtual ~HistogramViewConfigWidget();

    void updateConfig();

    virtual void configChanged() override;

protected:
    void updateEvalConfig();
    void updateInfo();
    void updateSelectedVar();
    void updateLogScale();

    virtual void onDisplayChange_impl() override;

    HistogramView* view_ = nullptr;

    // data variable
    QRadioButton*                       selected_var_check_ {nullptr}; // active if variable data is shown
    QWidget*                            selected_var_widget_{nullptr};
    dbContent::VariableSelectionWidget* select_var_         {nullptr};

    // eval
    QRadioButton* eval_results_check_       {nullptr}; // active if eval data is shown
    QWidget*      eval_results_widget_      {nullptr};
    QLabel*       eval_results_grpreq_label_{nullptr};
    QLabel*       eval_results_id_label_    {nullptr};

    // histogram info
    GroupBox*     info_widget_          = nullptr;
    QLabel*       info_range_min_label_ = nullptr;
    QLabel*       info_range_max_label_ = nullptr;
    QLabel*       info_oor_label_       = nullptr;
    QLabel*       info_zoom_label       = nullptr;

    // general
    QCheckBox*    log_check_{nullptr};
};

#endif /* HISTOGRAMVIEWCONFIGWIDGET_H_ */
