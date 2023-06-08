#include "evaluationfiltertabwidget.h"
#include "evaluationmanagerwidget.h"
#include "evaluationmanager.h"
#include "util/stringconv.h"
#include "util/timeconv.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QDateTime>
#include <QDateTimeEdit>

using namespace Utils;

EvaluationFilterTabWidget::EvaluationFilterTabWidget(
        EvaluationManager& eval_man, EvaluationManagerSettings& eval_settings, EvaluationManagerWidget& man_widget)
    : QWidget(nullptr), eval_man_(eval_man), eval_settings_(eval_settings), man_widget_(man_widget)
{
    QHBoxLayout* main_layout = new QHBoxLayout();

    QGridLayout* layout = new QGridLayout();

    int row = 0;

    use_filter_check_ = new QCheckBox ("Use Load Filter");
    connect(use_filter_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseFiltersSlot);
    layout->addWidget(use_filter_check_, row, 0);

    // time
    ++row;
    use_time_check_ = new QCheckBox ("Use Timestamp Filter");
    connect(use_time_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseTimeSlot);
    layout->addWidget(use_time_check_, row, 0);

    ++row;
    layout->addWidget(new QLabel("Timestamp Begin"), row, 0);

    time_begin_edit_ = new QDateTimeEdit(QDateTime::currentDateTime());
    time_begin_edit_->setDisplayFormat(Time::QT_DATETIME_FORMAT.c_str());
    connect(time_begin_edit_, &QDateTimeEdit::dateTimeChanged, this, &EvaluationFilterTabWidget::timeBeginEditedSlot);
    layout->addWidget(time_begin_edit_, row, 1);

//    time_begin_edit_ = new QLineEdit();
//    connect(time_begin_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::timeBeginEditedSlot);
//    layout->addWidget(time_begin_edit_, row, 1);

    ++row;
    layout->addWidget(new QLabel("Timestamp End"), row, 0);

    time_end_edit_ = new QDateTimeEdit(QDateTime::currentDateTime());
    time_end_edit_->setDisplayFormat(Time::QT_DATETIME_FORMAT.c_str());
    connect(time_end_edit_, &QDateTimeEdit::dateTimeChanged, this, &EvaluationFilterTabWidget::timeEndEditedSlot);
    layout->addWidget(time_end_edit_, row, 1);

//    time_end_edit_ = new QLineEdit();
//    connect(time_end_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::timeEndEditedSlot);
//    layout->addWidget(time_end_edit_, row, 1);

    // adsb
    ++row;

    use_adsb_check_ = new QCheckBox ("Use ADS-B Filter");
    connect(use_adsb_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseADSBSlot);
    layout->addWidget(use_adsb_check_, row, 0);

    // v0
    ++row;

    use_v0_check_ = new QCheckBox ("Use v0");
    connect(use_v0_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseV0Slot);
    layout->addWidget(use_v0_check_, row, 0);

    // nucp
    ++row;

    use_min_nucp_check_ = new QCheckBox ("Use Min NUCp");
    connect(use_min_nucp_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMinNUCPSlot);
    layout->addWidget(use_min_nucp_check_, row, 0);

    min_nucp_edit_ = new QLineEdit();
    connect(min_nucp_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::minNUCPEditedSlot);
    layout->addWidget(min_nucp_edit_, row, 1);

    ++row;

    use_max_nucp_check_ = new QCheckBox ("Use Max NUCp");
    connect(use_max_nucp_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMaxNUCPSlot);
    layout->addWidget(use_max_nucp_check_, row, 0);

    max_nucp_edit_ = new QLineEdit();
    connect(max_nucp_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::maxNUCPEditedSlot);
    layout->addWidget(max_nucp_edit_, row, 1);

    // v1
    ++row;

    use_v1_check_ = new QCheckBox ("Use v1");
    connect(use_v1_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseV1Slot);
    layout->addWidget(use_v1_check_, row, 0);

    ++row;

    use_v2_check_ = new QCheckBox ("Use v2");
    connect(use_v2_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseV2Slot);
    layout->addWidget(use_v2_check_, row, 0);

    // nic
    ++row;

    use_min_nic_check_ = new QCheckBox ("Use Min NIC");
    connect(use_min_nic_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMinNICSlot);
    layout->addWidget(use_min_nic_check_, row, 0);

    min_nic_edit_ = new QLineEdit();
    connect(min_nic_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::minNICEditedSlot);
    layout->addWidget(min_nic_edit_, row, 1);

    ++row;

    use_max_nic_check_ = new QCheckBox ("Use Max NIC");
    connect(use_max_nic_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMaxNICSlot);
    layout->addWidget(use_max_nic_check_, row, 0);

    max_nic_edit_ = new QLineEdit();
    connect(max_nic_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::maxNICEditedSlot);
    layout->addWidget(max_nic_edit_, row, 1);

    // nacp
    ++row;

    use_min_nacp_check_ = new QCheckBox ("Use Min NACp");
    connect(use_min_nacp_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMinNACpSlot);
    layout->addWidget(use_min_nacp_check_, row, 0);

    min_nacp_edit_ = new QLineEdit();
    connect(min_nacp_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::minNACPEditedSlot);
    layout->addWidget(min_nacp_edit_, row, 1);

    ++row;

    use_max_nacp_check_ = new QCheckBox ("Use Max NACp");
    connect(use_max_nacp_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMaxNACpSlot);
    layout->addWidget(use_max_nacp_check_, row, 0);

    max_nacp_edit_ = new QLineEdit();
    connect(max_nacp_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::maxNACPEditedSlot);
    layout->addWidget(max_nacp_edit_, row, 1);

    // sil
    ++row;

    use_min_sil_v1_check_ = new QCheckBox ("Use Min SIL v1");
    connect(use_min_sil_v1_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMinSILv1Slot);
    layout->addWidget(use_min_sil_v1_check_, row, 0);

    min_sil_v1_edit_ = new QLineEdit();
    connect(min_sil_v1_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::minSILv1PEditedSlot);
    layout->addWidget(min_sil_v1_edit_, row, 1);

    ++row;

    use_max_sil_v1_check_ = new QCheckBox ("Use Max SIL v1");
    connect(use_max_sil_v1_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMaxSILv1Slot);
    layout->addWidget(use_max_sil_v1_check_, row, 0);

    max_sil_v1_edit_ = new QLineEdit();
    connect(max_sil_v1_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::maxSILv1PEditedSlot);
    layout->addWidget(max_sil_v1_edit_, row, 1);

    ++row;

    use_min_sil_v2_check_ = new QCheckBox ("Use Min SIL v2");
    connect(use_min_sil_v2_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMinSILv2Slot);
    layout->addWidget(use_min_sil_v2_check_, row, 0);

    min_sil_v2_edit_ = new QLineEdit();
    connect(min_sil_v2_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::minSILv2PEditedSlot);
    layout->addWidget(min_sil_v2_edit_, row, 1);

    ++row;

    use_max_sil_v2_check_ = new QCheckBox ("Use Max SIL v2");
    connect(use_max_sil_v2_check_, &QCheckBox::clicked, this, &EvaluationFilterTabWidget::toggleUseMaxSILv2Slot);
    layout->addWidget(use_max_sil_v2_check_, row, 0);

    max_sil_v2_edit_ = new QLineEdit();
    connect(max_sil_v2_edit_, &QLineEdit::textEdited, this, &EvaluationFilterTabWidget::maxSILv2PEditedSlot);
    layout->addWidget(max_sil_v2_edit_, row, 1);

    update();

    setContentsMargins(0, 0, 0, 0);

    main_layout->addLayout(layout);
    main_layout->addStretch();

    setLayout(main_layout);
}

