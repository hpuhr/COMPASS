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

#ifndef EVALUATIONREQUIREMENTEXTRADATACONFIGWIDGET_H
#define EVALUATIONREQUIREMENTEXTRADATACONFIGWIDGET_H

#include "eval/requirement/base/baseconfigwidget.h"

class QLineEdit;
class QCheckBox;

class QFormLayout;

namespace EvaluationRequirement
{
    class ExtraDataConfig;

    class ExtraDataConfigWidget : public BaseConfigWidget
    {
        Q_OBJECT

    public slots:
        void minDurationEditSlot(QString value);
        void minNumUpdatesEditSlot(QString value);
        void toggleIgnorePrimaryOnlySlot();
        void maximumProbEditSlot(QString value);

    public:
        ExtraDataConfigWidget(ExtraDataConfig& cfg);

    protected:
        QLineEdit* min_duration_edit_{nullptr};
        QLineEdit* min_num_updates_edit_{nullptr};
        QCheckBox* ignore_primary_only_check_{nullptr};
        QLineEdit* maximum_probability_edit_{nullptr};

        ExtraDataConfig& config();
    };

}

#endif // EVALUATIONREQUIREMENTEXTRADATACONFIGWIDGET_H
