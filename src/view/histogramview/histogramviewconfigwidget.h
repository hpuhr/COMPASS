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

#include <QWidget>

#include "dbovariable.h"

class DBOVariableOrderedSetWidget;
class QCheckBox;
class HistogramView;
class QLineEdit;
class QPushButton;

/**
 * @brief Widget with configuration elements for a HistogramView
 *
 */
class HistogramViewConfigWidget : public QWidget
{
    Q_OBJECT

  public slots:
//    void toggleShowOnlySeletedSlot();
//    void toggleUsePresentation();
//    void toggleUseOverwrite();
//    void showAssociationsSlot();

    void exportSlot();
    void exportDoneSlot(bool cancelled);

    void reloadWantedSlot();
    void reloadRequestedSlot();
    void loadingStartedSlot();

  signals:
    void exportSignal(bool overwrite);
    void reloadRequestedSignal();  // reload from database

  public:
    /// @brief Constructor
    HistogramViewConfigWidget(HistogramView* view, QWidget* parent = nullptr);
    /// @brief Destructor
    virtual ~HistogramViewConfigWidget();

  protected:
    /// Base view
    HistogramView* view_;
    /// Variable read list widget
    //DBOVariableOrderedSetWidget* variable_set_widget_{nullptr};

//    QCheckBox* only_selected_check_{nullptr};
//    QCheckBox* presentation_check_{nullptr};
//    QCheckBox* associations_check_{nullptr};

//    QCheckBox* overwrite_check_{nullptr};

    QPushButton* export_button_{nullptr};

    QPushButton* update_button_{nullptr};
    bool reload_needed_{false};

    void updateUpdateButton();
};

#endif /* HISTOGRAMVIEWCONFIGWIDGET_H_ */