void EvaluationFilterTabWidget::toggleUseFiltersSlot()
{
    assert (use_filter_check_);
    eval_settings_.use_load_filter_ = use_filter_check_->checkState() == Qt::Checked;

    update();
}

void EvaluationFilterTabWidget::toggleUseTimeSlot()
{
    assert (use_time_check_);
    eval_settings_.use_timestamp_filter_ = use_time_check_->checkState() == Qt::Checked;

    update();
}

void EvaluationFilterTabWidget::timeBeginEditedSlot (const QDateTime& datetime)
{
    if (update_active_)
        return;

    loginf << "EvaluationFilterTabWidget: timeBeginEditedSlot: value "
           << datetime.toString(Time::QT_DATETIME_FORMAT.c_str()).toStdString();

    eval_man_.loadTimestampBegin(Time::fromString(datetime.toString(Time::QT_DATETIME_FORMAT.c_str()).toStdString()));
}

void EvaluationFilterTabWidget::timeEndEditedSlot (const QDateTime& datetime)
{
    if (update_active_)
        return;

    loginf << "EvaluationFilterTabWidget: timeEndEditedSlot: value "
           << datetime.toString(Time::QT_DATETIME_FORMAT.c_str()).toStdString();

    eval_man_.loadTimestampEnd(Time::fromString(datetime.toString(Time::QT_DATETIME_FORMAT.c_str()).toStdString()));
}

