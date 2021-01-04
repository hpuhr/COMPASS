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

#include "eval/requirement/mode_a/presentconfigwidget.h"
#include "eval/requirement/mode_a/presentconfig.h"
#include "textfielddoublevalidator.h"
#include "logger.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>

using namespace std;

namespace EvaluationRequirement
{

    ModeAPresentConfigWidget::ModeAPresentConfigWidget(ModeAPresentConfig& config)
            : QWidget(), config_(config)
    {
        form_layout_ = new QFormLayout();

        config_.addGUIElements(form_layout_);

        // max ref time diff
        max_ref_time_diff_edit_ = new QLineEdit(QString::number(config_.maxRefTimeDiff()));
        max_ref_time_diff_edit_->setValidator(new QDoubleValidator(0.0, 30.0, 2, this));
        connect(max_ref_time_diff_edit_, &QLineEdit::textEdited,
                this, &ModeAPresentConfigWidget::maxRefTimeDiffEditSlot);

        form_layout_->addRow("Maximum Reference Time Difference [s]", max_ref_time_diff_edit_);

        // prob
        min_prob_pres_check_ = new QCheckBox();
        min_prob_pres_check_->setChecked(config_.useMinimumProbabilityPresent());
        connect(min_prob_pres_check_, &QCheckBox::clicked,
                this, &ModeAPresentConfigWidget::toogleUseMinProbPresentSlot);
        form_layout_->addRow("Use Present Minimum Probability", min_prob_pres_check_);

        min_prob_pres_edit_ = new QLineEdit(QString::number(config_.minimumProbabilityPresent()));
        min_prob_pres_edit_->setValidator(new QDoubleValidator(0.0001, 1.0, 4, this));
        connect(min_prob_pres_edit_, &QLineEdit::textEdited,
                this, &ModeAPresentConfigWidget::minProbPresentEditSlot);

        form_layout_->addRow("Present Minimum Probability [1]", min_prob_pres_edit_);

        // false prob
        max_prob_false_check_ = new QCheckBox();
        max_prob_false_check_->setChecked(config_.useMaximumProbabilityFalse());
        connect(max_prob_false_check_, &QCheckBox::clicked,
                this, &ModeAPresentConfigWidget::toogleUseMaxProbFalseSlot);
        form_layout_->addRow("Use False Maximum Probability", max_prob_false_check_);

        max_prob_false_edit_ = new QLineEdit(QString::number(config_.maximumProbabilityFalse()));
        max_prob_false_edit_->setValidator(new QDoubleValidator(0.0001, 1.0, 4, this));
        connect(max_prob_false_edit_, &QLineEdit::textEdited,
                this, &ModeAPresentConfigWidget::maxProbFalseEditSlot);

        form_layout_->addRow("False Maximum Probability [1]", max_prob_false_edit_);

        setLayout(form_layout_);
    }

    void ModeAPresentConfigWidget::maxRefTimeDiffEditSlot(QString value)
    {
        loginf << "ModeAConfigWidget: maxRefTimeDiffEditSlot: value " << value.toStdString();

        bool ok;
        float val = value.toFloat(&ok);

        if (ok)
            config_.maxRefTimeDiff(val);
        else
            loginf << "ModeAConfigWidget: maxRefTimeDiffEditSlot: invalid value";
    }


    void ModeAPresentConfigWidget::toogleUseMinProbPresentSlot ()
    {
        assert (min_prob_pres_check_);
        config_.useMinimumProbabilityPresent(min_prob_pres_check_->checkState() == Qt::Checked);
    }


    void ModeAPresentConfigWidget::minProbPresentEditSlot(QString value)
    {
        loginf << "EvaluationRequirementModeAConfigWidget: minProbPresentEditSlot: value " << value.toStdString();

        bool ok;
        float val = value.toFloat(&ok);

        if (ok)
            config_.minimumProbabilityPresent(val);
        else
            loginf << "EvaluationRequirementModeAConfigWidget: minProbPresentEditSlot: invalid value";
    }

    void ModeAPresentConfigWidget::toogleUseMaxProbFalseSlot ()
    {
        assert (max_prob_false_check_);
        config_.useMaximumProbabilityFalse(max_prob_false_check_->checkState() == Qt::Checked);
    }


    void ModeAPresentConfigWidget::maxProbFalseEditSlot(QString value)
    {
        loginf << "EvaluationRequirementModeAConfigWidget: maxProbFalseEditSlot: value " << value.toStdString();

        bool ok;
        float val = value.toFloat(&ok);

        if (ok)
            config_.maximumProbabilityFalse(val);
        else
            loginf << "EvaluationRequirementModeAConfigWidget: maxProbFalseEditSlot: invalid value";
    }

}
