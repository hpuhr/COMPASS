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

#ifndef EVALUATIONREQUIREMENTMODECFALSECONFIGWIDGET_H
#define EVALUATIONREQUIREMENTMODECFALSECONFIGWIDGET_H

#include "eval/requirement/base/probabilitybaseconfigwidget.h"

class QLineEdit;
class QCheckBox;

class QFormLayout;

namespace EvaluationRequirement
{
class ModeCFalseConfig;

class ModeCFalseConfigWidget : public ProbabilityBaseConfigWidget
{
    Q_OBJECT

public slots:
    void maxDiffEditSlot(QString value);

public:
    ModeCFalseConfigWidget(ModeCFalseConfig& cfg);

protected:
    QLineEdit* max_diff_edit_{nullptr};

    ModeCFalseConfig& config();
};

}

#endif // EVALUATIONREQUIREMENTMODECFALSECONFIGWIDGET_H
