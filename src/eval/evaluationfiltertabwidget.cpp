#include "evaluationfiltertabwidget.h"
#include "evaluationmanager.h"
#include "textfielddoublevalidator.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QScrollArea>

using namespace Utils;

/**
 */
EvaluationFilterTabWidget::EvaluationFilterTabWidget(EvaluationCalculator& calculator)
    : QWidget(nullptr), calculator_(calculator)
{
    QFormLayout* form_layout = new QFormLayout;
    form_layout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);

    use_filter_check_ = new QCheckBox ();
    connect(use_filter_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseFiltersSlot);
    form_layout->addRow("Use Load Filter", use_filter_check_);

    // reftraj

    use_reftraj_acc_check_ = new QCheckBox ();
    connect(use_reftraj_acc_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseRefTrajAccuracySlot);
    form_layout->addRow("Use RefTraj Accuracy Filter", use_reftraj_acc_check_);

    min_reftraj_acc_edit_ = new QLineEdit();
    min_reftraj_acc_edit_->setValidator(new TextFieldDoubleValidator(0, 3600, 2));
    connect(min_reftraj_acc_edit_, &QLineEdit::textEdited,
            this, &EvaluationFilterTabWidget::minRefTrajAccuracyEditedSlot);
    //form_layout->addWidget(min_reftraj_acc_edit_, row, 1);
    form_layout->addRow("RefTraj Minimum Accuracy", min_reftraj_acc_edit_);


    // adsb

    use_adsb_check_ = new QCheckBox ();
    connect(use_adsb_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseADSBSlot);
    form_layout->addRow("Use ADS-B Filter", use_adsb_check_);

    // v0
    use_v0_check_ = new QCheckBox ();
    connect(use_v0_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseV0Slot);
    form_layout->addRow("Use V0", use_v0_check_);

    // nucp
    use_min_nucp_check_ = new QCheckBox ("Use Min NUCp");
    connect(use_min_nucp_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMinNUCPSlot);

    min_nucp_edit_ = new QLineEdit();
    connect(min_nucp_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::minNUCPEditedSlot);
    form_layout->addRow(use_min_nucp_check_, min_nucp_edit_);

    use_max_nucp_check_ = new QCheckBox ("Use Max NUCp");
    connect(use_max_nucp_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMaxNUCPSlot);

    max_nucp_edit_ = new QLineEdit();
    connect(max_nucp_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::maxNUCPEditedSlot);
    form_layout->addRow(use_max_nucp_check_, max_nucp_edit_);

    // v1
    use_v1_check_ = new QCheckBox ();
    connect(use_v1_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseV1Slot);
    form_layout->addRow("Use V1", use_v1_check_);

    use_v2_check_ = new QCheckBox ();
    connect(use_v2_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseV2Slot);
    form_layout->addRow("Use V2", use_v2_check_);

    // nic
    use_min_nic_check_ = new QCheckBox ("Use Min NIC");
    connect(use_min_nic_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMinNICSlot);

    min_nic_edit_ = new QLineEdit();
    connect(min_nic_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::minNICEditedSlot);
    form_layout->addRow(use_min_nic_check_, min_nic_edit_);

    use_max_nic_check_ = new QCheckBox ("Use Max NIC");
    connect(use_max_nic_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMaxNICSlot);

    max_nic_edit_ = new QLineEdit();
    connect(max_nic_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::maxNICEditedSlot);
    form_layout->addRow(use_max_nic_check_, max_nic_edit_);

    // nacp
    use_min_nacp_check_ = new QCheckBox ("Use Min NACp");
    connect(use_min_nacp_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMinNACpSlot);

    min_nacp_edit_ = new QLineEdit();
    connect(min_nacp_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::minNACPEditedSlot);
    form_layout->addRow(use_min_nacp_check_, min_nacp_edit_);

    use_max_nacp_check_ = new QCheckBox ("Use Max NACp");
    connect(use_max_nacp_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMaxNACpSlot);

    max_nacp_edit_ = new QLineEdit();
    connect(max_nacp_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::maxNACPEditedSlot);
    form_layout->addRow(use_max_nacp_check_, max_nacp_edit_);

    // sil
    use_min_sil_v1_check_ = new QCheckBox ("Use Min SIL v1");
    connect(use_min_sil_v1_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMinSILv1Slot);

    min_sil_v1_edit_ = new QLineEdit();
    connect(min_sil_v1_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::minSILv1PEditedSlot);
    form_layout->addRow(use_min_sil_v1_check_, min_sil_v1_edit_);

    use_max_sil_v1_check_ = new QCheckBox ("Use Max SIL v1");
    connect(use_max_sil_v1_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMaxSILv1Slot);

    max_sil_v1_edit_ = new QLineEdit();
    connect(max_sil_v1_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::maxSILv1PEditedSlot);
    form_layout->addRow(use_max_sil_v1_check_, max_sil_v1_edit_);

    use_min_sil_v2_check_ = new QCheckBox ("Use Min SIL v2");
    connect(use_min_sil_v2_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMinSILv2Slot);

    min_sil_v2_edit_ = new QLineEdit();
    connect(min_sil_v2_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::minSILv2PEditedSlot);
    form_layout->addRow(use_min_sil_v2_check_, min_sil_v2_edit_);

    use_max_sil_v2_check_ = new QCheckBox ("Use Max SIL v2");
    connect(use_max_sil_v2_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMaxSILv2Slot);

    max_sil_v2_edit_ = new QLineEdit();
    connect(max_sil_v2_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::maxSILv2PEditedSlot);
    form_layout->addRow(use_max_sil_v2_check_, max_sil_v2_edit_);

    updateValues();

    setContentsMargins(0, 0, 0, 0);

    setLayout(form_layout);
}

