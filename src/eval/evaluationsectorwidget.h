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

#ifndef EVALUATIONSECTORWIDGET_H
#define EVALUATIONSECTORWIDGET_H

#include <QScrollArea>

class EvaluationManager;
class EvaluationDialog;

class QGridLayout;

/**
*/
class EvaluationSectorWidget : public QScrollArea
{
    Q_OBJECT

public slots:
    void toggleUseGroupSlot();

public:
    EvaluationSectorWidget(EvaluationManager& eval_man, EvaluationDialog& dialog, QWidget* parent = nullptr);
    virtual ~EvaluationSectorWidget() = default;

    void update();

protected:
    EvaluationManager& eval_man_;
    EvaluationDialog& dialog_;

    QGridLayout* grid_layout_ {nullptr};
};

#endif // EVALUATIONSECTORWIDGET_H
