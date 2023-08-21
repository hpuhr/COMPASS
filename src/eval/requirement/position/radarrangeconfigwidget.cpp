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

#include "eval/requirement/position/radarrangeconfigwidget.h"
#include "eval/requirement/position/radarrangeconfig.h"
#include "textfielddoublevalidator.h"
#include "eval/requirement/base/comparisontypecombobox.h"
#include "logger.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>

namespace EvaluationRequirement
{

PositionRangeConfigWidget::PositionRangeConfigWidget(PositionRangeConfig& cfg)
    : BaseConfigWidget(cfg)
{
    // max dist
    threshold_value_edit_ = new QLineEdit(QString::number(config().thresholdValue()));
    threshold_value_edit_->setValidator(new QDoubleValidator(0.0, 10000.0, 2, this));
    threshold_value_edit_->setToolTip("Minimum/Maximum allowed distance from test target report to reference");
    connect(threshold_value_edit_, &QLineEdit::textEdited,
            this, &PositionRangeConfigWidget::thresholdValueEditSlot);

    form_layout_->addRow("Threshold Value [m]", threshold_value_edit_);

}

void PositionRangeConfigWidget::thresholdValueEditSlot(QString value)
{
    loginf << "PositionRangeConfigWidget: thresholdValueEditSlot: value " << value.toStdString();

    bool ok;
    float val = value.toFloat(&ok);

    if (ok)
        config().thresholdValue(val);
    else
        loginf << "PositionRangeConfigWidget: thresholdValueEditSlot: invalid value";
}

PositionRangeConfig& PositionRangeConfigWidget::config()
{
    PositionRangeConfig* config = dynamic_cast<PositionRangeConfig*>(&config_);
    assert (config);

    return *config;
}

}
