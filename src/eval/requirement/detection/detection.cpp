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
#include "logger.h"
#include "stringconv.h"
#include "sectorlayer.h"
#include "timeperiod.h"

using namespace std;
using namespace Utils;

namespace EvaluationRequirement
{

    Detection::Detection(
            const std::string& name, const std::string& short_name, const std::string& group_name,
            EvaluationManager& eval_man,
            float update_interval_s, float max_ref_time_diff, float minimum_probability,
            bool use_miss_tolerance, float miss_tolerance_s)
        : Base(name, short_name, group_name, eval_man), update_interval_s_(update_interval_s),
          max_ref_time_diff_(max_ref_time_diff), minimum_probability_(minimum_probability),
          use_miss_tolerance_(use_miss_tolerance),
          miss_tolerance_s_(miss_tolerance_s)
    {

    }

    float Detection::updateInterval() const
    {
        return update_interval_s_;
    }

    float Detection::maxRefTimeDiff() const
    {
        return max_ref_time_diff_;
    }

    float Detection::minimumProbability() const
    {
        return minimum_probability_;
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
               << " update_interval " << update_interval_s_ << " minimum_probability " << minimum_probability_
               << " use_miss_tolerance " << use_miss_tolerance_ << " miss_tolerance " << miss_tolerance_s_;

        // create ref time periods
        TimePeriodCollection ref_periods;

        EvaluationTargetPosition ref_pos;
        float tod{0}, last_tod{0};
        bool has_ground_bit;
        bool ground_bit_set;

        bool inside, was_inside;

        {
            const std::multimap<float, unsigned int>& ref_data = target_data.refData();
            bool first {true};

            for (auto& ref_it : ref_data)
            {
                tod = ref_it.first;
                was_inside = inside;

                // for ref
                tie (has_ground_bit, ground_bit_set) = target_data.tstGroundBitForTimeInterpolated(tod);

                inside = target_data.hasRefPosForTime(tod)
                        && sector_layer.isInside(target_data.refPosForTime(tod), has_ground_bit, ground_bit_set);

                if (first)
                {
                    if (inside) // create time period
                        ref_periods.add({tod, tod});

                    first = false;

                    continue;
                }

                // not first, was_inside is valid

                if (was_inside)
                {
                    if (inside)
                    {
                        // extend last time period, if possible, or finish last and create new one
                        if (ref_periods.lastPeriod().isCloseToEnd(tod, max_ref_time_diff_)) // 4.9
                            ref_periods.lastPeriod().extend(tod);
                        else
                            ref_periods.add({tod, tod});
                    }
                }
                else if (inside) // was not inside and is now inside
                    ref_periods.add({tod, tod}); // create new time period
            }
        }
        ref_periods.removeSmallPeriods(1);

        logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
               << " periods '" << ref_periods.print() << "'";

        tod = 0;
        last_tod = 0;

        // evaluate test data
        const std::multimap<float, unsigned int>& tst_data = target_data.tstData();

        int sum_uis = ref_periods.getUIs(update_interval_s_);

        float d_tod{0};

        int sum_missed_uis {0};
        //int max_gap_uis {0};

        //int no_ref_uis {0};

        bool is_inside {false}, was_outside {false};
        pair<EvaluationTargetPosition, bool> ret_pos;
        bool ok;

        vector<DetectionDetail> details;
        EvaluationTargetPosition pos_current;

        unsigned int tst_data_size = tst_data.size();

        if (!tst_data_size)
        {
            for (auto& period_it : ref_periods)
            {
                last_tod = period_it.begin();
                tod = period_it.end();

                d_tod = tod - last_tod;

                if (isMiss(d_tod))
                {
                    sum_missed_uis += floor(d_tod/update_interval_s_);

                    assert (target_data.hasRefPosForTime(tod));
                    pos_current = target_data.refPosForTime(tod);

                    logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                           << " miss of " << String::timeStringFromDouble(d_tod) << " uis "
                           << floor(d_tod/update_interval_s_)
                           << " at [" << String::timeStringFromDouble(last_tod) << "," << String::timeStringFromDouble(tod)
                           << "] sum_missed_uis " << sum_missed_uis;

                    string comment = "Miss detected (DToD > "
                            +String::doubleToStringPrecision(missThreshold(), 2)
                            +"), last was "+String::timeStringFromDouble(last_tod);

                    DetectionDetail detail {
                        tod, d_tod, true,
                                pos_current, true,
                                sum_missed_uis, comment};

                    assert (target_data.hasRefPosForTime(last_tod));
                    detail.pos_last = target_data.refPosForTime(last_tod);
                    detail.has_last_position_ = true;

                    details.push_back(detail);
                }
            }

            return make_shared<EvaluationRequirementResult::SingleDetection>(
                        "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                        eval_man_, sum_uis, sum_missed_uis, ref_periods, details);
        }


        // collect test times in ref periods
        map<unsigned int, float> period_last_tst_times; // period number -> lst tst times
        set<unsigned int> finalized_periods;

        bool is_inside_ref_time_period {false}, was_inside_ref_time_period {false};
        unsigned int period_index {0};

        set<unsigned int> finished_periods;

        int period_max_index_before_tod;

        string comment;

        bool skip_no_data_details = eval_man_.resultsGenerator().skipNoDataDetails();

        for (auto tst_it=tst_data.begin(); tst_it != tst_data.end(); ++tst_it)
        {
            comment = "";

            last_tod = tod;
            tod = tst_it->first;
            pos_current = target_data.tstPosForTime(tod);


            logdbg << "EvaluationRequirementDetection '" << name_
                   << "': evaluate: utn " << target_data.utn_ << " tod " << tod;

            was_inside_ref_time_period = is_inside_ref_time_period;
            is_inside_ref_time_period = ref_periods.isInside(tod);

            // check if previous periods need to be finalized
            period_max_index_before_tod = ref_periods.getPeriodMaxIndexBefore(tod);

            if (period_max_index_before_tod != -1)
            {
                for (unsigned int period_cnt=0; period_cnt <= period_max_index_before_tod; ++period_cnt)
                {
                    if (!finished_periods.count(period_cnt)) // previous not finalized
                    {
                        float last_period_tod = ref_periods.period(period_cnt).begin();
                        bool tst_time_found = false;
                        float last_period_end = ref_periods.period(period_cnt).end();

                        if (period_last_tst_times.count(period_cnt))
                        {
                            last_period_tod = period_last_tst_times.at(period_cnt);
                            tst_time_found = true;
                        }

                        assert (last_period_end >= last_period_tod);

                        d_tod = last_period_end - last_period_tod;

                        assert (target_data.hasRefPosForTime(last_period_end));
                        pos_current = target_data.refPosForTime(last_period_end);

                        if (isMiss(d_tod))
                        {
                            sum_missed_uis += floor(d_tod/update_interval_s_);

                            logdbg << "EvaluationRequirementDetection '" << name_
                                   << "': evaluate: utn " << target_data.utn_
                                   << " miss of " << String::timeStringFromDouble(d_tod) << " uis "
                                   << floor(d_tod/update_interval_s_)
                                   << " at [" << String::timeStringFromDouble(last_period_tod) << ","
                                   << String::timeStringFromDouble(ref_periods.period(period_cnt).end())
                                   << "] missed_uis " << sum_missed_uis;

                            comment = "Miss detected in previous period "+to_string(period_cnt)
                                    +" (DToD > " +String::doubleToStringPrecision(missThreshold(), 2)
                                    +"), between ["+String::timeStringFromDouble(last_period_tod)+", "
                                    +String::timeStringFromDouble(ref_periods.period(period_cnt).end())+"]\n";

                            DetectionDetail detail{tod, d_tod, true,
                                        pos_current, false,
                                        sum_missed_uis, comment};

                            if (tst_time_found)
                            {
                                assert (target_data.hasTstPosForTime(last_period_tod));
                                detail.pos_last = target_data.tstPosForTime(last_period_tod);
                                detail.has_last_position_ = true;
                            }
                            else
                            {
                                assert (target_data.hasRefPosForTime(last_period_tod));
                                detail.pos_last = target_data.refPosForTime(last_period_tod);
                                detail.has_last_position_ = true;
                            }

                            details.push_back(detail);
                        }
                        else
                        {
                            comment = "Previous period "+to_string(period_cnt)
                                    +" OK (DToD <= "+String::doubleToStringPrecision(missThreshold(), 2)+")\n";

                            DetectionDetail detail{tod, d_tod, false,
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
                    {tod, {}, false,
                     pos_current, is_inside_ref_time_period,
                     sum_missed_uis, "Outside of reference time periods"});

                // TODO undetected previous miss possible
                continue;
            }

            // is inside period
            period_index = ref_periods.getPeriodIndex(tod);

            ret_pos = target_data.interpolatedRefPosForTime(tod, max_ref_time_diff_);

            ref_pos = ret_pos.first;
            ok = ret_pos.second;
            //assert (ok); // must be since inside ref time interval

            if (!ok)
            {
                loginf << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                       << " impossible check failed";
                continue;
            }

            has_ground_bit = target_data.hasTstGroundBitForTime(tod);

            if (has_ground_bit)
                ground_bit_set = target_data.tstGroundBitForTime(tod);
            else
                ground_bit_set = false;

            is_inside = sector_layer.isInside(ref_pos, has_ground_bit, ground_bit_set);

            if (!is_inside)
            {
                logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                       << " outside";

                if (!skip_no_data_details)
                    details.push_back(
                    {tod, {}, false,
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
                    {tod, {}, false,
                     pos_current, is_inside_ref_time_period,
                     sum_missed_uis, "First target report after outside sector"});
                }
                else // first in period
                {
                    details.push_back(
                    {tod, {}, false,
                     pos_current, is_inside_ref_time_period,
                     sum_missed_uis, "First target report in period "+to_string(period_index)});

                    // check if begin time in period is miss

                    d_tod = tod - ref_periods.period(period_index).begin();

                    if (isMiss(d_tod))
                    {
                        sum_missed_uis += floor(d_tod/update_interval_s_);

                        logdbg << "EvaluationRequirementDetection '" << name_
                               << "': evaluate: utn " << target_data.utn_
                               << " miss of " << String::timeStringFromDouble(d_tod) << " uis "
                               << floor(d_tod/update_interval_s_)
                               << " at [" << String::timeStringFromDouble(ref_periods.period(period_index).begin())
                               << ","<< String::timeStringFromDouble(tod)
                               << "] missed_uis " << sum_missed_uis;

                        comment = "Miss detected in current period "+to_string(period_index)
                                +" (DToD > " +String::doubleToStringPrecision(missThreshold(), 2)
                                +"), between ["+String::timeStringFromDouble(ref_periods.period(period_index).begin())
                                +", "+String::timeStringFromDouble(tod)+"]\n";

                        DetectionDetail detail{tod, d_tod, true,
                                    target_data.tstPosForTime(tod), is_inside_ref_time_period,
                                    sum_missed_uis, comment};

                        assert (target_data.hasRefPosForTime(ref_periods.period(period_index).begin()));
                        detail.pos_last = target_data.refPosForTime(ref_periods.period(period_index).begin());
                        detail.has_last_position_ = true;

                        details.push_back(detail);
                    }
                }


                was_outside = false;
                period_last_tst_times[period_index] = tod;

                continue;
            }

            assert (tod >= last_tod);
            d_tod = tod - last_tod;

            logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                   << " tod " << String::timeStringFromDouble(tod) << " d_tod " << String::timeStringFromDouble(d_tod);

            if (isMiss(d_tod))
            {
                sum_missed_uis += floor(d_tod/update_interval_s_);

                logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                       << " miss of " << String::timeStringFromDouble(d_tod) << " uis "
                       << floor(d_tod/update_interval_s_)
                       << " at [" << String::timeStringFromDouble(last_tod) << "," << String::timeStringFromDouble(tod)
                       << "] sum_missed_uis " << sum_missed_uis;

                string comment = "Miss detected (DToD > "
                        +String::doubleToStringPrecision(missThreshold(), 2)
                        +"), last was "+String::timeStringFromDouble(last_tod);

                DetectionDetail detail {
                    tod, d_tod, true,
                            pos_current, is_inside_ref_time_period,
                            sum_missed_uis, comment};

                detail.pos_last = target_data.tstPosForTime(last_tod);
                detail.has_last_position_ = true;

                details.push_back(detail);
            }
            else
            {
                logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                       << " update ok";

                details.push_back(
                {tod, d_tod, false,
                 pos_current, is_inside_ref_time_period,
                 sum_missed_uis, "OK (DToD <= "+String::doubleToStringPrecision(missThreshold(), 2)+")"});
            }


            period_last_tst_times[period_index] = tod;
        }

        // finalize unfinished periods
        logdbg << "EvaluationRequirementDetection '" << name_
               << "': evaluate: utn " << target_data.utn_ << " finalizing " << ref_periods.size()
               << " unfinished periods";

        for (unsigned int period_cnt=0; period_cnt < ref_periods.size(); ++period_cnt)
        {
            if (!finished_periods.count(period_cnt)) // previous not finalized
            {
                float last_period_tod = ref_periods.period(period_cnt).begin();
                bool tst_time_found = false;
                float last_period_end = ref_periods.period(period_cnt).end();

                if (period_last_tst_times.count(period_cnt))
                {
                    last_period_tod = period_last_tst_times.at(period_cnt);
                    tst_time_found = true;
                }

                assert (last_period_end >= last_period_tod);

                d_tod = last_period_end - last_period_tod;

                assert (target_data.hasRefPosForTime(last_period_end));
                pos_current = target_data.refPosForTime(last_period_end);

                if (isMiss(d_tod))
                {
                    sum_missed_uis += floor(d_tod/update_interval_s_);

                    logdbg << "EvaluationRequirementDetection '" << name_
                           << "': evaluate: utn " << target_data.utn_
                           << " miss of " << String::timeStringFromDouble(d_tod) << " uis "
                           << floor(d_tod/update_interval_s_)
                           << " at [" << String::timeStringFromDouble(last_period_tod) << ","
                           << String::timeStringFromDouble(ref_periods.period(period_cnt).end())
                           << "] missed_uis " << sum_missed_uis;

                    comment = "Miss detected in previous period "+to_string(period_cnt)
                            +" (DToD > " +String::doubleToStringPrecision(missThreshold(), 2)
                            +"), between ["+String::timeStringFromDouble(last_period_tod)+", "
                            +String::timeStringFromDouble(ref_periods.period(period_cnt).end())+"]\n";

                    DetectionDetail detail{tod, d_tod, true,
                                pos_current, false,
                                sum_missed_uis, comment};

                    if (tst_time_found)
                    {
                        assert (target_data.hasTstPosForTime(last_period_tod));
                        detail.pos_last = target_data.tstPosForTime(last_period_tod);
                        detail.has_last_position_ = true;
                    }
                    else
                    {
                        assert (target_data.hasRefPosForTime(last_period_tod));
                        detail.pos_last = target_data.refPosForTime(last_period_tod);
                        detail.has_last_position_ = true;
                    }

                    details.push_back(detail);
                }
                else
                {
                    comment = "Previous period "+to_string(period_cnt)
                            +" OK (DToD <= "+String::doubleToStringPrecision(missThreshold(), 2)+")\n";

                    DetectionDetail detail{tod, d_tod, false,
                                pos_current, false,
                                sum_missed_uis, comment};

                    details.push_back(detail);
                }

                finished_periods.insert(period_cnt);
            }
        }

        logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
               << " sum_uis " << sum_uis;

        if (sum_uis)
        {
            //assert (sum_uis >= max_gap_uis+no_ref_uis);
            assert (sum_missed_uis <= sum_uis);

            float pd = 1.0 - ((float)sum_missed_uis/(float)(sum_uis)); // -max_gap_uis-no_ref_uis

            logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                   << " pd " << String::percentToString(100.0 * pd) << " passed " << (pd >= minimum_probability_);
        }
        else
            logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                   << " no data for pd";

        return make_shared<EvaluationRequirementResult::SingleDetection>(
                    "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                    eval_man_, sum_uis, sum_missed_uis, ref_periods, details);
    }

    bool Detection::isMiss (float d_tod)
    {
        if (use_miss_tolerance_)
            return d_tod > (update_interval_s_ + miss_tolerance_s_);
        else
            return d_tod > update_interval_s_;
    }
}
