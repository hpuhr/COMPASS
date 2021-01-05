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

#include "eval/requirement/identification/identificationconfigwidget.h"
#include "eval/requirement/identification/identificationconfig.h"
#include "textfielddoublevalidator.h"
#include "logger.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>

using namespace std;

namespace EvaluationRequirement
{

    IdentificationConfigWidget::IdentificationConfigWidget(IdentificationConfig& cfg)
        : BaseConfigWidget(cfg)
    {
//        form_layout_ = new QFormLayout();

//        config_.addGUIElements(form_layout_);

        // prob
        minimum_prob_edit_ = new QLineEdit(QString::number(config().minimumProbability()));
        minimum_prob_edit_->setValidator(new QDoubleValidator(0.0001, 1.0, 4, this));
        connect(minimum_prob_edit_, &QLineEdit::textEdited,
                this, &IdentificationConfigWidget::minimumProbEditSlot);

        form_layout_->addRow("Minimum Probability [1]", minimum_prob_edit_);

        //setLayout(form_layout_);
    }

    void IdentificationConfigWidget::minimumProbEditSlot(QString value)
    {
        loginf << "EvaluationRequirementIdentificationConfigWidget: minimumProbEditSlot: value " << value.toStdString();

        bool ok;
        float val = value.toFloat(&ok);

        if (ok)
            config().minimumProbability(val);
        else
            loginf << "EvaluationRequirementIdentificationConfigWidget: minimumProbEditSlot: invalid value";
    }

    IdentificationConfig& IdentificationConfigWidget::config()
    {
        IdentificationConfig* config = dynamic_cast<IdentificationConfig*>(&config_);
        assert (config);

        return *config;
    }

}
