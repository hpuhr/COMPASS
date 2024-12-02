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

#ifndef EVALUATIONDATAWIDGET_H
#define EVALUATIONDATAWIDGET_H

#include <QWidget>

class EvaluationData;
class EvaluationManager;

class QToolBar;
class QTableView;
class QSortFilterProxyModel;
class QMenu;
class QPushButton;

class EvaluationDataWidget : public QWidget
{
    Q_OBJECT

public slots:
    void customContextMenuSlot(const QPoint& p);
    void showFullUTNSlot ();
    void showSurroundingDataSlot ();
    void currentRowChanged(const QModelIndex& current, const QModelIndex& previous);
    void itemClicked(const QModelIndex& index);

public:
    EvaluationDataWidget(EvaluationData& eval_data, EvaluationManager& eval_man);

    void resizeColumnsToContents();
    void updateInterestMenu();

protected:
    void jumpToRequirement(const std::string& req_id, unsigned int utn);

    EvaluationData& eval_data_;
    EvaluationManager& eval_man_;

    //QToolBar* toolbar_ {nullptr};
    QMenu*       interest_menu_   = nullptr;
    QPushButton* interest_button_ = nullptr;

    QTableView* table_view_{nullptr};
    QSortFilterProxyModel* proxy_model_{nullptr};
};

#endif // EVALUATIONDATAWIDGET_H
