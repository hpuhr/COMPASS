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

#pragma once

#include "dbfilterwidget.h"
#include "adsbqualityfilter.h"

class QLabel;
class QLineEdit;
class QCheckBox;

/**
 */
class ADSBQualityFilterWidget : public DBFilterWidget
{
    Q_OBJECT

protected slots:
    void toggleUseV0Slot();
    void toggleUseV1Slot();
    void toggleUseV2Slot();

    void toggleUseMinNUCPSlot();
    void minNUCPEditedSlot (const QString& text);

    void toggleUseMinNICSlot();
    void minNICEditedSlot (const QString& text);

    void toggleUseMinNACpSlot();
    void minNACPEditedSlot (const QString& text);

    void toggleUseMinSILv1Slot();
    void minSILv1PEditedSlot (const QString& text);

    void toggleUseMinSILv2Slot();
    void minSILv2PEditedSlot (const QString& text);

    void toggleUseMaxNUCPSlot();
    void maxNUCPEditedSlot (const QString& text);

    void toggleUseMaxNICSlot();
    void maxNICEditedSlot (const QString& text);

    void toggleUseMaxNACpSlot();
    void maxNACPEditedSlot (const QString& text);

    void toggleUseMaxSILv1Slot();
    void maxSILv1PEditedSlot (const QString& text);

    void toggleUseMaxSILv2Slot();
    void maxSILv2PEditedSlot (const QString& text);

public:
    ADSBQualityFilterWidget(ADSBQualityFilter& filter);
    virtual ~ADSBQualityFilterWidget();

    virtual void update();

protected:
    ADSBQualityFilter& filter_;

    QCheckBox* use_v0_check_{nullptr};
    QCheckBox* use_v1_check_{nullptr};
    QCheckBox* use_v2_check_{nullptr};

    QCheckBox* use_min_nucp_check_{nullptr};
    QLineEdit* min_nucp_edit_{nullptr};

    QCheckBox* use_min_nic_check_{nullptr};
    QLineEdit* min_nic_edit_{nullptr};

    QCheckBox* use_min_nacp_check_{nullptr};
    QLineEdit* min_nacp_edit_{nullptr};

    QCheckBox* use_min_sil_v1_check_{nullptr};
    QLineEdit* min_sil_v1_edit_{nullptr};

    QCheckBox* use_min_sil_v2_check_{nullptr};
    QLineEdit* min_sil_v2_edit_{nullptr};

    QCheckBox* use_max_nucp_check_{nullptr};
    QLineEdit* max_nucp_edit_{nullptr};

    QCheckBox* use_max_nic_check_{nullptr};
    QLineEdit* max_nic_edit_{nullptr};

    QCheckBox* use_max_nacp_check_{nullptr};
    QLineEdit* max_nacp_edit_{nullptr};

    QCheckBox* use_max_sil_v1_check_{nullptr};
    QLineEdit* max_sil_v1_edit_{nullptr};

    QCheckBox* use_max_sil_v2_check_{nullptr};
    QLineEdit* max_sil_v2_edit_{nullptr};
};
