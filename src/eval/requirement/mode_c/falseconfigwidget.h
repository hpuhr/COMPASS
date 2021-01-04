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

#include <QWidget>

class QLineEdit;
class QCheckBox;

class QFormLayout;

namespace EvaluationRequirement
{
    class ModeCFalseConfig;

    class ModeCFalseConfigWidget : public QWidget
    {
        Q_OBJECT

    public slots:
        void maxRefTimeDiffEditSlot(QString value);

        void toogleUseMinProbPresentSlot ();
        void minProbPresentEditSlot(QString value);

        void toogleUseMaxProbFalseSlot ();
        void maxProbFalseEditSlot(QString value);

        void maxDiffEditSlot(QString value);

    public:
        ModeCFalseConfigWidget(ModeCFalseConfig& config);

    protected:
        ModeCFalseConfig& config_;

        QFormLayout* form_layout_ {nullptr};

        QLineEdit* max_ref_time_diff_edit_{nullptr};

        QCheckBox* min_prob_pres_check_{nullptr};
        QLineEdit* min_prob_pres_edit_{nullptr};

        QCheckBox* max_prob_false_check_{nullptr};
        QLineEdit* max_prob_false_edit_{nullptr};

        QLineEdit* max_diff_edit_{nullptr};
    };

}

#endif // EVALUATIONREQUIREMENTMODECFALSECONFIGWIDGET_H
