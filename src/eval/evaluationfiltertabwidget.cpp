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

EvaluationFilterTabWidget::EvaluationFilterTabWidget(EvaluationManager& eval_man,
                                                     EvaluationManagerWidget& man_widget)
    : QWidget(nullptr), eval_man_(eval_man), man_widget_(man_widget)
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
    eval_man_.useLoadFilter(use_filter_check_->checkState() == Qt::Checked);

    update();
}

void EvaluationFilterTabWidget::toggleUseTimeSlot()
{
    assert (use_time_check_);
    eval_man_.useTimestampFilter(use_time_check_->checkState() == Qt::Checked);

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
    eval_man_.useASDBFilter(use_adsb_check_->checkState() == Qt::Checked);

    update();
}

void EvaluationFilterTabWidget::toggleUseV0Slot()
{
    assert (use_v0_check_);
    eval_man_.useV0(use_v0_check_->checkState() == Qt::Checked);

    update();
}

void EvaluationFilterTabWidget::toggleUseV1Slot()
{
    assert (use_v1_check_);
    eval_man_.useV1(use_v1_check_->checkState() == Qt::Checked);

    update();
}

void EvaluationFilterTabWidget::toggleUseV2Slot()
{
    assert (use_v2_check_);
    eval_man_.useV2(use_v2_check_->checkState() == Qt::Checked);

    update();
}

void EvaluationFilterTabWidget::toggleUseMinNUCPSlot()
{
    assert (use_min_nucp_check_);
    eval_man_.useMinNUCP(use_min_nucp_check_->checkState() == Qt::Checked);

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
        eval_man_.minNUCP(val);
}

void EvaluationFilterTabWidget::toggleUseMinNICSlot()
{
    assert (use_min_nic_check_);
    eval_man_.useMinNIC(use_min_nic_check_->checkState() == Qt::Checked);

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
        eval_man_.minNIC(val);
}

void EvaluationFilterTabWidget::toggleUseMinNACpSlot()
{
    assert (use_min_nacp_check_);
    eval_man_.useMinNACp(use_min_nacp_check_->checkState() == Qt::Checked);

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
        eval_man_.minNACp(val);
}

void EvaluationFilterTabWidget::toggleUseMinSILv1Slot()
{
    assert (use_min_sil_v1_check_);
    eval_man_.useMinSILv1(use_min_sil_v1_check_->checkState() == Qt::Checked);

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
        eval_man_.minSILv1(val);
}


void EvaluationFilterTabWidget::toggleUseMinSILv2Slot()
{
    assert (use_min_sil_v2_check_);
    eval_man_.useMinSILv2(use_min_sil_v2_check_->checkState() == Qt::Checked);

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
        eval_man_.minSILv2(val);
}


void EvaluationFilterTabWidget::toggleUseMaxNUCPSlot()
{
    assert (use_max_nucp_check_);
    eval_man_.useMaxNUCP(use_max_nucp_check_->checkState() == Qt::Checked);

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
        eval_man_.maxNUCP(val);
}

void EvaluationFilterTabWidget::toggleUseMaxNICSlot()
{
    assert (use_max_nic_check_);
    eval_man_.useMaxNIC(use_max_nic_check_->checkState() == Qt::Checked);

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
        eval_man_.maxNIC(val);
}

void EvaluationFilterTabWidget::toggleUseMaxNACpSlot()
{
    assert (use_max_nacp_check_);
    eval_man_.useMaxNACp(use_max_nacp_check_->checkState() == Qt::Checked);

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
        eval_man_.maxNACp(val);
}

void EvaluationFilterTabWidget::toggleUseMaxSILv1Slot()
{
    assert (use_max_sil_v1_check_);
    eval_man_.useMaxSILv1(use_max_sil_v1_check_->checkState() == Qt::Checked);

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
        eval_man_.maxSILv1(val);
}


void EvaluationFilterTabWidget::toggleUseMaxSILv2Slot()
{
    assert (use_max_sil_v2_check_);
    eval_man_.useMaxSILv2(use_max_sil_v2_check_->checkState() == Qt::Checked);

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
        eval_man_.maxSILv2(val);
}

