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

#include "eval/requirement/mode_a/modeaconfigwidget.h"
#include "eval/requirement/mode_a/modeaconfig.h"
#include "textfielddoublevalidator.h"
#include "logger.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>

using namespace std;

namespace EvaluationRequirement
{

    ModeAConfigWidget::ModeAConfigWidget(ModeAConfig& config)
            : QWidget(), config_(config)
    {
        form_layout_ = new QFormLayout();

        config_.addGUIElements(form_layout_);

        // max ref time diff
        max_ref_time_diff_edit_ = new QLineEdit(QString::number(config_.maxRefTimeDiff()));
        max_ref_time_diff_edit_->setValidator(new QDoubleValidator(0.0, 30.0, 2, this));
        connect(max_ref_time_diff_edit_, &QLineEdit::textEdited,
                this, &ModeAConfigWidget::maxRefTimeDiffEditSlot);

        form_layout_->addRow("Maximum Reference Time Difference [s]", max_ref_time_diff_edit_);

        // prob
        minimum_prob_exist_check_ = new QCheckBox();
        minimum_prob_exist_check_->setChecked(config_.useMinimumProbabilityExisting());
        connect(minimum_prob_exist_check_, &QCheckBox::clicked,
                this, &ModeAConfigWidget::toogleUseMinimumProbExistingSlot);
        form_layout_->addRow("Use Minimum Probability Existing", minimum_prob_exist_check_);

        minimum_prob_exist_edit_ = new QLineEdit(QString::number(config_.minimumProbabilityExisting()));
        minimum_prob_exist_edit_->setValidator(new QDoubleValidator(0.0001, 1.0, 4, this));
        connect(minimum_prob_exist_edit_, &QLineEdit::textEdited,
                this, &ModeAConfigWidget::minimumProbExistingEditSlot);

        form_layout_->addRow("Minimum Probability Existing [1]", minimum_prob_exist_edit_);

        // false prob
        maximum_prob_false_check_ = new QCheckBox();
        maximum_prob_false_check_->setChecked(config_.useMaximumProbabilityFalse());
        connect(maximum_prob_false_check_, &QCheckBox::clicked,
                this, &ModeAConfigWidget::toogleUseMaximumProbFalseSlot);
        form_layout_->addRow("Use Maximum Probability False", maximum_prob_false_check_);

        maximum_prob_false_edit_ = new QLineEdit(QString::number(config_.maximumProbabilityFalse()));
        maximum_prob_false_edit_->setValidator(new QDoubleValidator(0.0001, 1.0, 4, this));
        connect(maximum_prob_false_edit_, &QLineEdit::textEdited,
                this, &ModeAConfigWidget::maximumProbFalseEditSlot);

        form_layout_->addRow("Maximum Probability False [1]", maximum_prob_false_edit_);

        setLayout(form_layout_);
    }

    void ModeAConfigWidget::maxRefTimeDiffEditSlot(QString value)
    {
        loginf << "ModeAConfigWidget: maxRefTimeDiffEditSlot: value " << value.toStdString();

        bool ok;
        float val = value.toFloat(&ok);

        if (ok)
            config_.maxRefTimeDiff(val);
        else
            loginf << "ModeAConfigWidget: maxRefTimeDiffEditSlot: invalid value";
    }


    void ModeAConfigWidget::toogleUseMinimumProbExistingSlot ()
    {
        assert (minimum_prob_exist_check_);
        config_.useMinimumProbabilityExisting(minimum_prob_exist_check_->checkState() == Qt::Checked);
    }


    void ModeAConfigWidget::minimumProbExistingEditSlot(QString value)
    {
        loginf << "EvaluationRequirementModeAConfigWidget: minimumProbExistingEditSlot: value " << value.toStdString();

        bool ok;
        float val = value.toFloat(&ok);

        if (ok)
            config_.minimumProbabilityExisting(val);
        else
            loginf << "EvaluationRequirementModeAConfigWidget: minimumProbExistingEditSlot: invalid value";
    }

    void ModeAConfigWidget::toogleUseMaximumProbFalseSlot ()
    {
        assert (maximum_prob_false_check_);
        config_.useMaximumProbabilityFalse(maximum_prob_false_check_->checkState() == Qt::Checked);
    }


    void ModeAConfigWidget::maximumProbFalseEditSlot(QString value)
    {
        loginf << "EvaluationRequirementModeAConfigWidget: maximumProbFalseEditSlot: value " << value.toStdString();

        bool ok;
        float val = value.toFloat(&ok);

        if (ok)
            config_.maximumProbabilityFalse(val);
        else
            loginf << "EvaluationRequirementModeAConfigWidget: maximumProbFalseEditSlot: invalid value";
    }

}
