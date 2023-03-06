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

#include "eval/requirement/detection/detection.h"
#include "eval/results/detection/single.h"
#include "evaluationdata.h"
#include "evaluationmanager.h"
#include "evaluationdetail.h"
#include "logger.h"
#include "stringconv.h"
#include "sectorlayer.h"
#include "timeperiod.h"

#include "boost/date_time/posix_time/ptime.hpp"

using namespace std;
using namespace Utils;
using namespace boost::posix_time;

namespace EvaluationRequirement
{

Detection::Detection(
        const std::string& name, const std::string& short_name, const std::string& group_name,
        float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man,
        float update_interval_s, bool use_min_gap_length, float min_gap_length_s,
        bool use_max_gap_length, float max_gap_length_s, bool invert_prob,
        bool use_miss_tolerance, float miss_tolerance_s)
    : Base(name, short_name, group_name, prob, prob_check_type, eval_man), update_interval_s_(update_interval_s),
      use_min_gap_length_(use_min_gap_length), min_gap_length_s_(min_gap_length_s),
      use_max_gap_length_(use_max_gap_length), max_gap_length_s_(max_gap_length_s), invert_prob_(invert_prob),
      use_miss_tolerance_(use_miss_tolerance), miss_tolerance_s_(miss_tolerance_s)
{

}

float Detection::updateInterval() const
{
    return update_interval_s_;
}
bool Detection::useMinGapLength() const
{
    return use_min_gap_length_;
}

float Detection::minGapLength() const
{
    return min_gap_length_s_;
}

bool Detection::useMaxGapLength() const
{
    return use_max_gap_length_;
}

float Detection::maxGapLength() const
{
    return max_gap_length_s_;
}

bool Detection::useMissTolerance() const
{
    return use_miss_tolerance_;
}

float Detection::missTolerance() const
{
    return miss_tolerance_s_;
}

float Detection::missThreshold() const
{
    return use_miss_tolerance_ ? update_interval_s_+miss_tolerance_s_ : update_interval_s_;
}

std::shared_ptr<EvaluationRequirementResult::Single> Detection::evaluate (
        const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
        const SectorLayer& sector_layer)
{
    logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
           << " update_interval " << update_interval_s_ << " prob " << prob_
           << " use_miss_tolerance " << use_miss_tolerance_ << " miss_tolerance " << miss_tolerance_s_;

    time_duration max_ref_time_diff = Time::partialSeconds(eval_man_.maxRefTimeDiff());

    // create ref time periods
    TimePeriodCollection ref_periods;

    EvaluationTargetPosition ref_pos;
    ptime timestamp, last_ts;
    bool has_ground_bit;
    bool ground_bit_set;

    bool inside, was_inside;

    {
        const std::multimap<ptime, unsigned int>& ref_data = target_data.refData();
        bool first {true};

        for (auto& ref_it : ref_data)
        {
            timestamp = ref_it.first;
            was_inside = inside;

            // for ref
            tie (has_ground_bit, ground_bit_set) = target_data.refGroundBitForTime(timestamp);
            // for tst
            if (!ground_bit_set)
                tie (has_ground_bit, ground_bit_set) = target_data.tstGroundBitForTimeInterpolated(timestamp);

            inside = target_data.hasRefPosForTime(timestamp)
                    && sector_layer.isInside(target_data.refPosForTime(timestamp), has_ground_bit, ground_bit_set);

            if (first)
            {
                if (inside) // create time period
                    ref_periods.add({timestamp, timestamp});

                first = false;

                continue;
            }

            // not first, was_inside is valid

            if (was_inside)
            {
                if (inside)
                {
                    // extend last time period, if possible, or finish last and create new one
                    if (ref_periods.lastPeriod().isCloseToEnd(timestamp, max_ref_time_diff)) // 4.9
                        ref_periods.lastPeriod().extend(timestamp);
                    else
                        ref_periods.add({timestamp, timestamp});
                }
            }
            else if (inside) // was not inside and is now inside
                ref_periods.add({timestamp, timestamp}); // create new time period
        }
    }
    ref_periods.removeSmallPeriods(Time::partialSeconds(1));

    logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
           << " periods '" << ref_periods.print() << "'";

    timestamp = {};
    last_ts = {};

    // evaluate test data
    const std::multimap<ptime, unsigned int>& tst_data = target_data.tstData();

    int sum_uis = ref_periods.getUIs(update_interval_s_);

    float t_diff;

    int sum_missed_uis {0};
    //int max_gap_uis {0};

    //int no_ref_uis {0};

    bool is_inside {false}, was_outside {false};
    //pair<EvaluationTargetPosition, bool> ret_pos;
    bool ok;

    EvaluationRequirementResult::Single::EvaluationDetails details;

    EvaluationTargetPosition pos_current;

    unsigned int tst_data_size = tst_data.size();

    if (!tst_data_size)
    {
        for (auto& period_it : ref_periods)
        {
            last_ts = period_it.begin();
            timestamp = period_it.end();

            t_diff = Time::partialSeconds(timestamp - last_ts);

            if (isMiss(t_diff))
            {
                sum_missed_uis += getNumMisses(t_diff);

                assert (target_data.hasRefPosForTime(timestamp));
                pos_current = target_data.refPosForTime(timestamp);

                logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                       << " miss of " << String::timeStringFromDouble(t_diff)
                       << " uis " << getNumMisses(t_diff)
                       << " at [" << Time::toString(last_ts)
                       << "," << Time::toString(timestamp)
                       << "] sum_missed_uis " << sum_missed_uis;

                string comment = "Miss detected (DToD > "
                        +String::doubleToStringPrecision(missThreshold(), 2)
                        +"), last was "+Time::toString(last_ts);

                DetectionDetail detail {
                    timestamp, t_diff, true,
                            pos_current, true,
                            sum_missed_uis, comment};

                assert (target_data.hasRefPosForTime(last_ts));
                detail.pos_last_ = target_data.refPosForTime(last_ts);
                detail.has_last_position_ = true;

                details.push_back(detail);
            }
        }

        return make_shared<EvaluationRequirementResult::SingleDetection>(
                    "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                    eval_man_, sum_uis, sum_missed_uis, ref_periods, details);
    }


    // collect test times in ref periods
    map<unsigned int, ptime> period_last_tst_times; // period number -> lst tst times
    set<unsigned int> finalized_periods;

    bool is_inside_ref_time_period {false};//, was_inside_ref_time_period {false};
    unsigned int period_index {0};

    set<unsigned int> finished_periods;

    int period_max_index_before_ts;

    string comment;

    bool skip_no_data_details = eval_man_.reportSkipNoDataDetails();

    for (auto tst_it=tst_data.begin(); tst_it != tst_data.end(); ++tst_it)
    {
        comment = "";

        last_ts = timestamp;
        timestamp = tst_it->first;
        pos_current = target_data.tstPosForTime(timestamp);

        logdbg << "EvaluationRequirementDetection '" << name_
               << "': evaluate: utn " << target_data.utn_ << " tod " << timestamp;

        //was_inside_ref_time_period = is_inside_ref_time_period;
        is_inside_ref_time_period = ref_periods.isInside(timestamp);

        // check if previous periods need to be finalized
        period_max_index_before_ts = ref_periods.getPeriodMaxIndexBefore(timestamp);

        if (period_max_index_before_ts != -1)
        {
            for (unsigned int period_cnt=0; period_cnt <= period_max_index_before_ts; ++period_cnt)
            {
                if (!finished_periods.count(period_cnt)) // previous not finalized
                {
                    ptime last_period_ts = ref_periods.period(period_cnt).begin();
                    bool tst_time_found = false;
                    ptime last_period_ts_end = ref_periods.period(period_cnt).end();

                    if (period_last_tst_times.count(period_cnt))
                    {
                        last_period_ts = period_last_tst_times.at(period_cnt);
                        tst_time_found = true;
                    }

                    assert (last_period_ts_end >= last_period_ts);

                    t_diff = Time::partialSeconds(last_period_ts_end - last_period_ts);

                    assert (target_data.hasRefPosForTime(last_period_ts_end));
                    pos_current = target_data.refPosForTime(last_period_ts_end);

                    if (isMiss(t_diff))
                    {
                        sum_missed_uis += getNumMisses(t_diff);

                        logdbg << "EvaluationRequirementDetection '" << name_
                               << "': evaluate: utn " << target_data.utn_
                               << " miss of " << String::timeStringFromDouble(t_diff)
                               << " uis " << getNumMisses(t_diff)
                               << " at [" << Time::toString(last_period_ts)
                               << "," << Time::toString(ref_periods.period(period_cnt).end())
                               << "] missed_uis " << sum_missed_uis;

                        comment = "Miss detected in previous period "+to_string(period_cnt)
                                +" (DToD > " +String::doubleToStringPrecision(missThreshold(), 2)
                                +"), between ["+Time::toString(last_period_ts)+", "
                                +Time::toString(ref_periods.period(period_cnt).end())+"]\n";

                        DetectionDetail detail{timestamp, t_diff, true,
                                    pos_current, false,
                                    sum_missed_uis, comment};

                        if (tst_time_found)
                        {
                            assert (target_data.hasTstPosForTime(last_period_ts));
                            detail.pos_last_ = target_data.tstPosForTime(last_period_ts);
                            detail.has_last_position_ = true;
                        }
                        else
                        {
                            assert (target_data.hasRefPosForTime(last_period_ts));
                            detail.pos_last_ = target_data.refPosForTime(last_period_ts);
                            detail.has_last_position_ = true;
                        }

                        details.push_back(detail);
                    }
                    else
                    {
                        comment = "Previous period "+to_string(period_cnt)
                                +" OK (DToD <= "+String::doubleToStringPrecision(missThreshold(), 2)+")\n";

                        DetectionDetail detail{timestamp, t_diff, false,
                                    pos_current, false,
                                    sum_missed_uis, comment};

                        details.push_back(detail);
                    }

                    finished_periods.insert(period_cnt);
                }
            }
        }

        if (!is_inside_ref_time_period)
        {
            if (!skip_no_data_details)
                details.push_back(
                            {timestamp, {}, false,
                             pos_current, is_inside_ref_time_period,
                             sum_missed_uis, "Outside of reference time periods"});

            // TODO undetected previous miss possible
            continue;
        }

        // is inside period
        period_index = ref_periods.getPeriodIndex(timestamp);

        tie(ref_pos, ok) = target_data.interpolatedRefPosForTime(timestamp, max_ref_time_diff);

//        ref_pos = ret_pos.first;
//        ok = ret_pos.second;
        //assert (ok); // must be since inside ref time interval

        if (!ok) // only happens if test time is exact beginning of reference interval
        {
            if (!skip_no_data_details)
                details.push_back(
                            {timestamp, {}, false,
                             pos_current, is_inside_ref_time_period,
                             sum_missed_uis, "At exact beginning of reference time period"});

            continue;
        }

        has_ground_bit = target_data.hasTstGroundBitForTime(timestamp);

        if (has_ground_bit)
            ground_bit_set = target_data.tstGroundBitForTime(timestamp);
        else
            ground_bit_set = false;

        if (!ground_bit_set)
            tie(has_ground_bit, ground_bit_set) = target_data.interpolatedRefGroundBitForTime(timestamp, seconds(15));

        is_inside = sector_layer.isInside(ref_pos, has_ground_bit, ground_bit_set);

        if (!is_inside)
        {
            logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                   << " outside";

            if (!skip_no_data_details)
                details.push_back(
                            {timestamp, {}, false,
                             pos_current, is_inside_ref_time_period,
                             sum_missed_uis, "Outside sector"});

            was_outside = true;

            //++num_pos_outside;
            continue;
        }

        if (!period_last_tst_times.count(period_index) || was_outside) // first in period or was outside
        {
            logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                   << " first in period " << period_index <<" (" << period_last_tst_times.count(period_index)
                   << ") or was_outside " << was_outside;

            if (was_outside)
            {
                details.push_back(
                            {timestamp, {}, false,
                             pos_current, is_inside_ref_time_period,
                             sum_missed_uis, "First target report after outside sector"});
            }
            else // first in period
            {
                details.push_back(
                            {timestamp, {}, false,
                             pos_current, is_inside_ref_time_period,
                             sum_missed_uis, "First target report in period "+to_string(period_index)});

                // check if begin time in period is miss

                t_diff = Time::partialSeconds(timestamp - ref_periods.period(period_index).begin());

                if (isMiss(t_diff))
                {
                    sum_missed_uis += getNumMisses(t_diff);

                    logdbg << "EvaluationRequirementDetection '" << name_
                           << "': evaluate: utn " << target_data.utn_
                           << " miss of " << String::timeStringFromDouble(t_diff)
                           << " uis " << getNumMisses(t_diff)
                           << " at [" << Time::toString(ref_periods.period(period_index).begin())
                           << "," << Time::toString(timestamp)
                           << "] missed_uis " << sum_missed_uis;

                    comment = "Miss detected in current period "+to_string(period_index)
                            +" (DToD > " +String::doubleToStringPrecision(missThreshold(), 2)
                            +"), between ["+Time::toString(ref_periods.period(period_index).begin())
                            +", "+Time::toString(timestamp)+"]\n";

                    DetectionDetail detail{timestamp, t_diff, true,
                                target_data.tstPosForTime(timestamp), is_inside_ref_time_period,
                                sum_missed_uis, comment};

                    assert (target_data.hasRefPosForTime(ref_periods.period(period_index).begin()));
                    detail.pos_last_ = target_data.refPosForTime(ref_periods.period(period_index).begin());
                    detail.has_last_position_ = true;

                    details.push_back(detail);
                }
            }


            was_outside = false;
            period_last_tst_times[period_index] = timestamp;

            continue;
        }

        assert (timestamp >= last_ts);
        t_diff = Time::partialSeconds(timestamp - last_ts);

        logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
               << " ts " << Time::toString(timestamp) << " d_tod " << String::timeStringFromDouble(t_diff);

        if (isMiss(t_diff))
        {
            sum_missed_uis += getNumMisses(t_diff);

            logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                   << " miss of " << String::timeStringFromDouble(t_diff)
                   << " uis " << getNumMisses(t_diff)
                   << " at [" << Time::toString(last_ts)
                   << "," << Time::toString(timestamp)
                   << "] sum_missed_uis " << sum_missed_uis;

            string comment = "Miss detected (DToD > "
                    +String::doubleToStringPrecision(missThreshold(), 2)
                    +"), last was "+Time::toString(last_ts);

            DetectionDetail detail {
                timestamp, t_diff, true,
                        pos_current, is_inside_ref_time_period,
                        sum_missed_uis, comment};

            detail.pos_last_ = target_data.tstPosForTime(last_ts);
            detail.has_last_position_ = true;

            details.push_back(detail);
        }
        else
        {
            logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                   << " update ok";

            details.push_back(
                        {timestamp, t_diff, false,
                         pos_current, is_inside_ref_time_period,
                         sum_missed_uis, "OK (DToD <= "+String::doubleToStringPrecision(missThreshold(), 2)+")"});
        }


        period_last_tst_times[period_index] = timestamp;
    }

    // finalize unfinished periods
    logdbg << "EvaluationRequirementDetection '" << name_
           << "': evaluate: utn " << target_data.utn_ << " finalizing " << ref_periods.size()
           << " unfinished periods";

    for (unsigned int period_cnt=0; period_cnt < ref_periods.size(); ++period_cnt)
    {
        if (!finished_periods.count(period_cnt)) // previous not finalized
        {
            ptime last_period_tod = ref_periods.period(period_cnt).begin();
            bool tst_time_found = false;
            ptime last_period_end = ref_periods.period(period_cnt).end();

            if (period_last_tst_times.count(period_cnt))
            {
                last_period_tod = period_last_tst_times.at(period_cnt);
                tst_time_found = true;
            }

            assert (last_period_end >= last_period_tod);

            t_diff = Time::partialSeconds(last_period_end - last_period_tod);

            assert (target_data.hasRefPosForTime(last_period_end));
            pos_current = target_data.refPosForTime(last_period_end);

            if (isMiss(t_diff))
            {
                sum_missed_uis += getNumMisses(t_diff); // TODO substract miss_tolerance_s_?

                logdbg << "EvaluationRequirementDetection '" << name_
                       << "': evaluate: utn " << target_data.utn_
                       << " miss of " << String::timeStringFromDouble(t_diff)
                       << " uis " << getNumMisses(t_diff)
                       << " at [" << Time::toString(last_period_tod) << ","
                       << Time::toString(ref_periods.period(period_cnt).end())
                       << "] missed_uis " << sum_missed_uis;

                comment = "Miss detected in previous period "+to_string(period_cnt)
                        +" (DToD > " +String::doubleToStringPrecision(missThreshold(), 2)
                        +"), between ["+Time::toString(last_period_tod)+", "
                        +Time::toString(ref_periods.period(period_cnt).end())+"]\n";

                DetectionDetail detail{timestamp, t_diff, true,
                            pos_current, false,
                            sum_missed_uis, comment};

                if (tst_time_found)
                {
                    assert (target_data.hasTstPosForTime(last_period_tod));
                    detail.pos_last_ = target_data.tstPosForTime(last_period_tod);
                    detail.has_last_position_ = true;
                }
                else
                {
                    assert (target_data.hasRefPosForTime(last_period_tod));
                    detail.pos_last_ = target_data.refPosForTime(last_period_tod);
                    detail.has_last_position_ = true;
                }

                details.push_back(detail);
            }
            else
            {
                comment = "Previous period "+to_string(period_cnt)
                        +" OK (DToD <= "+String::doubleToStringPrecision(missThreshold(), 2)+")\n";

                DetectionDetail detail{timestamp, t_diff, false,
                            pos_current, false,
                            sum_missed_uis, comment};

                details.push_back(detail);
            }

            finished_periods.insert(period_cnt);
        }
    }

    logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
           << " sum_uis " << sum_uis;

//    if (sum_uis)
//    {
//        //assert (sum_uis >= max_gap_uis+no_ref_uis);
//        assert (sum_missed_uis <= sum_uis);

//        float pd = 1.0 - ((float)sum_missed_uis/(float)(sum_uis)); // -max_gap_uis-no_ref_uis

//        logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
//               << " pd " << String::percentToString(100.0 * pd) << " passed " << (pd >= minimum_probability_);
//    }
//    else
//        logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
//               << " no data for pd";

    return make_shared<EvaluationRequirementResult::SingleDetection>(
                "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                eval_man_, sum_uis, sum_missed_uis, ref_periods, details);
}

bool Detection::invertProb() const
{
    return invert_prob_;
}

bool Detection::isMiss (float d_tod)
{
    if (use_miss_tolerance_)
        d_tod -= miss_tolerance_s_;

    if (use_min_gap_length_ && d_tod < min_gap_length_s_) // supress gaps smaller as min gap length
        return false;

    if (use_max_gap_length_ && d_tod > max_gap_length_s_) // supress gaps larger as max gap length
        return false;

    return d_tod > update_interval_s_;
}

unsigned int Detection::getNumMisses(float d_tod)
{
    assert (isMiss(d_tod));

    if (use_miss_tolerance_)
        d_tod -= miss_tolerance_s_;

    return floor(d_tod/update_interval_s_);
}

}
