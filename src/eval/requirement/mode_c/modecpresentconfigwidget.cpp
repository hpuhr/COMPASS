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

#include "eval/requirement/mode_c/modecpresentconfigwidget.h"
#include "eval/requirement/mode_c/presentconfig.h"
#include "textfielddoublevalidator.h"
#include "logger.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>

using namespace std;

namespace EvaluationRequirement
{

ModeCPresentConfigWidget::ModeCPresentConfigWidget(ModeCPresentConfig& cfg)
    : BaseConfigWidget(cfg)
{
    // prob
    min_prob_pres_edit_ = new QLineEdit(QString::number(config().minimumProbabilityPresent()));
    min_prob_pres_edit_->setValidator(new QDoubleValidator(0.0001, 1.0, 4, this));
//    connect(min_prob_pres_edit_, &QLineEdit::textEdited,
//            this, &ModeCPresentConfigWidget::minProbPresentEditSlot);

    form_layout_->addRow("Present Minimum Probability [1]", min_prob_pres_edit_);
}

void ModeCPresentConfigWidget::minProbPresentEditSlot(QString value)
{
    loginf << "EvaluationRequirementModeCConfigWidget: minProbPresentEditSlot: value " << value.toStdString();

    bool ok;
    float val = value.toFloat(&ok);

    if (ok)
        config().minimumProbabilityPresent(val);
    else
        loginf << "EvaluationRequirementModeCConfigWidget: minProbPresentEditSlot: invalid value";
}

ModeCPresentConfig& ModeCPresentConfigWidget::config()
{
    ModeCPresentConfig* config = dynamic_cast<ModeCPresentConfig*>(&config_);
    assert (config);

    return *config;
}

}
