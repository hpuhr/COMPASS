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

#include "eval/requirement/mode_c/modecconfigwidget.h"
#include "eval/requirement/mode_c/modecconfig.h"
#include "textfielddoublevalidator.h"
#include "logger.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>

using namespace std;

namespace EvaluationRequirement
{

    ModeCConfigWidget::ModeCConfigWidget(ModeCConfig& config)
            : QWidget(), config_(config)
    {
        form_layout_ = new QFormLayout();

        config_.addGUIElements(form_layout_);

        // max ref time diff
        max_ref_time_diff_edit_ = new QLineEdit(QString::number(config_.maxRefTimeDiff()));
        max_ref_time_diff_edit_->setValidator(new QDoubleValidator(0.0, 30.0, 2, this));
        connect(max_ref_time_diff_edit_, &QLineEdit::textEdited,
                this, &ModeCConfigWidget::maxRefTimeDiffEditSlot);

        form_layout_->addRow("Maximum Reference Time Difference [s]", max_ref_time_diff_edit_);

        // prob
        min_prob_pres_check_ = new QCheckBox();
        min_prob_pres_check_->setChecked(config_.useMinimumProbabilityPresent());
        connect(min_prob_pres_check_, &QCheckBox::clicked,
                this, &ModeCConfigWidget::toogleUseMinProbPresentSlot);
        form_layout_->addRow("Use Present Minimum Probability", min_prob_pres_check_);

        min_prob_pres_edit_ = new QLineEdit(QString::number(config_.minimumProbabilityPresent()));
        min_prob_pres_edit_->setValidator(new QDoubleValidator(0.0001, 1.0, 4, this));
        connect(min_prob_pres_edit_, &QLineEdit::textEdited,
                this, &ModeCConfigWidget::minProbPresentEditSlot);

        form_layout_->addRow("Present Minimum Probability [1]", min_prob_pres_edit_);

        // false prob
        max_prob_false_check_ = new QCheckBox();
        max_prob_false_check_->setChecked(config_.useMaximumProbabilityFalse());
        connect(max_prob_false_check_, &QCheckBox::clicked,
                this, &ModeCConfigWidget::toogleUseMaxProbFalseSlot);
        form_layout_->addRow("Use False Maximum Probability", max_prob_false_check_);

        max_prob_false_edit_ = new QLineEdit(QString::number(config_.maximumProbabilityFalse()));
        max_prob_false_edit_->setValidator(new QDoubleValidator(0.0001, 1.0, 4, this));
        connect(max_prob_false_edit_, &QLineEdit::textEdited,
                this, &ModeCConfigWidget::maxProbFalseEditSlot);

        form_layout_->addRow("False Maximum Probability [1]", max_prob_false_edit_);

        setLayout(form_layout_);
    }

    void ModeCConfigWidget::maxRefTimeDiffEditSlot(QString value)
    {
        loginf << "ModeCConfigWidget: maxRefTimeDiffEditSlot: value " << value.toStdString();

        bool ok;
        float val = value.toFloat(&ok);

        if (ok)
            config_.maxRefTimeDiff(val);
        else
            loginf << "ModeCConfigWidget: maxRefTimeDiffEditSlot: invalid value";
    }


    void ModeCConfigWidget::toogleUseMinProbPresentSlot ()
    {
        assert (min_prob_pres_check_);
        config_.useMinimumProbabilityPresent(min_prob_pres_check_->checkState() == Qt::Checked);
    }


    void ModeCConfigWidget::minProbPresentEditSlot(QString value)
    {
        loginf << "EvaluationRequirementModeCConfigWidget: minProbPresentEditSlot: value " << value.toStdString();

        bool ok;
        float val = value.toFloat(&ok);

        if (ok)
            config_.minimumProbabilityPresent(val);
        else
            loginf << "EvaluationRequirementModeCConfigWidget: minProbPresentEditSlot: invalid value";
    }

    void ModeCConfigWidget::toogleUseMaxProbFalseSlot ()
    {
        assert (max_prob_false_check_);
        config_.useMaximumProbabilityFalse(max_prob_false_check_->checkState() == Qt::Checked);
    }


    void ModeCConfigWidget::maxProbFalseEditSlot(QString value)
    {
        loginf << "EvaluationRequirementModeCConfigWidget: maxProbFalseEditSlot: value " << value.toStdString();

        bool ok;
        float val = value.toFloat(&ok);

        if (ok)
            config_.maximumProbabilityFalse(val);
        else
            loginf << "EvaluationRequirementModeCConfigWidget: maxProbFalseEditSlot: invalid value";
    }

}