/**
 */
void EvaluationFilterTabWidget::toggleUseFiltersSlot()
{
    assert (use_filter_check_);
    calculator_.settings().use_load_filter_ = use_filter_check_->checkState() == Qt::Checked;

    updateValues();
}



/**
 */
void EvaluationFilterTabWidget::toggleUseRefTrajAccuracySlot()
{
    assert (use_reftraj_acc_check_);
    calculator_.settings().use_ref_traj_accuracy_filter_ = use_reftraj_acc_check_->checkState() == Qt::Checked;
}

/**
 */
void EvaluationFilterTabWidget::minRefTrajAccuracyEditedSlot (const QString& text)
{
    float val;
    bool ok;

    val = text.toFloat(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: minRefTrajAccuracyEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        calculator_.settings().ref_traj_minimum_accuracy_ = val;
}

/**
 */
void EvaluationFilterTabWidget::toggleUseADSBSlot()
{
    assert (use_adsb_check_);
    calculator_.settings().use_adsb_filter_ = use_adsb_check_->checkState() == Qt::Checked;

    updateValues();
}

/**
 */
void EvaluationFilterTabWidget::toggleUseV0Slot()
{
    assert (use_v0_check_);
    calculator_.settings().use_v0_ = use_v0_check_->checkState() == Qt::Checked;

    updateValues();
}

/**
 */
void EvaluationFilterTabWidget::toggleUseV1Slot()
{
    assert (use_v1_check_);
    calculator_.settings().use_v1_ = use_v1_check_->checkState() == Qt::Checked;

    updateValues();
}

/**
 */
void EvaluationFilterTabWidget::toggleUseV2Slot()
{
    assert (use_v2_check_);
    calculator_.settings().use_v2_ = use_v2_check_->checkState() == Qt::Checked;

    updateValues();
}

/**
 */
void EvaluationFilterTabWidget::toggleUseMinNUCPSlot()
{
    assert (use_min_nucp_check_);
    calculator_.settings().use_min_nucp_ = use_min_nucp_check_->checkState() == Qt::Checked;

    updateValues();
}

/**
 */
void EvaluationFilterTabWidget::minNUCPEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: minNUCPEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        calculator_.settings().min_nucp_ = val;
}

/**
 */
void EvaluationFilterTabWidget::toggleUseMinNICSlot()
{
    assert (use_min_nic_check_);
    calculator_.settings().use_min_nic_ = use_min_nic_check_->checkState() == Qt::Checked;

    updateValues();
}

/**
 */
void EvaluationFilterTabWidget::minNICEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: minNICEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        calculator_.settings().min_nic_ = val;
}

/**
 */
void EvaluationFilterTabWidget::toggleUseMinNACpSlot()
{
    assert (use_min_nacp_check_);
    calculator_.settings().use_min_nacp_ = use_min_nacp_check_->checkState() == Qt::Checked;

    updateValues();
}

/**
 */
void EvaluationFilterTabWidget::minNACPEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: minNACPEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        calculator_.settings().min_nacp_ = val;
}

/**
 */
void EvaluationFilterTabWidget::toggleUseMinSILv1Slot()
{
    assert (use_min_sil_v1_check_);
    calculator_.settings().use_min_sil_v1_ = use_min_sil_v1_check_->checkState() == Qt::Checked;

    updateValues();
}

/**
 */