void EvaluationFilterTabWidget::toggleUseADSBSlot()
{
    assert (use_adsb_check_);
    eval_settings_.use_adsb_filter_ = use_adsb_check_->checkState() == Qt::Checked;

    update();
}

void EvaluationFilterTabWidget::toggleUseV0Slot()
{
    assert (use_v0_check_);
    eval_settings_.use_v0_ = use_v0_check_->checkState() == Qt::Checked;

    update();
}

void EvaluationFilterTabWidget::toggleUseV1Slot()
{
    assert (use_v1_check_);
    eval_settings_.use_v1_ = use_v1_check_->checkState() == Qt::Checked;

    update();
}

void EvaluationFilterTabWidget::toggleUseV2Slot()
{
    assert (use_v2_check_);
    eval_settings_.use_v2_ = use_v2_check_->checkState() == Qt::Checked;

    update();
}

void EvaluationFilterTabWidget::toggleUseMinNUCPSlot()
{
    assert (use_min_nucp_check_);
    eval_settings_.use_min_nucp_ = use_min_nucp_check_->checkState() == Qt::Checked;

    update();
}

void EvaluationFilterTabWidget::minNUCPEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: minNUCPEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        eval_settings_.min_nucp_ = val;
}

void EvaluationFilterTabWidget::toggleUseMinNICSlot()
{
    assert (use_min_nic_check_);
    eval_settings_.use_min_nic_ = use_min_nic_check_->checkState() == Qt::Checked;

    update();
}

void EvaluationFilterTabWidget::minNICEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: minNICEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        eval_settings_.min_nic_ = val;
}

void EvaluationFilterTabWidget::toggleUseMinNACpSlot()
{
    assert (use_min_nacp_check_);
    eval_settings_.use_min_nacp_ = use_min_nacp_check_->checkState() == Qt::Checked;

    update();
}

void EvaluationFilterTabWidget::minNACPEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: minNACPEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        eval_settings_.min_nacp_ = val;
}

void EvaluationFilterTabWidget::toggleUseMinSILv1Slot()
{
    assert (use_min_sil_v1_check_);
    eval_settings_.use_min_sil_v1_ = use_min_sil_v1_check_->checkState() == Qt::Checked;

    update();
}

void EvaluationFilterTabWidget::minSILv1PEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: minSILv1PEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        eval_settings_.min_sil_v1_ = val;
}


void EvaluationFilterTabWidget::toggleUseMinSILv2Slot()
{
    assert (use_min_sil_v2_check_);
    eval_settings_.use_min_sil_v2_ = use_min_sil_v2_check_->checkState() == Qt::Checked;

    update();
}

void EvaluationFilterTabWidget::minSILv2PEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: minSILv2PEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        eval_settings_.min_sil_v2_ = val;
}


void EvaluationFilterTabWidget::toggleUseMaxNUCPSlot()
{
    assert (use_max_nucp_check_);
    eval_settings_.use_max_nucp_ = use_max_nucp_check_->checkState() == Qt::Checked;

    update();
}

void EvaluationFilterTabWidget::maxNUCPEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: maxNUCPEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        eval_settings_.max_nucp_ = val;
}

void EvaluationFilterTabWidget::toggleUseMaxNICSlot()
{
    assert (use_max_nic_check_);
    eval_settings_.use_max_nic_ = use_max_nic_check_->checkState() == Qt::Checked;

    update();
}

