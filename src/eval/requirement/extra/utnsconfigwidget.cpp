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

#include "eval/requirement/extra/utnsconfigwidget.h"
#include "eval/requirement/extra/utnsconfig.h"
#include "textfielddoublevalidator.h"
#include "logger.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>

using namespace std;

namespace EvaluationRequirement
{

ExtraDataConfigWidget::ExtraDataConfigWidget(ExtraDataConfig& config)
    : QWidget(), config_(config)
{
    form_layout_ = new QFormLayout();

    config_.addGUIElements(form_layout_);

    // min_duration
    min_duration_edit_ = new QLineEdit(QString::number(config_.minDuration()));
    min_duration_edit_->setValidator(new QDoubleValidator(0.1, 3000.0, 1, this));
    connect(min_duration_edit_, &QLineEdit::textEdited,
            this, &ExtraDataConfigWidget::minDurationEditSlot);

    form_layout_->addRow("Minimum Duration [s]", min_duration_edit_);

    // min_num_updates
    min_num_updates_edit_ = new QLineEdit(QString::number(config_.minNumUpdates()));
    min_num_updates_edit_->setValidator(new QDoubleValidator(0, 300, 0, this));
    connect(min_num_updates_edit_, &QLineEdit::textEdited,
            this, &ExtraDataConfigWidget::minNumUpdatesEditSlot);

    form_layout_->addRow("Minimum Number of Updates [1]", min_num_updates_edit_);

    // ignore_primary_only
    ignore_primary_only_check_ = new QCheckBox ();
    ignore_primary_only_check_->setChecked(config_.ignorePrimaryOnly());
    connect(ignore_primary_only_check_, &QCheckBox::clicked,
            this, &ExtraDataConfigWidget::toggleIgnorePrimaryOnlySlot);

    form_layout_->addRow("Ignore Primary Only", ignore_primary_only_check_);

    // prob
    maximum_probability_edit_ = new QLineEdit(QString::number(config_.maximumProbability()));
    maximum_probability_edit_->setValidator(new QDoubleValidator(0.0001, 1.0, 4, this));
    connect(maximum_probability_edit_, &QLineEdit::textEdited,
            this, &ExtraDataConfigWidget::maximumProbEditSlot);

    form_layout_->addRow("Maximum Probability [0-1]", maximum_probability_edit_);

    setLayout(form_layout_);
}

void ExtraDataConfigWidget::minDurationEditSlot(QString value)
{
    loginf << "ExtraDataConfigWidget: minDurationEditSlot: value " << value.toStdString();

    bool ok;
    float val = value.toFloat(&ok);

    if (ok)
        config_.minDuration(val);
    else
        loginf << "ExtraDataConfigWidget: minDurationEditSlot: invalid value";
}

void ExtraDataConfigWidget::minNumUpdatesEditSlot(QString value)
{
    loginf << "ExtraDataConfigWidget: minNumUpdatesEditSlot: value " << value.toStdString();

    bool ok;
    unsigned int val = value.toUInt(&ok);

    if (ok)
        config_.minNumUpdates(val);
    else
        loginf << "ExtraDataConfigWidget: minNumUpdatesEditSlot: invalid value";
}

void ExtraDataConfigWidget::toggleIgnorePrimaryOnlySlot()
{
    loginf << "ExtraDataConfigWidget: toggleIgnorePrimaryOnlySlot";

    assert (ignore_primary_only_check_);
    config_.ignorePrimaryOnly(ignore_primary_only_check_->checkState() == Qt::Checked);
}

void ExtraDataConfigWidget::maximumProbEditSlot(QString value)
{
    loginf << "ExtraDataConfigWidget: maximumProbEditSlot: value " << value.toStdString();

    bool ok;
    float val = value.toFloat(&ok);

    if (ok)
        config_.maximumProbability(val);
    else
        loginf << "ExtraDataConfigWidget: maximumProbEditSlot: invalid value";
}

}
