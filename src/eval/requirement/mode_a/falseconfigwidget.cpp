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

#include "eval/requirement/mode_a/falseconfigwidget.h"
#include "eval/requirement/mode_a/falseconfig.h"
#include "textfielddoublevalidator.h"
#include "logger.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>

using namespace std;

namespace EvaluationRequirement
{

    ModeAFalseConfigWidget::ModeAFalseConfigWidget(ModeAFalseConfig& cfg)
            : BaseConfigWidget(cfg)
    {
//        form_layout_ = new QFormLayout();

//        config_.addGUIElements(form_layout_);

        // false prob
        max_prob_false_edit_ = new QLineEdit(QString::number(config().maximumProbabilityFalse()));
        max_prob_false_edit_->setValidator(new QDoubleValidator(0.0001, 1.0, 4, this));
        connect(max_prob_false_edit_, &QLineEdit::textEdited,
                this, &ModeAFalseConfigWidget::maxProbFalseEditSlot);

        form_layout_->addRow("False Maximum Probability [1]", max_prob_false_edit_);

        //setLayout(form_layout_);
    }

    void ModeAFalseConfigWidget::maxProbFalseEditSlot(QString value)
    {
        loginf << "EvaluationRequirementModeAConfigWidget: maxProbFalseEditSlot: value " << value.toStdString();

        bool ok;
        float val = value.toFloat(&ok);

        if (ok)
            config().maximumProbabilityFalse(val);
        else
            loginf << "EvaluationRequirementModeAConfigWidget: maxProbFalseEditSlot: invalid value";
    }

    ModeAFalseConfig& ModeAFalseConfigWidget::config()
    {
        ModeAFalseConfig* config = dynamic_cast<ModeAFalseConfig*>(&config_);
        assert (config);

        return *config;
    }
}
