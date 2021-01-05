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

#include "eval/requirement/position/latencyconfigwidget.h"
#include "eval/requirement/position/latencyconfig.h"
#include "textfielddoublevalidator.h"
#include "logger.h"
#include "stringconv.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>

using namespace Utils;

namespace EvaluationRequirement
{

PositionLatencyConfigWidget::PositionLatencyConfigWidget(PositionLatencyConfig& cfg)
    : BaseConfigWidget(cfg)
{
//    form_layout_ = new QFormLayout();

//    config_.addGUIElements(form_layout_);

    // max dist
    max_abs_value_edit_ = new QLineEdit(String::timeStringFromDouble(config().maxAbsValue()).c_str());
    //max_abs_value_edit_->setValidator(new QDoubleValidator(0.0, 10000.0, 2, this));
    connect(max_abs_value_edit_, &QLineEdit::textEdited,
            this, &PositionLatencyConfigWidget::maxAbsValueEditSlot);

    form_layout_->addRow("Maximum Absolute Value [s]", max_abs_value_edit_);

    // prob
    minimum_prob_edit_ = new QLineEdit(QString::number(config().minimumProbability()));
    minimum_prob_edit_->setValidator(new QDoubleValidator(0.0001, 1.0, 4, this));
    connect(minimum_prob_edit_, &QLineEdit::textEdited,
            this, &PositionLatencyConfigWidget::minimumProbEditSlot);

    form_layout_->addRow("Minimum Probability [1]", minimum_prob_edit_);

    //setLayout(form_layout_);
}

void PositionLatencyConfigWidget::maxAbsValueEditSlot(QString value)
{
    loginf << "PositionLatencyConfigWidget: maxAbsValueEditSlot: value " << value.toStdString();

    bool ok;
    float val = String::timeFromString(value.toStdString(), &ok);

    if (ok)
    {
        config().maxAbsValue(val);
    }
    else
        loginf << "PositionLatencyConfigWidget: maxAbsValueEditSlot: invalid value";
}

void PositionLatencyConfigWidget::PositionLatencyConfigWidget::minimumProbEditSlot(QString value)
{
    loginf << "PositionLatencyConfigWidget: maximumProbEditSlot: value " << value.toStdString();

    bool ok;
    float val = value.toFloat(&ok);

    if (ok)
        config().minimumProbability(val);
    else
        loginf << "PositionLatencyConfigWidget: maximumProbEditSlot: invalid value";
}

PositionLatencyConfig& PositionLatencyConfigWidget::config()
{
    PositionLatencyConfig* config = dynamic_cast<PositionLatencyConfig*>(&config_);
    assert (config);

    return *config;
}

}
