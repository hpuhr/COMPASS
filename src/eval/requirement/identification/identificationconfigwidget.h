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

#ifndef EVALUATIONREQUIREMENTIDENTIFICATIONCONFIGWIDGET_H
#define EVALUATIONREQUIREMENTIDENTIFICATIONCONFIGWIDGET_H

#include "eval/requirement/base/baseconfigwidget.h"

class QLineEdit;
class QCheckBox;

class QFormLayout;

namespace EvaluationRequirement
{
class IdentificationConfig;

class IdentificationConfigWidget : public BaseConfigWidget
{
    Q_OBJECT

public slots:
    void toggleRequireCorrectnessSlot();
    void toggleRequireCorrectnessOfAllSlot();

    void toggleUseModeASlot();
    void toggleUseMsTaSlot();
    void toggleUseMsTiSlot();


public:
    IdentificationConfigWidget(IdentificationConfig& cfg);

protected:
    QCheckBox* require_correctness_check_{nullptr};
    QCheckBox* require_correctness_of_all_check_{nullptr};

    // mode a ssr code
    QCheckBox* use_mode_a_check_{nullptr};
    // 24-bit mode s address
    QCheckBox* use_ms_ta_check_{nullptr};
    // downlinked aircraft identification
    QCheckBox* use_ms_ti_check_{nullptr};


    IdentificationConfig& config();
};

}

#endif // EVALUATIONREQUIREMENTIDENTIFICATIONCONFIGWIDGET_H
