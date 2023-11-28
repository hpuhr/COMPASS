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

#include "evaluationtargetstabwidget.h"

#include "evaluationmanagerwidget.h"
#include "evaluationmanager.h"
//#include "logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

EvaluationTargetsTabWidget::EvaluationTargetsTabWidget(EvaluationManager& eval_man,
                                                       EvaluationManagerWidget& man_widget)
    : QWidget(nullptr), eval_man_(eval_man), man_widget_(man_widget)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    main_layout->addWidget(eval_man_.getData().widget());

    setContentsMargins(0, 0, 0, 0);
    setLayout(main_layout);
}
