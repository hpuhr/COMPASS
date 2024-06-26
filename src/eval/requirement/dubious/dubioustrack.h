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

#include "eval/requirement/base/probabilitybase.h"

#include <QVariant>

#include <boost/date_time/posix_time/posix_time_duration.hpp>

#include <cassert>

namespace EvaluationRequirement
{

class DubiousTrack : public ProbabilityBase
{
public:
    DubiousTrack(const std::string& name, 
                 const std::string& short_name, 
                 const std::string& group_name,
                 bool eval_only_single_ds_id, 
                 unsigned int single_ds_id,
                 float minimum_comparison_time, 
                 float maximum_comparison_time,
                 bool mark_primary_only, 
                 bool use_min_updates, 
                 unsigned int min_updates,
                 bool use_min_duration, 
                 float min_duration,
                 bool use_max_groundspeed, 
                 float max_groundspeed_kts,
                 bool use_max_acceleration, 
                 float max_acceleration,
                 bool use_max_turnrate, 
                 float max_turnrate,
                 bool use_rocd, 
                 float max_rocd, 
                 float dubious_prob,
                 float prob, 
                 COMPARISON_TYPE prob_check_type, 
                 EvaluationManager& eval_man);

    virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (const EvaluationTargetData& target_data, 
                                                                           std::shared_ptr<Base> instance,
                                                                           const SectorLayer& sector_layer) override;

    std::string probabilityNameShort() const override final { return "PDT"; }
    std::string probabilityName() const override final { return "Probability of dubious track"; }

    bool markPrimaryOnly() const;

    bool useMinUpdates() const;
    unsigned int minUpdates() const;

    bool useMinDuration() const;
    float minDuration() const;

    bool useMaxAcceleration() const;
    float maxAcceleration() const;

    bool useMaxTurnrate() const;
    float maxTurnrate() const;

    bool useROCD() const;
    float maxROCD() const;

protected:
    bool eval_only_single_ds_id_ {false};
    unsigned int single_ds_id_ {0};

    float minimum_comparison_time_ {1.0};
    float maximum_comparison_time_ {30.0};

    bool mark_primary_only_ {true};

    bool use_min_updates_ {true};
    unsigned int min_updates_ {10};

    bool use_min_duration_ {true};
    boost::posix_time::time_duration min_duration_ {boost::posix_time::seconds(30)};

    bool use_max_groundspeed_ {true};
    float max_groundspeed_kts_ {1333.0};

    bool use_max_acceleration_ {true};
    float max_acceleration_ {29.43}; // m/s^2

    bool use_max_turnrate_ {true};
    float max_turnrate_ {30.0}; // deg/s

    bool use_rocd_ {true};
    float max_rocd_ {1000.0}; // ft/s

    float dubious_prob_ {0.05};
};

}
