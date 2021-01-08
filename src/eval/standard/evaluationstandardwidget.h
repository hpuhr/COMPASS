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

#ifndef EVALUATIONSTANDARDWIDGET_H
#define EVALUATIONSTANDARDWIDGET_H

#include <QWidget>
#include <QModelIndex>
#include <QTreeView>

#include <memory>

#include "evaluationstandardtreemodel.h"

class EvaluationStandard;

class QTreeView;
class QStackedWidget;
class QSplitter;

class EvaluationStandardWidget : public QWidget
{
    Q_OBJECT

public slots:
    void itemClickedSlot(const QModelIndex& index);

public:
    EvaluationStandardWidget(EvaluationStandard& standard);
    virtual ~EvaluationStandardWidget();

    EvaluationStandardTreeModel& model();
    void expandAll();

    void showRequirementWidget(QWidget* widget); // can be nullptr

protected:
    EvaluationStandard& standard_;

    EvaluationStandardTreeModel standard_model_;
    std::unique_ptr<QTreeView> tree_view_;

    QSplitter* splitter_ {nullptr};

    QStackedWidget* requirements_widget_{nullptr};
};

#endif // EVALUATIONSTANDARDWIDGET_H
