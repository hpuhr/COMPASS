#include "evaluationrequirementdetectionconfigwidget.h"
#include "evaluationrequirementdetectionconfig.h"
#include "textfielddoublevalidator.h"
#include "logger.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>

using namespace std;

namespace EvaluationRequirement
{

DetectionConfigWidget::DetectionConfigWidget(
        DetectionConfig& config)
    : QWidget(), config_(config)
{
    form_layout_ = new QFormLayout();

    config_.addGUIElements(form_layout_);

    // ui
    update_interval_edit_ = new QLineEdit(QString::number(config_.updateInterval()));
    update_interval_edit_->setValidator(new QDoubleValidator(0.1, 30.0, 2, this));
    connect(update_interval_edit_, &QLineEdit::textEdited,
            this, &DetectionConfigWidget::updateIntervalEditSlot);

    form_layout_->addRow("Update Interval [s]", update_interval_edit_);

    // prob
    minimum_prob_edit_ = new QLineEdit(QString::number(config_.minimumProbability()));
    minimum_prob_edit_->setValidator(new QDoubleValidator(0.01, 1.0, 2, this));
    connect(minimum_prob_edit_, &QLineEdit::textEdited,
            this, &DetectionConfigWidget::minimumProbEditSlot);

    form_layout_->addRow("Minimum Probability [1]", minimum_prob_edit_);

    // max gap
    use_max_gap_check_ = new QCheckBox ();
    use_max_gap_check_->setChecked(config_.useMaxGapInterval());
    connect(use_max_gap_check_, &QCheckBox::clicked,
            this, &DetectionConfigWidget::toggleUseMaxGapSlot);

    form_layout_->addRow("Use Maximum Gap Supression", use_max_gap_check_);

    max_gap_interval_edit_ = new QLineEdit(QString::number(config_.maxGapInterval()));
    max_gap_interval_edit_->setValidator(new QDoubleValidator(0.01, 600, 2, this));
    connect(max_gap_interval_edit_, &QLineEdit::textEdited,
            this, &DetectionConfigWidget::maxGapEditSlot);

    form_layout_->addRow("Maximum Gap [s]", max_gap_interval_edit_);

    // miss tolerance
    use_miss_tolerance_check_ = new QCheckBox ();
    use_miss_tolerance_check_->setChecked(config_.useMissTolerance());
    connect(use_miss_tolerance_check_, &QCheckBox::clicked,
            this, &DetectionConfigWidget::toggleUseMissToleranceSlot);

    form_layout_->addRow("Use Miss Tolerance", use_miss_tolerance_check_);

    miss_tolerance_edit_ = new QLineEdit(QString::number(config_.missTolerance()));
    miss_tolerance_edit_->setValidator(new QDoubleValidator(0.01, 1.0, 3, this));
    connect(miss_tolerance_edit_, &QLineEdit::textEdited,
            this, &DetectionConfigWidget::missToleranceEditSlot);

    form_layout_->addRow("Miss Tolerance [s]", miss_tolerance_edit_);

    setLayout(form_layout_);
}


void DetectionConfigWidget::updateIntervalEditSlot(QString value)
{
    loginf << "EvaluationRequirementDetectionConfigWidget: updateIntervalEditSlot: value " << value.toStdString();

    bool ok;
    float val = value.toFloat(&ok);

    if (ok)
        config_.updateInterval(val);
    else
        loginf << "EvaluationRequirementDetectionConfigWidget: updateIntervalEditSlot: invalid value";
}

void DetectionConfigWidget::minimumProbEditSlot(QString value)
{
    loginf << "EvaluationRequirementDetectionConfigWidget: minimumProbEditSlot: value " << value.toStdString();

    bool ok;
    float val = value.toFloat(&ok);

    if (ok)
        config_.minimumProbability(val);
    else
        loginf << "EvaluationRequirementDetectionConfigWidget: minimumProbEditSlot: invalid value";
}

void DetectionConfigWidget::toggleUseMaxGapSlot()
{
    loginf << "EvaluationRequirementDetectionConfigWidget: toggleUseMaxGapSlot";

    assert (use_max_gap_check_);
    config_.useMaxGapInterval(use_max_gap_check_->checkState() == Qt::Checked);
}
void DetectionConfigWidget::maxGapEditSlot(QString value)
{
    loginf << "EvaluationRequirementDetectionConfigWidget: maxGapEditSlot: value " << value.toStdString();

    bool ok;
    float val = value.toFloat(&ok);

    if (ok)
        config_.maxGapInterval(val);
    else
        loginf << "EvaluationRequirementDetectionConfigWidget: maxGapEditSlot: invalid value";
}

void DetectionConfigWidget::toggleUseMissToleranceSlot()
{
    loginf << "EvaluationRequirementDetectionConfigWidget: toggleUseMissToleranceSlot";

    assert (use_miss_tolerance_check_);
    config_.useMissTolerance(use_miss_tolerance_check_->checkState() == Qt::Checked);
}
void DetectionConfigWidget::missToleranceEditSlot(QString value)
{
    loginf << "EvaluationRequirementDetectionConfigWidget: missToleranceEditSlot: value " << value.toStdString();

    bool ok;
    float val = value.toFloat(&ok);

    if (ok)
        config_.missTolerance(val);
    else
        loginf << "EvaluationRequirementDetectionConfigWidget: missToleranceEditSlot: invalid value";
}

}
