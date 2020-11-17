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

#include "adsbqualityfilterwidget.h"
#include "logger.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>

ADSBQualityFilterWidget::ADSBQualityFilterWidget(ADSBQualityFilter& filter, const std::string& class_id,
                                                 const std::string& instance_id)
    : DBFilterWidget(class_id, instance_id, filter), filter_(filter)
{
    QFormLayout* layout = new QFormLayout();
    //layout->setContentsMargins(0, 0, 0, 0);
    //layout->setSpacing(0);
    child_layout_->addLayout(layout);

    // v0
    use_v0_check_ = new QCheckBox ();
    connect(use_v0_check_, &QCheckBox::clicked, this, &ADSBQualityFilterWidget::toggleUseV0Slot);
    layout->addRow("Use v0", use_v0_check_);

    // nucp
    use_min_nucp_check_ = new QCheckBox ();
    connect(use_min_nucp_check_, &QCheckBox::clicked, this, &ADSBQualityFilterWidget::toggleUseMinNUCPSlot);
    layout->addRow("Use Min NUCp", use_min_nucp_check_);

    min_nucp_edit_ = new QLineEdit();
    connect(min_nucp_edit_, &QLineEdit::textEdited, this, &ADSBQualityFilterWidget::minNUCPEditedSlot);
    layout->addRow("Min NUCp", min_nucp_edit_);

    // others
    use_v1_check_ = new QCheckBox ();
    connect(use_v1_check_, &QCheckBox::clicked, this, &ADSBQualityFilterWidget::toggleUseV1Slot);
    layout->addRow("Use v1", use_v1_check_);

    use_v2_check_ = new QCheckBox ();
    connect(use_v2_check_, &QCheckBox::clicked, this, &ADSBQualityFilterWidget::toggleUseV2Slot);
    layout->addRow("Use v2", use_v2_check_);

    // nic
    use_min_nic_check_ = new QCheckBox ();
    connect(use_min_nic_check_, &QCheckBox::clicked, this, &ADSBQualityFilterWidget::toggleUseMinNICSlot);
    layout->addRow("Use Min NIC", use_min_nic_check_);

    min_nic_edit_ = new QLineEdit();
    connect(min_nic_edit_, &QLineEdit::textEdited, this, &ADSBQualityFilterWidget::minNICEditedSlot);
    layout->addRow("Min NIC", min_nic_edit_);

    // nacp
    use_min_nacp_check_ = new QCheckBox ();
    connect(use_min_nacp_check_, &QCheckBox::clicked, this, &ADSBQualityFilterWidget::toggleUseMinNACpSlot);
    layout->addRow("Use Min NACp", use_min_nacp_check_);

    min_nacp_edit_ = new QLineEdit();
    connect(min_nacp_edit_, &QLineEdit::textEdited, this, &ADSBQualityFilterWidget::minNACPEditedSlot);
    layout->addRow("Min NACp", min_nacp_edit_);

    // sil
    use_min_sil_v1_check_ = new QCheckBox ();
    connect(use_min_sil_v1_check_, &QCheckBox::clicked, this, &ADSBQualityFilterWidget::toggleUseMinSILv1Slot);
    layout->addRow("Use Min SIL v1", use_min_sil_v1_check_);

    min_sil_v1_edit_ = new QLineEdit();
    connect(min_sil_v1_edit_, &QLineEdit::textEdited, this, &ADSBQualityFilterWidget::minSILv1PEditedSlot);
    layout->addRow("Min SIL v1", min_sil_v1_edit_);

    use_min_sil_v2_check_ = new QCheckBox ();
    connect(use_min_sil_v2_check_, &QCheckBox::clicked, this, &ADSBQualityFilterWidget::toggleUseMinSILv2Slot);
    layout->addRow("Use Min SIL v2", use_min_sil_v2_check_);

    min_sil_v2_edit_ = new QLineEdit();
    connect(min_sil_v2_edit_, &QLineEdit::textEdited, this, &ADSBQualityFilterWidget::minSILv2PEditedSlot);
    layout->addRow("Min SIL v2", min_sil_v2_edit_);

    update();
}

ADSBQualityFilterWidget::~ADSBQualityFilterWidget()
{

}

