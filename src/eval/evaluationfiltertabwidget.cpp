#include "evaluationfiltertabwidget.h"
#include "evaluationmanagerwidget.h"
#include "evaluationmanager.h"
#include "stringconv.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QCheckBox>
#include <QLineEdit>

using namespace Utils;

EvaluationFilterTabWidget::EvaluationFilterTabWidget(EvaluationManager& eval_man,
                                                     EvaluationManagerWidget& man_widget)
    : QWidget(nullptr), eval_man_(eval_man), man_widget_(man_widget)
{
    QHBoxLayout* main_layout = new QHBoxLayout();

    QGridLayout* layout = new QGridLayout();

    int row = 0;

    QLabel* label = new QLabel("Use Load Filter");
    label->setMinimumWidth(100);
    layout->addWidget(label, row, 0);

    use_filter_check_ = new QCheckBox ();
    connect(use_filter_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseFiltersSlot);
    layout->addWidget(use_filter_check_, row, 1);

    // time
    ++row;
    layout->addWidget(new QLabel("Use Time Filter"), row, 0);

    use_time_check_ = new QCheckBox ();
    connect(use_time_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseTimeSlot);
    layout->addWidget(use_time_check_, row, 1);

    ++row;
    layout->addWidget(new QLabel("Time Begin"), row, 0);

    time_begin_edit_ = new QLineEdit();
    connect(time_begin_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::timeBeginEditedSlot);
    layout->addWidget(time_begin_edit_, row, 1);

    ++row;
    layout->addWidget(new QLabel("Time End"), row, 0);

    time_end_edit_ = new QLineEdit();
    connect(time_end_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::timeEndEditedSlot);
    layout->addWidget(time_end_edit_, row, 1);

    // adsb
    ++row;
    layout->addWidget(new QLabel("Use ADS-B Filter"), row, 0);

    use_adsb_check_ = new QCheckBox ();
    connect(use_adsb_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseADSBSlot);
    layout->addWidget(use_adsb_check_, row, 1);

    // v0
    ++row;
    layout->addWidget(new QLabel("Use v0"), row, 0);

    use_v0_check_ = new QCheckBox ();
    connect(use_v0_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseV0Slot);
    layout->addWidget(use_v0_check_, row, 1);

    // nucp
    ++row;
    layout->addWidget(new QLabel("Use Min NUCp"), row, 0);

    use_min_nucp_check_ = new QCheckBox ();
    connect(use_min_nucp_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMinNUCPSlot);
    layout->addWidget(use_min_nucp_check_, row, 1);

    ++row;
    layout->addWidget(new QLabel("Min NUCp"), row, 0);

    min_nucp_edit_ = new QLineEdit();
    connect(min_nucp_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::minNUCPEditedSlot);
    layout->addWidget(min_nucp_edit_, row, 1);

    // others
    ++row;
    layout->addWidget(new QLabel("Use v1"), row, 0);

    use_v1_check_ = new QCheckBox ();
    connect(use_v1_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseV1Slot);
    layout->addWidget(use_v1_check_, row, 1);

    ++row;
    layout->addWidget(new QLabel("Use v2"), row, 0);

    use_v2_check_ = new QCheckBox ();
    connect(use_v2_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseV2Slot);
    layout->addWidget(use_v2_check_, row, 1);

    // nic
    ++row;
    layout->addWidget(new QLabel("Use Min NIC"), row, 0);

    use_min_nic_check_ = new QCheckBox ();
    connect(use_min_nic_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMinNICSlot);
    layout->addWidget(use_min_nic_check_, row, 1);

    ++row;
    layout->addWidget(new QLabel("Min NIC"), row, 0);

    min_nic_edit_ = new QLineEdit();
    connect(min_nic_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::minNICEditedSlot);
    layout->addWidget(min_nic_edit_, row, 1);

    // nacp
    ++row;
    layout->addWidget(new QLabel("Use Min NACp"), row, 0);

    use_min_nacp_check_ = new QCheckBox ();
    connect(use_min_nacp_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMinNACpSlot);
    layout->addWidget(use_min_nacp_check_, row, 1);

    ++row;
    layout->addWidget(new QLabel("Min NACp"), row, 0);

    min_nacp_edit_ = new QLineEdit();
    connect(min_nacp_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::minNACPEditedSlot);
    layout->addWidget(min_nacp_edit_, row, 1);

    // sil
    ++row;
    layout->addWidget(new QLabel("Use Min SIL v1"), row, 0);

    use_min_sil_v1_check_ = new QCheckBox ();
    connect(use_min_sil_v1_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMinSILv1Slot);
    layout->addWidget(use_min_sil_v1_check_, row, 1);

    ++row;
    layout->addWidget(new QLabel("Min SIL v1"), row, 0);

    min_sil_v1_edit_ = new QLineEdit();
    connect(min_sil_v1_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::minSILv1PEditedSlot);
    layout->addWidget(min_sil_v1_edit_, row, 1);

    ++row;
    layout->addWidget(new QLabel("Use Min SIL v2"), row, 0);

    use_min_sil_v2_check_ = new QCheckBox ();
    connect(use_min_sil_v2_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMinSILv2Slot);
    layout->addWidget(use_min_sil_v2_check_, row, 1);

    ++row;
    layout->addWidget(new QLabel("Min SIL v2"), row, 0);

    min_sil_v2_edit_ = new QLineEdit();
    connect(min_sil_v2_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::minSILv2PEditedSlot);
    layout->addWidget(min_sil_v2_edit_, row, 1);

    update();

    setContentsMargins(0, 0, 0, 0);

    main_layout->addLayout(layout);
    main_layout->addStretch();

    setLayout(main_layout);
}