void EvaluationFilterTabWidget::maxNICEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: maxNICEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        eval_settings_.max_nic_ = val;
}

void EvaluationFilterTabWidget::toggleUseMaxNACpSlot()
{
    assert (use_max_nacp_check_);
    eval_settings_.use_max_nacp_ = use_max_nacp_check_->checkState() == Qt::Checked;

    update();
}

void EvaluationFilterTabWidget::maxNACPEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: maxNACPEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        eval_settings_.max_nacp_ = val;
}

void EvaluationFilterTabWidget::toggleUseMaxSILv1Slot()
{
    assert (use_max_sil_v1_check_);
    eval_settings_.use_max_sil_v1_ = use_max_sil_v1_check_->checkState() == Qt::Checked;

    update();
}

void EvaluationFilterTabWidget::maxSILv1PEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: maxSILv1PEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        eval_settings_.max_sil_v1_ = val;
}


void EvaluationFilterTabWidget::toggleUseMaxSILv2Slot()
{
    assert (use_max_sil_v2_check_);
    eval_settings_.use_max_sil_v2_ = use_max_sil_v2_check_->checkState() == Qt::Checked;

    update();
}

void EvaluationFilterTabWidget::maxSILv2PEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "EvaluationFilterTabWidget: maxSILv2PEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        eval_settings_.max_sil_v2_ = val;
}

