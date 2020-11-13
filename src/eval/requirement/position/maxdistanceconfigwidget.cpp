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

#include "eval/requirement/position/maxdistanceconfigwidget.h"
#include "eval/requirement/position/maxdistanceconfig.h"
#include "textfielddoublevalidator.h"
#include "logger.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>

namespace EvaluationRequirement
{

    PositionMaxDistanceConfigWidget::PositionMaxDistanceConfigWidget(PositionMaxDistanceConfig& config)
        : QWidget(), config_(config)
    {
        form_layout_ = new QFormLayout();

        config_.addGUIElements(form_layout_);

        // max ref time diff
        max_ref_time_diff_edit_ = new QLineEdit(QString::number(config_.maxRefTimeDiff()));
        max_ref_time_diff_edit_->setValidator(new QDoubleValidator(0.0, 30.0, 2, this));
        connect(max_ref_time_diff_edit_, &QLineEdit::textEdited,
                this, &PositionMaxDistanceConfigWidget::maxRefTimeDiffEditSlot);

        form_layout_->addRow("Maximum Reference Time Difference [s]", max_ref_time_diff_edit_);


        // max dist
        max_distance_edit_ = new QLineEdit(QString::number(config_.maxDistance()));
        max_distance_edit_->setValidator(new QDoubleValidator(0.0, 10000.0, 2, this));
        connect(max_distance_edit_, &QLineEdit::textEdited,
                this, &PositionMaxDistanceConfigWidget::maxDistanceEditSlot);

        form_layout_->addRow("Maximum Distance [m]", max_distance_edit_);

        // prob
        minimum_prob_edit_ = new QLineEdit(QString::number(config_.minimumProbability()));
        minimum_prob_edit_->setValidator(new QDoubleValidator(0.0001, 1.0, 4, this));
        connect(minimum_prob_edit_, &QLineEdit::textEdited,
                this, &PositionMaxDistanceConfigWidget::minimumProbEditSlot);

        form_layout_->addRow("Minimum Probability [1]", minimum_prob_edit_);

        setLayout(form_layout_);
    }

    void PositionMaxDistanceConfigWidget::maxRefTimeDiffEditSlot(QString value)
    {
        loginf << "PositionMaxDistanceConfigWidget: maxRefTimeDiffEditSlot: value " << value.toStdString();

        bool ok;
        float val = value.toFloat(&ok);

        if (ok)
            config_.maxRefTimeDiff(val);
        else
            loginf << "PositionMaxDistanceConfigWidget: maxRefTimeDiffEditSlot: invalid value";
    }


    void PositionMaxDistanceConfigWidget::maxDistanceEditSlot(QString value)
    {
        loginf << "PositionMaxDistanceConfigWidget: maxDistanceEditSlot: value " << value.toStdString();

        bool ok;
        float val = value.toFloat(&ok);

        if (ok)
            config_.maxDistance(val);
        else
            loginf << "PositionMaxDistanceConfigWidget: maxDistanceEditSlot: invalid value";
    }

    void PositionMaxDistanceConfigWidget::PositionMaxDistanceConfigWidget::minimumProbEditSlot(QString value)
    {
        loginf << "PositionMaxDistanceConfigWidget: maximumProbEditSlot: value " << value.toStdString();

        bool ok;
        float val = value.toFloat(&ok);

        if (ok)
            config_.minimumProbability(val);
        else
            loginf << "PositionMaxDistanceConfigWidget: maximumProbEditSlot: invalid value";
    }

}
