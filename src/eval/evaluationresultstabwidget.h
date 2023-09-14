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

#ifndef EVALUATIONRESULTSTABWIDGET_H
#define EVALUATIONRESULTSTABWIDGET_H

#include <QWidget>
#include <QTreeView>
#include <QStackedWidget>

#include <memory>

#include <boost/optional.hpp>

#include "json.h"

class EvaluationManager;
class EvaluationManagerWidget;

class QPushButton;
class QSplitter;

class EvaluationResultsTabWidget : public QWidget
{
    Q_OBJECT

public slots:
    void itemClickedSlot(const QModelIndex& index);

    void stepBackSlot();

public:
    EvaluationResultsTabWidget(EvaluationManager& eval_man, EvaluationManagerWidget& man_widget);
    virtual ~EvaluationResultsTabWidget();

    void expand();

    void showResultWidget(QWidget* widget); // can be nullptr

    void selectId (const std::string& id);
    void reshowLastId ();

    boost::optional<nlohmann::json> getTableData(const std::string& result_id, const std::string& table_id) const;

protected:
    EvaluationManager& eval_man_;
    EvaluationManagerWidget& man_widget_;

    QSplitter* splitter_ {nullptr};
    std::unique_ptr<QTreeView> tree_view_;

    QStackedWidget* results_widget_{nullptr};

    QPushButton* back_button_ {nullptr};
    std::vector<std::string> id_history_;

    void expandAllParents (QModelIndex index);
    void updateBackButton ();
};

#endif // EVALUATIONRESULTSTABWIDGET_H
