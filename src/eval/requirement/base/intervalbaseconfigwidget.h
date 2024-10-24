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

#include "eval/requirement/base/probabilitybaseconfigwidget.h"

class QLineEdit;
class QCheckBox;

namespace EvaluationRequirement 
{

class IntervalBaseConfig;

/**
*/
class IntervalBaseConfigWidget : public ProbabilityBaseConfigWidget
{
    Q_OBJECT
public:
    IntervalBaseConfigWidget(IntervalBaseConfig& cfg);
    virtual ~IntervalBaseConfigWidget() = default;

public slots:
    void updateIntervalEditSlot(QString value);

    void toggleUseMinGapLengthSlot();
    void minGapLengthEditSlot(QString value);

    void toggleUseMaxGapLengthSlot();
    void maxGapLengthEditSlot(QString value);

    void toggleUseMissToleranceSlot();
    void missToleranceEditSlot(QString value);

    void toggleHoldForAnyTargetSlot();

protected:
    void configure(uint32_t flags);
    void updateActive();

    IntervalBaseConfig& config();

    QLineEdit* update_interval_edit_      = nullptr;

    QCheckBox* use_min_gap_length_check_  = nullptr;
    QLineEdit* min_gap_length_edit_       = nullptr;

    QCheckBox* use_max_gap_length_check_  = nullptr;
    QLineEdit* max_gap_length_edit_       = nullptr;

    QCheckBox* use_miss_tolerance_check_  = nullptr;
    QLineEdit* miss_tolerance_edit_       = nullptr;

    QCheckBox* hold_for_any_target_check_ = nullptr;
};

} // namespace EvaluationRequirement
