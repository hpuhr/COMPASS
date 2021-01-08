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

#include "eval/requirement/detection/detectionconfigwidget.h"
#include "eval/requirement/detection/detectionconfig.h"
#include "textfielddoublevalidator.h"
#include "logger.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>

using namespace std;

namespace EvaluationRequirement
{

DetectionConfigWidget::DetectionConfigWidget(DetectionConfig& cfg)
    : BaseConfigWidget(cfg)
{
    // ui
    update_interval_edit_ = new QLineEdit(QString::number(config().updateInterval()));
    update_interval_edit_->setValidator(new QDoubleValidator(0.1, 30.0, 2, this));
    connect(update_interval_edit_, &QLineEdit::textEdited,
            this, &DetectionConfigWidget::updateIntervalEditSlot);

    form_layout_->addRow("Update Interval [s]", update_interval_edit_);

    // min gap
    use_min_gap_length_check_ = new QCheckBox ();
    use_min_gap_length_check_->setChecked(config().useMinGapLength());
    connect(use_min_gap_length_check_, &QCheckBox::clicked,
            this, &DetectionConfigWidget::toggleUseMinGapLengthSlot);

    form_layout_->addRow("Use Minimum Gap Length", use_min_gap_length_check_);

    min_gap_length_edit_ = new QLineEdit(QString::number(config().minGapLength()));
    min_gap_length_edit_->setValidator(new QDoubleValidator(0.01, 300.0, 3, this));
    connect(min_gap_length_edit_, &QLineEdit::textEdited,
            this, &DetectionConfigWidget::minGapLengthEditSlot);

    form_layout_->addRow("Minimum Gap Length[s]", min_gap_length_edit_);

    // max gap
    use_max_gap_length_check_ = new QCheckBox ();
    use_max_gap_length_check_->setChecked(config().useMaxGapLength());
    connect(use_max_gap_length_check_, &QCheckBox::clicked,
            this, &DetectionConfigWidget::toggleUseMaxGapLengthSlot);

    form_layout_->addRow("Use Maximum Gap Length", use_max_gap_length_check_);

    max_gap_length_edit_ = new QLineEdit(QString::number(config().maxGapLength()));
    max_gap_length_edit_->setValidator(new QDoubleValidator(0.01, 300.0, 3, this));
    connect(max_gap_length_edit_, &QLineEdit::textEdited,
            this, &DetectionConfigWidget::maxGapLengthEditSlot);

    form_layout_->addRow("Maximum Gap Length[s]", max_gap_length_edit_);

    // miss tolerance
    use_miss_tolerance_check_ = new QCheckBox ();
    use_miss_tolerance_check_->setChecked(config().useMissTolerance());
    connect(use_miss_tolerance_check_, &QCheckBox::clicked,
            this, &DetectionConfigWidget::toggleUseMissToleranceSlot);

    form_layout_->addRow("Use Miss Tolerance", use_miss_tolerance_check_);

    miss_tolerance_edit_ = new QLineEdit(QString::number(config().missTolerance()));
    miss_tolerance_edit_->setValidator(new QDoubleValidator(0.01, 1.0, 3, this));
    connect(miss_tolerance_edit_, &QLineEdit::textEdited,
            this, &DetectionConfigWidget::missToleranceEditSlot);

    form_layout_->addRow("Miss Tolerance [s]", miss_tolerance_edit_);
}


void DetectionConfigWidget::updateIntervalEditSlot(QString value)
{
    loginf << "EvaluationRequirementDetectionConfigWidget: updateIntervalEditSlot: value " << value.toStdString();

    bool ok;
    float val = value.toFloat(&ok);

    if (ok)
        config().updateInterval(val);
    else
        loginf << "EvaluationRequirementDetectionConfigWidget: updateIntervalEditSlot: invalid value";
}

// min
void DetectionConfigWidget::toggleUseMinGapLengthSlot()
{
    loginf << "EvaluationRequirementDetectionConfigWidget: toggleUseMinGapLengthSlot";

    assert (use_min_gap_length_check_);
    config().useMinGapLength(use_min_gap_length_check_->checkState() == Qt::Checked);
}
void DetectionConfigWidget::minGapLengthEditSlot(QString value)
{
    loginf << "EvaluationRequirementDetectionConfigWidget: minGapLengthEditSlot: value " << value.toStdString();

    bool ok;
    float val = value.toFloat(&ok);

    if (ok)
        config().minGapLength(val);
    else
        loginf << "EvaluationRequirementDetectionConfigWidget: minGapLengthEditSlot: invalid value";
}

// max
void DetectionConfigWidget::toggleUseMaxGapLengthSlot()
{
    loginf << "EvaluationRequirementDetectionConfigWidget: toggleUseMaxGapLengthSlot";

    assert (use_max_gap_length_check_);
    config().useMaxGapLength(use_max_gap_length_check_->checkState() == Qt::Checked);
}
void DetectionConfigWidget::maxGapLengthEditSlot(QString value)
{
    loginf << "EvaluationRequirementDetectionConfigWidget: maxGapLengthEditSlot: value " << value.toStdString();

    bool ok;
    float val = value.toFloat(&ok);

    if (ok)
        config().maxGapLength(val);
    else
        loginf << "EvaluationRequirementDetectionConfigWidget: maxGapLengthEditSlot: axvalid value";
}

// miss tol
void DetectionConfigWidget::toggleUseMissToleranceSlot()
{
    loginf << "EvaluationRequirementDetectionConfigWidget: toggleUseMissToleranceSlot";

    assert (use_miss_tolerance_check_);
    config().useMissTolerance(use_miss_tolerance_check_->checkState() == Qt::Checked);
}
void DetectionConfigWidget::missToleranceEditSlot(QString value)
{
    loginf << "EvaluationRequirementDetectionConfigWidget: missToleranceEditSlot: value " << value.toStdString();

    bool ok;
    float val = value.toFloat(&ok);

    if (ok)
        config().missTolerance(val);
    else
        loginf << "EvaluationRequirementDetectionConfigWidget: missToleranceEditSlot: invalid value";
}

DetectionConfig& DetectionConfigWidget::config()
{
    DetectionConfig* config = dynamic_cast<DetectionConfig*>(&config_);
    assert (config);

    return *config;
}

}