void EvaluationFilterTabWidget::minSILv1PEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: minSILv1PEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        calculator_.settings().min_sil_v1_ = val;
}

/**
 */
void EvaluationFilterTabWidget::toggleUseMinSILv2Slot()
{
    assert (use_min_sil_v2_check_);
    calculator_.settings().use_min_sil_v2_ = use_min_sil_v2_check_->checkState() == Qt::Checked;

    updateValues();
}

/**
 */
void EvaluationFilterTabWidget::minSILv2PEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: minSILv2PEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        calculator_.settings().min_sil_v2_ = val;
}

/**
 */
void EvaluationFilterTabWidget::toggleUseMaxNUCPSlot()
{
    assert (use_max_nucp_check_);
    calculator_.settings().use_max_nucp_ = use_max_nucp_check_->checkState() == Qt::Checked;

    updateValues();
}

/**
 */
void EvaluationFilterTabWidget::maxNUCPEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: maxNUCPEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        calculator_.settings().max_nucp_ = val;
}

/**
 */
void EvaluationFilterTabWidget::toggleUseMaxNICSlot()
{
    assert (use_max_nic_check_);
    calculator_.settings().use_max_nic_ = use_max_nic_check_->checkState() == Qt::Checked;

    updateValues();
}

/**
 */
void EvaluationFilterTabWidget::maxNICEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: maxNICEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        calculator_.settings().max_nic_ = val;
}

/**
 */
void EvaluationFilterTabWidget::toggleUseMaxNACpSlot()
{
    assert (use_max_nacp_check_);
    calculator_.settings().use_max_nacp_ = use_max_nacp_check_->checkState() == Qt::Checked;

    updateValues();
}

/**
 */
void EvaluationFilterTabWidget::maxNACPEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: maxNACPEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        calculator_.settings().max_nacp_ = val;
}

/**
 */
void EvaluationFilterTabWidget::toggleUseMaxSILv1Slot()
{
    assert (use_max_sil_v1_check_);
    calculator_.settings().use_max_sil_v1_ = use_max_sil_v1_check_->checkState() == Qt::Checked;

    updateValues();
}

/**
 */
void EvaluationFilterTabWidget::maxSILv1PEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: maxSILv1PEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        calculator_.settings().max_sil_v1_ = val;
}

/**
 */
void EvaluationFilterTabWidget::toggleUseMaxSILv2Slot()
{
    assert (use_max_sil_v2_check_);
    calculator_.settings().use_max_sil_v2_ = use_max_sil_v2_check_->checkState() == Qt::Checked;

    updateValues();
}

/**
 */
void EvaluationFilterTabWidget::maxSILv2PEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: maxSILv2PEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        calculator_.settings().max_sil_v2_ = val;
}

/**
 */