void EvaluationFilterTabWidget::toggleUseFiltersSlot()
{
    assert (use_filter_check_);
    eval_man_.useLoadFilter(use_filter_check_->checkState() == Qt::Checked);
}

void EvaluationFilterTabWidget::toggleUseTimeSlot()
{
    assert (use_time_check_);
    eval_man_.useTimeFilter(use_time_check_->checkState() == Qt::Checked);
}

void EvaluationFilterTabWidget::timeBeginEditedSlot (const QString& text)
{
    double val;
    bool ok;

    val = String::timeFromString(text.toStdString(), &ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: timeBeginEditedSlot: unable to parse value '"
               << text.toStdString() << "'";
    else
        eval_man_.loadTimeBegin(val);
}

void EvaluationFilterTabWidget::timeEndEditedSlot (const QString& text)
{
    double val;
    bool ok;

    val = String::timeFromString(text.toStdString(), &ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: timeEndEditedSlot: unable to parse value '"
               << text.toStdString() << "'";
    else
        eval_man_.loadTimeEnd(val);
}

void EvaluationFilterTabWidget::toggleUseADSBSlot()
{
    assert (use_adsb_check_);
    eval_man_.useASDBFilter(use_adsb_check_->checkState() == Qt::Checked);
}

void EvaluationFilterTabWidget::toggleUseV0Slot()
{
    assert (use_v0_check_);
    eval_man_.useV0(use_v0_check_->checkState() == Qt::Checked);
}

void EvaluationFilterTabWidget::toggleUseV1Slot()
{
    assert (use_v1_check_);
    eval_man_.useV1(use_v1_check_->checkState() == Qt::Checked);
}

void EvaluationFilterTabWidget::toggleUseV2Slot()
{
    assert (use_v2_check_);
    eval_man_.useV2(use_v2_check_->checkState() == Qt::Checked);
}

void EvaluationFilterTabWidget::toggleUseMinNUCPSlot()
{
    assert (use_min_nucp_check_);
    eval_man_.useMinNUCP(use_min_nucp_check_->checkState() == Qt::Checked);
}

void EvaluationFilterTabWidget::minNUCPEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: minNUCPEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        eval_man_.minNUCP(val);
}

void EvaluationFilterTabWidget::toggleUseMinNICSlot()
{
    assert (use_min_nic_check_);
    eval_man_.useMinNIC(use_min_nic_check_->checkState() == Qt::Checked);
}

void EvaluationFilterTabWidget::minNICEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: minNICEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        eval_man_.minNIC(val);
}

void EvaluationFilterTabWidget::toggleUseMinNACpSlot()
{
    assert (use_min_nacp_check_);
    eval_man_.useMinNACp(use_min_nacp_check_->checkState() == Qt::Checked);
}

void EvaluationFilterTabWidget::minNACPEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: minNACPEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        eval_man_.minNACp(val);
}

void EvaluationFilterTabWidget::toggleUseMinSILv1Slot()
{
    assert (use_min_sil_v1_check_);
    eval_man_.useMinSILv1(use_min_sil_v1_check_->checkState() == Qt::Checked);
}

void EvaluationFilterTabWidget::minSILv1PEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: minSILv1PEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        eval_man_.minSILv1(val);
}


void EvaluationFilterTabWidget::toggleUseMinSILv2Slot()
{
    assert (use_min_sil_v2_check_);
    eval_man_.useMinSILv2(use_min_sil_v2_check_->checkState() == Qt::Checked);
}

void EvaluationFilterTabWidget::minSILv2PEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: minSILv2PEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        eval_man_.minSILv2(val);
}

void EvaluationFilterTabWidget::update()
{
    assert (use_filter_check_);
    use_filter_check_->setChecked(eval_man_.useLoadFilter());

    assert (use_time_check_);
    use_time_check_->setChecked(eval_man_.useTimeFilter());

    assert (time_begin_edit_);
    time_begin_edit_->setText(String::timeStringFromDouble(eval_man_.loadTimeBegin()).c_str());

    assert (time_end_edit_);
    time_end_edit_->setText(String::timeStringFromDouble(eval_man_.loadTimeEnd()).c_str());

    assert (use_adsb_check_);
    use_adsb_check_->setChecked(eval_man_.useASDBFilter());

    assert (use_v0_check_);
    use_v0_check_->setChecked(eval_man_.useV0());

    assert (use_min_nucp_check_);
    use_min_nucp_check_->setChecked(eval_man_.useMinNUCP());
    assert (min_nucp_edit_);
    min_nucp_edit_->setText(QString::number(eval_man_.minNUCP()));

    assert (use_v1_check_);
    use_v1_check_->setChecked(eval_man_.useV1());
    assert (use_v2_check_);
    use_v2_check_->setChecked(eval_man_.useV2());

    assert (use_min_nic_check_);
    use_min_nic_check_->setChecked(eval_man_.useMinNIC());
    assert (min_nic_edit_);
    min_nic_edit_->setText(QString::number(eval_man_.minNIC()));

    assert (use_min_nacp_check_);
    use_min_nacp_check_->setChecked(eval_man_.useMinNACp());
    assert (min_nacp_edit_);
    min_nacp_edit_->setText(QString::number(eval_man_.minNACp()));

    assert (use_min_sil_v1_check_);
    use_min_sil_v1_check_->setChecked(eval_man_.useMinSILv1());
    assert (min_sil_v1_edit_);
    min_sil_v1_edit_->setText(QString::number(eval_man_.minSILv1()));

    assert (use_min_sil_v2_check_);
    use_min_sil_v2_check_->setChecked(eval_man_.useMinSILv2());
    assert (min_sil_v2_edit_);
    min_sil_v2_edit_->setText(QString::number(eval_man_.minSILv2()));
}