void EvaluationFilterTabWidget::update()
{
    bool use_filter = eval_settings_.use_load_filter_;

    assert (use_filter_check_);
    use_filter_check_->setChecked(eval_settings_.use_load_filter_);

    // time filter
    assert (use_time_check_);
    use_time_check_->setChecked(eval_settings_.use_timestamp_filter_);
    use_time_check_->setEnabled(use_filter);

    update_active_ = true;

    assert (time_begin_edit_);
    //time_begin_edit_->setText(String::timeStringFromDouble(eval_man_.loadTimestampBegin()).c_str());
    time_begin_edit_->setDateTime(QDateTime::fromString(Time::toString(eval_man_.loadTimestampBegin()).c_str(),
                                                 Time::QT_DATETIME_FORMAT.c_str()));
    time_begin_edit_->setEnabled(use_filter && eval_settings_.use_timestamp_filter_);

    assert (time_end_edit_);
    //time_end_edit_->setText(String::timeStringFromDouble(eval_man_.loadTimestampEnd()).c_str());
    time_end_edit_->setDateTime(QDateTime::fromString(Time::toString(eval_man_.loadTimestampEnd()).c_str(),
                                                 Time::QT_DATETIME_FORMAT.c_str()));
    time_end_edit_->setEnabled(use_filter && eval_settings_.use_timestamp_filter_);

    update_active_ = false;

    bool use_adsb_filter = use_filter && eval_settings_.use_adsb_filter_;

    assert (use_adsb_check_);
    use_adsb_check_->setChecked(eval_settings_.use_adsb_filter_);
    use_adsb_check_->setEnabled(use_filter);

    // v0
    assert (use_v0_check_);
    use_v0_check_->setChecked(eval_settings_.use_v0_);
    use_v0_check_->setEnabled(use_adsb_filter);

    // nucp
    assert (use_min_nucp_check_);
    use_min_nucp_check_->setChecked(eval_settings_.use_min_nucp_);
    use_min_nucp_check_->setEnabled(use_adsb_filter && eval_settings_.use_v0_);
    assert (min_nucp_edit_);
    min_nucp_edit_->setText(QString::number(eval_settings_.min_nucp_));
    min_nucp_edit_->setEnabled(use_adsb_filter && eval_settings_.use_min_nucp_ && eval_settings_.use_v0_);

    assert (use_max_nucp_check_);
    use_max_nucp_check_->setChecked(eval_settings_.use_max_nucp_);
    use_max_nucp_check_->setEnabled(use_adsb_filter && eval_settings_.use_v0_);
    assert (max_nucp_edit_);
    max_nucp_edit_->setText(QString::number(eval_settings_.max_nucp_));
    max_nucp_edit_->setEnabled(use_adsb_filter && eval_settings_.use_max_nucp_ && eval_settings_.use_v0_);

    // v1
    assert (use_v1_check_);
    use_v1_check_->setChecked(eval_settings_.use_v1_);
    use_v1_check_->setEnabled(use_adsb_filter);
    assert (use_v2_check_);
    use_v2_check_->setChecked(eval_settings_.use_v2_);
    use_v2_check_->setEnabled(use_adsb_filter);

    bool use_v12 = eval_settings_.use_v1_ || eval_settings_.use_v2_;

    // nic
    assert (use_min_nic_check_);
    use_min_nic_check_->setChecked(eval_settings_.use_min_nic_);
    use_min_nic_check_->setEnabled(use_adsb_filter && use_v12);
    assert (min_nic_edit_);
    min_nic_edit_->setText(QString::number(eval_settings_.min_nic_));
    min_nic_edit_->setEnabled(use_adsb_filter && eval_settings_.use_min_nic_ && use_v12);

    assert (use_max_nic_check_);
    use_max_nic_check_->setChecked(eval_settings_.use_max_nic_);
    use_max_nic_check_->setEnabled(use_adsb_filter && use_v12);
    assert (max_nic_edit_);
    max_nic_edit_->setText(QString::number(eval_settings_.max_nic_));
    max_nic_edit_->setEnabled(use_adsb_filter && eval_settings_.use_max_nic_ && use_v12);

    // nacp
    assert (use_min_nacp_check_);
    use_min_nacp_check_->setChecked(eval_settings_.use_min_nacp_);
    use_min_nacp_check_->setEnabled(use_adsb_filter && use_v12);
    assert (min_nacp_edit_);
    min_nacp_edit_->setText(QString::number(eval_settings_.min_nacp_));
    min_nacp_edit_->setEnabled(use_adsb_filter && eval_settings_.use_min_nacp_ && use_v12);

    assert (use_max_nacp_check_);
    use_max_nacp_check_->setChecked(eval_settings_.use_max_nacp_);
    use_max_nacp_check_->setEnabled(use_adsb_filter && use_v12);
    assert (max_nacp_edit_);
    max_nacp_edit_->setText(QString::number(eval_settings_.max_nacp_));
    max_nacp_edit_->setEnabled(use_adsb_filter && eval_settings_.use_max_nacp_ && use_v12);

    // sil v1
    assert (use_min_sil_v1_check_);
    use_min_sil_v1_check_->setChecked(eval_settings_.use_min_sil_v1_);
    use_min_sil_v1_check_->setEnabled(use_adsb_filter && use_v12);
    assert (min_sil_v1_edit_);
    min_sil_v1_edit_->setText(QString::number(eval_settings_.min_sil_v1_));
    min_sil_v1_edit_->setEnabled(use_adsb_filter && eval_settings_.use_min_sil_v1_ && use_v12);

    assert (use_max_sil_v1_check_);
    use_max_sil_v1_check_->setChecked(eval_settings_.use_max_sil_v1_);
    use_max_sil_v1_check_->setEnabled(use_adsb_filter && use_v12);
    assert (max_sil_v1_edit_);
    max_sil_v1_edit_->setText(QString::number(eval_settings_.max_sil_v1_));
    max_sil_v1_edit_->setEnabled(use_adsb_filter && eval_settings_.use_max_sil_v1_ && use_v12);

    // sil v2
    assert (use_min_sil_v2_check_);
    use_min_sil_v2_check_->setChecked(eval_settings_.use_min_sil_v2_);
    use_min_sil_v2_check_->setEnabled(use_adsb_filter && use_v12);
    assert (min_sil_v2_edit_);
    min_sil_v2_edit_->setText(QString::number(eval_settings_.min_sil_v2_));
    min_sil_v2_edit_->setEnabled(use_adsb_filter && eval_settings_.use_min_sil_v2_ && use_v12);

    assert (use_max_sil_v2_check_);
    use_max_sil_v2_check_->setChecked(eval_settings_.use_max_sil_v2_);
    use_max_sil_v2_check_->setEnabled(use_adsb_filter && use_v12);
    assert (max_sil_v2_edit_);
    max_sil_v2_edit_->setText(QString::number(eval_settings_.max_sil_v2_));
    max_sil_v2_edit_->setEnabled(use_adsb_filter && eval_settings_.use_max_sil_v2_ && use_v12);
}