void EvaluationFilterTabWidget::updateValues()
{
    const auto& eval_settings = calculator_.settings();

    bool use_filter = eval_settings.use_load_filter_;

    assert (use_filter_check_);
    use_filter_check_->setChecked(eval_settings.use_load_filter_);

    // reftraj

    assert(use_reftraj_acc_check_);
    use_reftraj_acc_check_->setChecked(eval_settings.use_ref_traj_accuracy_filter_);
    use_reftraj_acc_check_->setEnabled(use_filter);

    assert (min_reftraj_acc_edit_);
    min_reftraj_acc_edit_->setText(QString::number(eval_settings.ref_traj_minimum_accuracy_));
    min_reftraj_acc_edit_->setEnabled(use_filter && eval_settings.use_ref_traj_accuracy_filter_);

    // adsb

    bool use_adsb_filter = use_filter && eval_settings.use_adsb_filter_;

    assert (use_adsb_check_);
    use_adsb_check_->setChecked(eval_settings.use_adsb_filter_);
    use_adsb_check_->setEnabled(use_filter);

    // v0
    assert (use_v0_check_);
    use_v0_check_->setChecked(eval_settings.use_v0_);
    use_v0_check_->setEnabled(use_adsb_filter);

    // nucp
    assert (use_min_nucp_check_);
    use_min_nucp_check_->setChecked(eval_settings.use_min_nucp_);
    use_min_nucp_check_->setEnabled(use_adsb_filter && eval_settings.use_v0_);
    assert (min_nucp_edit_);
    min_nucp_edit_->setText(QString::number(eval_settings.min_nucp_));
    min_nucp_edit_->setEnabled(use_adsb_filter && eval_settings.use_min_nucp_ && eval_settings.use_v0_);

    assert (use_max_nucp_check_);
    use_max_nucp_check_->setChecked(eval_settings.use_max_nucp_);
    use_max_nucp_check_->setEnabled(use_adsb_filter && eval_settings.use_v0_);
    assert (max_nucp_edit_);
    max_nucp_edit_->setText(QString::number(eval_settings.max_nucp_));
    max_nucp_edit_->setEnabled(use_adsb_filter && eval_settings.use_max_nucp_ && eval_settings.use_v0_);

    // v1
    assert (use_v1_check_);
    use_v1_check_->setChecked(eval_settings.use_v1_);
    use_v1_check_->setEnabled(use_adsb_filter);
    assert (use_v2_check_);
    use_v2_check_->setChecked(eval_settings.use_v2_);
    use_v2_check_->setEnabled(use_adsb_filter);

    bool use_v12 = eval_settings.use_v1_ || eval_settings.use_v2_;

    // nic
    assert (use_min_nic_check_);
    use_min_nic_check_->setChecked(eval_settings.use_min_nic_);
    use_min_nic_check_->setEnabled(use_adsb_filter && use_v12);
    assert (min_nic_edit_);
    min_nic_edit_->setText(QString::number(eval_settings.min_nic_));
    min_nic_edit_->setEnabled(use_adsb_filter && eval_settings.use_min_nic_ && use_v12);

    assert (use_max_nic_check_);
    use_max_nic_check_->setChecked(eval_settings.use_max_nic_);
    use_max_nic_check_->setEnabled(use_adsb_filter && use_v12);
    assert (max_nic_edit_);
    max_nic_edit_->setText(QString::number(eval_settings.max_nic_));
    max_nic_edit_->setEnabled(use_adsb_filter && eval_settings.use_max_nic_ && use_v12);

    // nacp
    assert (use_min_nacp_check_);
    use_min_nacp_check_->setChecked(eval_settings.use_min_nacp_);
    use_min_nacp_check_->setEnabled(use_adsb_filter && use_v12);
    assert (min_nacp_edit_);
    min_nacp_edit_->setText(QString::number(eval_settings.min_nacp_));
    min_nacp_edit_->setEnabled(use_adsb_filter && eval_settings.use_min_nacp_ && use_v12);

    assert (use_max_nacp_check_);
    use_max_nacp_check_->setChecked(eval_settings.use_max_nacp_);
    use_max_nacp_check_->setEnabled(use_adsb_filter && use_v12);
    assert (max_nacp_edit_);
    max_nacp_edit_->setText(QString::number(eval_settings.max_nacp_));
    max_nacp_edit_->setEnabled(use_adsb_filter && eval_settings.use_max_nacp_ && use_v12);

    // sil v1
    assert (use_min_sil_v1_check_);
    use_min_sil_v1_check_->setChecked(eval_settings.use_min_sil_v1_);
    use_min_sil_v1_check_->setEnabled(use_adsb_filter && use_v12);
    assert (min_sil_v1_edit_);
    min_sil_v1_edit_->setText(QString::number(eval_settings.min_sil_v1_));
    min_sil_v1_edit_->setEnabled(use_adsb_filter && eval_settings.use_min_sil_v1_ && use_v12);

    assert (use_max_sil_v1_check_);
    use_max_sil_v1_check_->setChecked(eval_settings.use_max_sil_v1_);
    use_max_sil_v1_check_->setEnabled(use_adsb_filter && use_v12);
    assert (max_sil_v1_edit_);
    max_sil_v1_edit_->setText(QString::number(eval_settings.max_sil_v1_));
    max_sil_v1_edit_->setEnabled(use_adsb_filter && eval_settings.use_max_sil_v1_ && use_v12);

    // sil v2
    assert (use_min_sil_v2_check_);
    use_min_sil_v2_check_->setChecked(eval_settings.use_min_sil_v2_);
    use_min_sil_v2_check_->setEnabled(use_adsb_filter && use_v12);
    assert (min_sil_v2_edit_);
    min_sil_v2_edit_->setText(QString::number(eval_settings.min_sil_v2_));
    min_sil_v2_edit_->setEnabled(use_adsb_filter && eval_settings.use_min_sil_v2_ && use_v12);

    assert (use_max_sil_v2_check_);
    use_max_sil_v2_check_->setChecked(eval_settings.use_max_sil_v2_);
    use_max_sil_v2_check_->setEnabled(use_adsb_filter && use_v12);
    assert (max_sil_v2_edit_);
    max_sil_v2_edit_->setText(QString::number(eval_settings.max_sil_v2_));
    max_sil_v2_edit_->setEnabled(use_adsb_filter && eval_settings.use_max_sil_v2_ && use_v12);
}
