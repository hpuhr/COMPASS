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

#ifndef EVALUATIONREQUIREMENTDUBIOUSTRACK_H
#define EVALUATIONREQUIREMENTDUBIOUSTRACK_H

#include "eval/requirement/base/base.h"
#include "evaluationtargetposition.h"

#include <QVariant>

#include <cassert>

namespace EvaluationRequirement
{

class DubiousTrackUpdateDetail
{
public:
    DubiousTrackUpdateDetail(float tod, EvaluationTargetPosition pos)
        : tod_(tod), pos_(pos)
    {
    }

    float tod_ {0};
    EvaluationTargetPosition pos_;
    std::map<std::string, std::string> dubious_comments_;

    std::string dubiousReasonsString()
    {
        if (!dubious_comments_.size())
            return "OK";

        std::string str;

        for (auto& reas_it : dubious_comments_)
        {
            if (str.size())
                str += ", ";

            str += reas_it.first;
            if (reas_it.second.size())
                str += "("+reas_it.second+")";
        }

        if (!str.size())
            return "OK";
        else
            return str;
    }
};

class DubiousTrackDetail
{
public:
    DubiousTrackDetail(unsigned int track_num, float tod_begin)
        : track_num_(track_num), tod_begin_(tod_begin)
    {
        tod_end_ = tod_begin_;
    }

    bool first_inside_ {true};

    unsigned int track_num_ {0};
    float tod_begin_ {0};
    float tod_end_ {0};
    float duration_ {0};
    //std::vector<float> tods_inside_;

    unsigned int num_pos_inside_{0};
    unsigned int num_pos_inside_dubious_{0};

    //std::map<float, std::map<std::string, std::string>> tods_inside_dubious_; // tod -> type -> comment
    std::vector<DubiousTrackUpdateDetail> updates_; // only for inside, also not dubious

    bool has_mode_ac_ {false};
    bool has_mode_s_ {false};

    bool left_sector_ {false};

    bool is_dubious_ {false};

    std::map<std::string, std::string> dubious_reasons_; // type -> comment

    EvaluationTargetPosition pos_begin_;
    EvaluationTargetPosition pos_last_;

    std::string dubiousReasonsString()
    {
        std::string str;

        for (auto& reas_it : dubious_reasons_)
        {
            if (str.size())
                str += ", ";

            str += reas_it.first;
            if (reas_it.second.size())
                str += "("+reas_it.second+")";
        }

        if (!str.size())
            return "OK";
        else
            return str;
    }

    unsigned int getNumUpdatesDubious()
    {
        unsigned int cnt = 0;

        for (auto& update : updates_)
            if (update.dubious_comments_.size())
                ++cnt;

        return cnt;
    }
};


class DubiousTrack : public Base
{
public:
    DubiousTrack(const std::string& name, const std::string& short_name, const std::string& group_name,
                 bool eval_only_single_ds_id, unsigned int single_ds_id,
                 float minimum_comparison_time, float maximum_comparison_time,
                 bool mark_primary_only, bool use_min_updates, unsigned int min_updates,
                 bool use_min_duration, float min_duration,
                 bool use_max_groundspeed, float max_groundspeed_kts,
                 bool use_max_acceleration, float max_acceleration,
                 bool use_max_turnrate, float max_turnrate,
                 bool use_rocd, float max_rocd, float dubious_prob,
                 float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man);

    virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer) override;

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
    float min_duration_ {30.0};

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

#endif // EVALUATIONREQUIREMENTDUBIOUSTRACK_H
