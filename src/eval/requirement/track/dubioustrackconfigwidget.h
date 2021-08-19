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

#ifndef EVALUATIONREQUIREMENTDUBIOUSTRACKCONFIGWIDGET_H
#define EVALUATIONREQUIREMENTDUBIOUSTRACKCONFIGWIDGET_H

#include "eval/requirement/base/baseconfigwidget.h"

class ComparisonTypeComboBox;

class QLineEdit;
class QCheckBox;

class QFormLayout;

namespace EvaluationRequirement
{

class DubiousTrackConfig;

class DubiousTrackConfigWidget : public BaseConfigWidget
{
    Q_OBJECT

public slots:
    void toggleMarkPrimaryOnlySlot();

    void toggleUseMinUpdatesSlot();
    void minUpdatesEditSlot(QString value);

    void toggleUseMinDurationSlot();
    void minDurationEditSlot(QString value);

public:
    DubiousTrackConfigWidget(DubiousTrackConfig& cfg);

protected:
    QCheckBox* mark_primary_only_check_{nullptr};

    QCheckBox* use_min_updates_check_{nullptr};
    QLineEdit* min_updates_edit_{nullptr};

    QCheckBox* use_min_duration_check_{nullptr};
    QLineEdit* min_duration_edit_{nullptr};

    DubiousTrackConfig& config();

    void updateActive();
};

}

#endif // EVALUATIONREQUIREMENTDUBIOUSTRACKCONFIGWIDGET_H
