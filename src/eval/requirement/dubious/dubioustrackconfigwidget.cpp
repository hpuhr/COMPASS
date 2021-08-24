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

#include "eval/requirement/dubious/dubioustrackconfigwidget.h"
#include "eval/requirement/dubious/dubioustrackconfig.h"
#include "textfielddoublevalidator.h"
#include "logger.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>

namespace EvaluationRequirement
{

DubiousTrackConfigWidget::DubiousTrackConfigWidget(DubiousTrackConfig& cfg)
    : BaseConfigWidget(cfg)
{
    // mark primary-only tracks
    mark_primary_only_check_ = new QCheckBox ();
    mark_primary_only_check_->setChecked(config().markPrimaryOnly());
    connect(mark_primary_only_check_, &QCheckBox::clicked,
            this, &DubiousTrackConfigWidget::toggleMarkPrimaryOnlySlot);

    form_layout_->addRow("Mark Primary-Only", mark_primary_only_check_);


    // use min updates
    use_min_updates_check_ = new QCheckBox ();
    use_min_updates_check_->setChecked(config().useMinUpdates());
    connect(use_min_updates_check_, &QCheckBox::clicked,
            this, &DubiousTrackConfigWidget::toggleUseMinUpdatesSlot);

    form_layout_->addRow("Use Minimum Updates", use_min_updates_check_);

    // min updates
    min_updates_edit_ = new QLineEdit(QString::number(config().minUpdates()));
    min_updates_edit_->setValidator(new QDoubleValidator(0, 100, 0, this));
    connect(min_updates_edit_, &QLineEdit::textEdited,
            this, &DubiousTrackConfigWidget::minUpdatesEditSlot);

    form_layout_->addRow("Minimum Updates [1]", min_updates_edit_);


    // use min duration
    use_min_duration_check_ = new QCheckBox ();
    use_min_duration_check_->setChecked(config().useMinDuration());
    connect(use_min_duration_check_, &QCheckBox::clicked,
            this, &DubiousTrackConfigWidget::toggleUseMinDurationSlot);

    form_layout_->addRow("Use Minimum Duration", use_min_duration_check_);

    // min duration
    min_duration_edit_ = new QLineEdit(QString::number(config().minDuration()));
    min_duration_edit_->setValidator(new QDoubleValidator(0, 100, 0, this));
    connect(min_duration_edit_, &QLineEdit::textEdited,
            this, &DubiousTrackConfigWidget::minDurationEditSlot);

    form_layout_->addRow("Minimum Duration [1]", min_duration_edit_);
}


void DubiousTrackConfigWidget::toggleMarkPrimaryOnlySlot()
{
    loginf << "DubiousTrackConfigWidget: toggleMarkPrimaryOnlySlot";

    assert (mark_primary_only_check_);
    config().markPrimaryOnly(mark_primary_only_check_->checkState() == Qt::Checked);

    updateActive();
}

void DubiousTrackConfigWidget::toggleUseMinUpdatesSlot()
{
    loginf << "DubiousTrackConfigWidget: toggleUseMinUpdatesSlot";

    assert (use_min_updates_check_);
    config().useMinUpdates(use_min_updates_check_->checkState() == Qt::Checked);

    updateActive();
}

void DubiousTrackConfigWidget::minUpdatesEditSlot(QString value)
{
    loginf << "DubiousTrackConfigWidget: minUpdatesEditSlot: value " << value.toStdString();

    bool ok;
    unsigned int val = value.toUInt(&ok);

    if (ok)
        config().minUpdates(val);
    else
        loginf << "DubiousTrackConfigWidget: thresholdPercentEditSlot: invalid value";
}

void DubiousTrackConfigWidget::toggleUseMinDurationSlot()
{
    loginf << "DubiousTrackConfigWidget: toggleUseMinDurationSlot";

    assert (use_min_duration_check_);
    config().useMinDuration(use_min_duration_check_->checkState() == Qt::Checked);

    updateActive();
}

void DubiousTrackConfigWidget::minDurationEditSlot(QString value)
{
    loginf << "DubiousTrackConfigWidget: minDurationEditSlot: value " << value.toStdString();

    bool ok;
    float val = value.toFloat(&ok);

    if (ok)
        config().minDuration(val);
    else
        loginf << "DubiousTrackConfigWidget: minDurationEditSlot: invalid value";
}

DubiousTrackConfig& DubiousTrackConfigWidget::config()
{
    DubiousTrackConfig* config = dynamic_cast<DubiousTrackConfig*>(&config_);
    assert (config);

    return *config;
}

void DubiousTrackConfigWidget::updateActive()
{
    assert (min_updates_edit_);
    min_updates_edit_->setEnabled(config().useMinUpdates());

    assert (min_duration_edit_);
    min_duration_edit_->setEnabled(config().useMinDuration());
}


}