void ADSBQualityFilterWidget::toggleUseV0Slot()
{
    assert (use_v0_check_);
    filter_.useV0(use_v0_check_->checkState() == Qt::Checked);
}

void ADSBQualityFilterWidget::toggleUseV1Slot()
{
    assert (use_v1_check_);
    filter_.useV1(use_v1_check_->checkState() == Qt::Checked);
}

void ADSBQualityFilterWidget::toggleUseV2Slot()
{
    assert (use_v2_check_);
    filter_.useV2(use_v2_check_->checkState() == Qt::Checked);
}

void ADSBQualityFilterWidget::toggleUseMinNUCPSlot()
{
    assert (use_min_nucp_check_);
    filter_.useMinNUCP(use_min_nucp_check_->checkState() == Qt::Checked);
}

void ADSBQualityFilterWidget::minNUCPEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "ADSBQualityFilterWidget: minNUCPEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        filter_.minNUCP(val);
}

void ADSBQualityFilterWidget::toggleUseMinNICSlot()
{
    assert (use_min_nic_check_);
    filter_.useMinNIC(use_min_nic_check_->checkState() == Qt::Checked);
}

void ADSBQualityFilterWidget::minNICEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "ADSBQualityFilterWidget: minNICEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        filter_.minNIC(val);
}

void ADSBQualityFilterWidget::toggleUseMinNACpSlot()
{
    assert (use_min_nacp_check_);
    filter_.useMinNACp(use_min_nacp_check_->checkState() == Qt::Checked);
}

void ADSBQualityFilterWidget::minNACPEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "ADSBQualityFilterWidget: minNACPEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        filter_.minNACp(val);
}

void ADSBQualityFilterWidget::toggleUseMinSILv1Slot()
{
    assert (use_min_sil_v1_check_);
    filter_.useMinSILv1(use_min_sil_v1_check_->checkState() == Qt::Checked);
}

void ADSBQualityFilterWidget::minSILv1PEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "ADSBQualityFilterWidget: minSILv1PEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        filter_.minSILv1(val);
}


void ADSBQualityFilterWidget::toggleUseMinSILv2Slot()
{
    assert (use_min_sil_v2_check_);
    filter_.useMinSILv2(use_min_sil_v2_check_->checkState() == Qt::Checked);
}

void ADSBQualityFilterWidget::minSILv2PEditedSlot (const QString& text)
{
    unsigned int val;
    bool ok;

    val = text.toUInt(&ok);

    if (!ok)
        logwrn << "ADSBQualityFilterWidget: minSILv2PEditedSlot: unable to parse value '" << text.toStdString() << "'";
    else
        filter_.minSILv2(val);
}


void ADSBQualityFilterWidget::update()

{
    DBFilterWidget::update();

    assert (use_v0_check_);
    use_v0_check_->setChecked(filter_.useV0());

    assert (use_min_nucp_check_);
    use_min_nucp_check_->setChecked(filter_.useMinNUCP());
    assert (min_nucp_edit_);
    min_nucp_edit_->setText(QString::number(filter_.minNUCP()));

    assert (use_v1_check_);
    use_v1_check_->setChecked(filter_.useV1());
    assert (use_v2_check_);
    use_v2_check_->setChecked(filter_.useV2());

    assert (use_min_nic_check_);
    use_min_nic_check_->setChecked(filter_.useMinNIC());
    assert (min_nic_edit_);
    min_nic_edit_->setText(QString::number(filter_.minNIC()));

    assert (use_min_nacp_check_);
    use_min_nacp_check_->setChecked(filter_.useMinNACp());
    assert (min_nacp_edit_);
    min_nacp_edit_->setText(QString::number(filter_.minNACp()));

    assert (use_min_sil_v1_check_);
    use_min_sil_v1_check_->setChecked(filter_.useMinSILv1());
    assert (min_sil_v1_edit_);
    min_sil_v1_edit_->setText(QString::number(filter_.minSILv1()));

    assert (use_min_sil_v2_check_);
    use_min_sil_v2_check_->setChecked(filter_.useMinSILv2());
    assert (min_sil_v2_edit_);
    min_sil_v2_edit_->setText(QString::number(filter_.minSILv2()));

}


