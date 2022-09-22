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

#include "dbcontent/variable/variable.h"

class HistogramView;

namespace dbContent {
class VariableOrderedSetWidget;
class VariableSelectionWidget;
}

class QCheckBox;
class QLineEdit;
class QPushButton;
class QLabel;

/**
 * @brief Widget with configuration elements for a HistogramView
 *
 */
class HistogramViewConfigWidget : public ViewConfigWidget
{
    Q_OBJECT

  public slots:
    void showSelectedVariableDataSlot();
    void showEvaluationResultDataSlot();
    void selectedVariableChangedSlot();

    void toggleLogScale();

//    void exportSlot();
//    void exportDoneSlot(bool cancelled);

    void reloadRequestedSlot();
    void loadingStartedSlot();

  public:
    HistogramViewConfigWidget(HistogramView* view, QWidget* parent = nullptr);
    virtual ~HistogramViewConfigWidget();

    void updateEvalConfig();

    virtual void setStatus(const QString& text, bool visible, const QColor& color = Qt::black) override;

  protected:
    HistogramView* view_;

    // data variable
    QCheckBox* selected_var_check_ {nullptr}; // active if variable data is shown
    dbContent::VariableSelectionWidget* select_var_ {nullptr};

    // eval
    QCheckBox* eval_results_check_ {nullptr}; // active if eval data is shown
    QLabel* eval_results_grpreq_label_{nullptr};
    QLabel* eval_results_id_label_{nullptr};

    QCheckBox* log_check_ {nullptr};
    //QPushButton* export_button_{nullptr};

    QLabel* status_label_ {nullptr};
    QPushButton* reload_button_{nullptr};
};

#endif /* HISTOGRAMVIEWCONFIGWIDGET_H_ */
