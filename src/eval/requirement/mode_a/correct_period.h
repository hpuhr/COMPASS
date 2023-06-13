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

#include "eval/requirement/base/intervalbase.h"
#include "eval/requirement/base/intervalbaseconfig.h"
#include "eval/requirement/base/intervalbaseconfigwidget.h"

namespace EvaluationRequirement
{

/**
*/
class ModeACorrectPeriod : public IntervalBase
{
public:
    ModeACorrectPeriod(
            const std::string& name, 
            const std::string& short_name, 
            const std::string& group_name,
            float prob, 
            COMPARISON_TYPE prob_check_type,
            EvaluationManager& eval_man,
            float update_interval_s, 
            bool  use_miss_tolerance,
            float miss_tolerance_s);

    static std::string probabilityName();
    static std::string probabilityDescription();

protected:
    virtual Validity isValid(const dbContent::TargetReport::DataID& data_id,
                             const EvaluationTargetData& target_data,
                             const SectorLayer& sector_layer,
                             const boost::posix_time::time_duration& max_ref_time_diff) const override;
    virtual std::shared_ptr<EvaluationRequirementResult::Single> createResult(const std::string& result_id,
                                                                              std::shared_ptr<Base> instance, 
                                                                              const EvaluationTargetData& target_data,
                                                                              const SectorLayer& sector_layer, 
                                                                              const std::vector<EvaluationDetail>& details,
                                                                              const TimePeriodCollection& periods,
                                                                              unsigned int sum_uis,
                                                                              unsigned int misses_total) override;
};

/**
*/
class ModeACorrectPeriodConfig : public IntervalBaseConfig
{
public:
    ModeACorrectPeriodConfig(const std::string& class_id, 
                             const std::string& instance_id,
                             Group& group, 
                             EvaluationStandard& standard,
                             EvaluationManager& eval_man);
    virtual ~ModeACorrectPeriodConfig() = default;

    std::shared_ptr<Base> createRequirement() override;

protected:
    virtual BaseConfigWidget* createWidget_impl() override;
    virtual std::string probabilityDescription() const override;
};

/**
*/
class ModeACorrectPeriodConfigWidget : public IntervalBaseConfigWidget
{
public:
    ModeACorrectPeriodConfigWidget(ModeACorrectPeriodConfig& cfg);
    virtual ~ModeACorrectPeriodConfigWidget() = default;

protected:
    ModeACorrectPeriodConfig& config();
};

}