void EvaluationFilterTabWidget::update()
{
    bool use_filter = eval_man_.useLoadFilter();

    assert (use_filter_check_);
    use_filter_check_->setChecked(eval_man_.useLoadFilter());

    // time filter
    assert (use_time_check_);
    use_time_check_->setChecked(eval_man_.useTimestampFilter());
    use_time_check_->setEnabled(use_filter);

    update_active_ = true;

    assert (time_begin_edit_);
    //time_begin_edit_->setText(String::timeStringFromDouble(eval_man_.loadTimestampBegin()).c_str());
    time_begin_edit_->setDateTime(QDateTime::fromString(Time::toString(eval_man_.loadTimestampBegin()).c_str(),
                                                 Time::QT_DATETIME_FORMAT.c_str()));
    time_begin_edit_->setEnabled(use_filter && eval_man_.useTimestampFilter());

    assert (time_end_edit_);
    //time_end_edit_->setText(String::timeStringFromDouble(eval_man_.loadTimestampEnd()).c_str());
    time_end_edit_->setDateTime(QDateTime::fromString(Time::toString(eval_man_.loadTimestampEnd()).c_str(),
                                                 Time::QT_DATETIME_FORMAT.c_str()));
    time_end_edit_->setEnabled(use_filter && eval_man_.useTimestampFilter());

    update_active_ = false;

    bool use_adsb_filter = use_filter && eval_man_.useASDBFilter();

    assert (use_adsb_check_);
    use_adsb_check_->setChecked(eval_man_.useASDBFilter());
    use_adsb_check_->setEnabled(use_filter);

    // v0
    assert (use_v0_check_);
    use_v0_check_->setChecked(eval_man_.useV0());
    use_v0_check_->setEnabled(use_adsb_filter);

    // nucp
    assert (use_min_nucp_check_);
    use_min_nucp_check_->setChecked(eval_man_.useMinNUCP());
    use_min_nucp_check_->setEnabled(use_adsb_filter && eval_man_.useV0());
    assert (min_nucp_edit_);
    min_nucp_edit_->setText(QString::number(eval_man_.minNUCP()));
    min_nucp_edit_->setEnabled(use_adsb_filter && eval_man_.useMinNUCP() && eval_man_.useV0());

    assert (use_max_nucp_check_);
    use_max_nucp_check_->setChecked(eval_man_.useMaxNUCP());
    use_max_nucp_check_->setEnabled(use_adsb_filter && eval_man_.useV0());
    assert (max_nucp_edit_);
    max_nucp_edit_->setText(QString::number(eval_man_.maxNUCP()));
    max_nucp_edit_->setEnabled(use_adsb_filter && eval_man_.useMaxNUCP() && eval_man_.useV0());

    // v1
    assert (use_v1_check_);
    use_v1_check_->setChecked(eval_man_.useV1());
    use_v1_check_->setEnabled(use_adsb_filter);
    assert (use_v2_check_);
    use_v2_check_->setChecked(eval_man_.useV2());
    use_v2_check_->setEnabled(use_adsb_filter);

    bool use_v12 = eval_man_.useV1() || eval_man_.useV2();

    // nic
    assert (use_min_nic_check_);
    use_min_nic_check_->setChecked(eval_man_.useMinNIC());
    use_min_nic_check_->setEnabled(use_adsb_filter && use_v12);
    assert (min_nic_edit_);
    min_nic_edit_->setText(QString::number(eval_man_.minNIC()));
    min_nic_edit_->setEnabled(use_adsb_filter && eval_man_.useMinNIC() && use_v12);

    assert (use_max_nic_check_);
    use_max_nic_check_->setChecked(eval_man_.useMaxNIC());
    use_max_nic_check_->setEnabled(use_adsb_filter && use_v12);
    assert (max_nic_edit_);
    max_nic_edit_->setText(QString::number(eval_man_.maxNIC()));
    max_nic_edit_->setEnabled(use_adsb_filter && eval_man_.useMaxNIC() && use_v12);

    // nacp
    assert (use_min_nacp_check_);
    use_min_nacp_check_->setChecked(eval_man_.useMinNACp());
    use_min_nacp_check_->setEnabled(use_adsb_filter && use_v12);
    assert (min_nacp_edit_);
    min_nacp_edit_->setText(QString::number(eval_man_.minNACp()));
    min_nacp_edit_->setEnabled(use_adsb_filter && eval_man_.useMinNACp() && use_v12);

    assert (use_max_nacp_check_);
    use_max_nacp_check_->setChecked(eval_man_.useMaxNACp());
    use_max_nacp_check_->setEnabled(use_adsb_filter && use_v12);
    assert (max_nacp_edit_);
    max_nacp_edit_->setText(QString::number(eval_man_.maxNACp()));
    max_nacp_edit_->setEnabled(use_adsb_filter && eval_man_.useMaxNACp() && use_v12);

    // sil v1
    assert (use_min_sil_v1_check_);
    use_min_sil_v1_check_->setChecked(eval_man_.useMinSILv1());
    use_min_sil_v1_check_->setEnabled(use_adsb_filter && use_v12);
    assert (min_sil_v1_edit_);
    min_sil_v1_edit_->setText(QString::number(eval_man_.minSILv1()));
    min_sil_v1_edit_->setEnabled(use_adsb_filter && eval_man_.useMinSILv1() && use_v12);

    assert (use_max_sil_v1_check_);
    use_max_sil_v1_check_->setChecked(eval_man_.useMaxSILv1());
    use_max_sil_v1_check_->setEnabled(use_adsb_filter && use_v12);
    assert (max_sil_v1_edit_);
    max_sil_v1_edit_->setText(QString::number(eval_man_.maxSILv1()));
    max_sil_v1_edit_->setEnabled(use_adsb_filter && eval_man_.useMaxSILv1() && use_v12);

    // sil v2
    assert (use_min_sil_v2_check_);
    use_min_sil_v2_check_->setChecked(eval_man_.useMinSILv2());
    use_min_sil_v2_check_->setEnabled(use_adsb_filter && use_v12);
    assert (min_sil_v2_edit_);
    min_sil_v2_edit_->setText(QString::number(eval_man_.minSILv2()));
    min_sil_v2_edit_->setEnabled(use_adsb_filter && eval_man_.useMinSILv2() && use_v12);

    assert (use_max_sil_v2_check_);
    use_max_sil_v2_check_->setChecked(eval_man_.useMaxSILv2());
    use_max_sil_v2_check_->setEnabled(use_adsb_filter && use_v12);
    assert (max_sil_v2_edit_);
    max_sil_v2_edit_->setText(QString::number(eval_man_.maxSILv2()));
    max_sil_v2_edit_->setEnabled(use_adsb_filter && eval_man_.useMaxSILv2() && use_v12);
}
