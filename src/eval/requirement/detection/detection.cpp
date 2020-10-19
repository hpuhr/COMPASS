#include "eval/requirement/detection/detection.h"
#include "eval/results/detection/single.h"
#include "evaluationdata.h"
#include "logger.h"
#include "stringconv.h"
#include "sectorlayer.h"

using namespace std;
using namespace Utils;

namespace EvaluationRequirement
{

    Detection::Detection(
            const std::string& name, const std::string& short_name, const std::string& group_name,
            EvaluationManager& eval_man,
            float update_interval_s, float max_ref_time_diff, float minimum_probability, bool use_max_gap_interval,
            float max_gap_interval_s, bool use_miss_tolerance, float miss_tolerance_s)
        : Base(name, short_name, group_name, eval_man), update_interval_s_(update_interval_s),
          max_ref_time_diff_(max_ref_time_diff),
          minimum_probability_(minimum_probability), use_max_gap_interval_(use_max_gap_interval),
          max_gap_interval_s_(max_gap_interval_s), use_miss_tolerance_(use_miss_tolerance),
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

    bool Detection::useMaxGapInterval() const
    {
        return use_max_gap_interval_;
    }

    float Detection::maxGapInterval() const
    {
        return max_gap_interval_s_;
    }

    bool Detection::useMissTolerance() const
    {
        return use_miss_tolerance_;
    }

    float Detection::missTolerance() const
    {
        return miss_tolerance_s_;
    }

    std::shared_ptr<EvaluationRequirementResult::Single> Detection::evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer)
    {
        logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
               << " update_interval " << update_interval_s_ << " minimum_probability " << minimum_probability_
               << " use_max_gap_interval " << use_max_gap_interval_ << " max_gap_interval " << max_gap_interval_s_
               << " use_miss_tolerance " << use_miss_tolerance_ << " miss_tolerance " << miss_tolerance_s_;

        const std::multimap<float, unsigned int>& tst_data = target_data.tstData();

        bool first {true};
        float first_tod{0}; // first in inside sector leg
        float sum_uis {0};

        float tod{0}, last_tod{0};
        float d_tod{0};

        int missed_uis {0};
        int max_gap_uis {0};

        bool no_ref_exists {false};
        float no_ref_first_tod {0};
        int no_ref_uis {0};

        //bool ref_exists;
        bool is_inside;
        bool was_outside;
        pair<EvaluationTargetPosition, bool> ret_pos;
        EvaluationTargetPosition ref_pos;
        bool ok;

        vector<DetectionDetail> details;
        EvaluationTargetPosition pos_current;

        for (const auto& tst_id : tst_data)
        {
            last_tod = tod;
            tod = tst_id.first;
            pos_current = target_data.tstPosForTime(tod);

            if (!target_data.hasRefDataForTime(tod, max_ref_time_diff_)) // seconds max time difference
            {
                if (!no_ref_exists)
                {
                    no_ref_first_tod = tod; // save first occurance of no ref

                    details.push_back(
                    {tod, {}, false,
                     pos_current, false,
                     missed_uis, max_gap_uis, no_ref_uis,
                     "Occurance of no reference"});

                }
                else
                    details.push_back(
                    {tod, {}, false,
                     pos_current, false,
                     missed_uis, max_gap_uis, no_ref_uis,
                     "Still no reference"});

                no_ref_exists = true;

                logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                       << " no ref at " << String::timeStringFromDouble(tod);

                continue; // try next time
            }

            ret_pos = target_data.interpolatedRefPosForTime(tod, max_ref_time_diff_);

            ref_pos = ret_pos.first;
            ok = ret_pos.second;

            if (!ok)
            {
                details.push_back(
                {tod, {}, false,
                 pos_current, false,
                 missed_uis, max_gap_uis, no_ref_uis,
                 "No reference position"});

                no_ref_exists = true;

                //++num_no_ref_pos;
                continue;
            }

            //ref_exists = true;

            is_inside = sector_layer.isInside(ref_pos);

            if (!is_inside)
            {

                if (was_outside)
                {
                    details.push_back(
                    {tod, {}, false,
                     pos_current, false,
                     missed_uis, max_gap_uis, no_ref_uis,
                     "Outside sector"});
                }
                else
                {
                    details.push_back(
                    {tod, {}, false,
                     pos_current, false,
                     missed_uis, max_gap_uis, no_ref_uis,
                     "Outside sector, finishing leg ["+String::timeStringFromDouble(first_tod)
                                +","+String::timeStringFromDouble(tod)+"]"});


                    sum_uis += floor(tod - first_tod);

                }
                was_outside = true;

                //++num_pos_outside;
                continue;
            }

            if (first || was_outside)
            {
                first = false;
                first_tod = tod;

                logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                       << " first tod " << String::timeStringFromDouble(first_tod);

                if (was_outside)
                    details.push_back(
                    {tod, {}, false,
                     pos_current, target_data.hasRefDataForTime(tod, max_ref_time_diff_),
                     missed_uis, max_gap_uis, no_ref_uis, "First target report after outside sector"});
                else
                    details.push_back(
                    {tod, {}, false,
                     pos_current, target_data.hasRefDataForTime(tod, max_ref_time_diff_),
                     missed_uis, max_gap_uis, no_ref_uis, "First target report"});


                was_outside = false;

                continue;
            }

            if (no_ref_exists) // previously no ref existed
            {
                assert (no_ref_first_tod);
                assert (tod >= no_ref_first_tod);
                no_ref_uis = floor(tod - no_ref_first_tod);

                details.push_back(
                {tod, {}, false,
                 pos_current, true,
                 missed_uis, max_gap_uis, no_ref_uis,
                 "Reference exists again, was missing since "+String::timeStringFromDouble(no_ref_first_tod)});

                no_ref_exists = false;
                no_ref_first_tod = 0;

                logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                       << " ref after no ref at " << String::timeStringFromDouble(tod) << " no_ref_uis " << no_ref_uis;

                continue; // cannot assess this time, goto next
            }

            assert (tod >= last_tod);
            d_tod = tod - last_tod;

            logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                   << " tod " << String::timeStringFromDouble(tod) << " d_tod " << String::timeStringFromDouble(d_tod);

            if (isMaxGap(d_tod))
            {
                max_gap_uis += floor(d_tod/update_interval_s_);

                loginf << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                       << " max gap of " << String::timeStringFromDouble(floor(d_tod/update_interval_s_))
                       << " at " << String::timeStringFromDouble(tod) << " max_gap_uis " << max_gap_uis;

                string comment = "Max gap detected (DToD > "+to_string(max_gap_interval_s_)
                        +"), last was "+String::timeStringFromDouble(last_tod);

                DetectionDetail detail {
                    tod, d_tod, true,
                            pos_current, true,
                            missed_uis, max_gap_uis, no_ref_uis, comment};

                detail.pos_last = target_data.tstPosForTime(last_tod);
                detail.has_last_position_ = true;

                details.push_back(detail);

                continue;
            }

            if (isMiss(d_tod))
            {
                missed_uis += floor(d_tod/update_interval_s_);

                logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                       << " miss of " << String::timeStringFromDouble(d_tod) << " uis "
                       << floor(d_tod/update_interval_s_)
                       << " at [" << String::timeStringFromDouble(last_tod) << "," << String::timeStringFromDouble(tod)
                       << "] missed_uis " << missed_uis;

                string comment = "Miss detected (DToD > "
                        +to_string(use_miss_tolerance_ ? update_interval_s_+miss_tolerance_s_ : update_interval_s_)
                        +"), last was "+String::timeStringFromDouble(last_tod);

                DetectionDetail detail {
                    tod, d_tod, true,
                            pos_current, true,
                            missed_uis, max_gap_uis, no_ref_uis, comment};

                detail.pos_last = target_data.tstPosForTime(last_tod);
                detail.has_last_position_ = true;

                details.push_back(detail);
            }
            else
                details.push_back(
                {tod, d_tod, false,
                 pos_current, true,
                 missed_uis, max_gap_uis, no_ref_uis,
                 "OK (DToD <= "+to_string(update_interval_s_)+")"});
        }

        if (!was_outside)
        {
            sum_uis += floor(tod - first_tod); // add last timeframe
        }

        assert (details.size() == tst_data.size());

        logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
               << " sum_uis " << sum_uis << " max_gap_uis " << max_gap_uis << " no_ref_uis " << no_ref_uis
               << " max_gap_uis+no_ref_uis " << max_gap_uis+no_ref_uis;

        if (!first && sum_uis)
        {
            //assert (sum_uis >= max_gap_uis+no_ref_uis);
            if (sum_uis < max_gap_uis+no_ref_uis) // TODO error in calc
            {
                logwrn << "EvaluationRequirementDetection '" << name_ << "': evaluate: sum_is ("
                       << sum_uis << ") too small, max_gap_uis " << max_gap_uis << " no_ref_uis " << no_ref_uis;
            }
            else
            {

                float pd = 1.0 - ((float)missed_uis/(float)(sum_uis-max_gap_uis-no_ref_uis));

                logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                       << " pd " << String::percentToString(100.0 * pd) << " passed " << (pd >= minimum_probability_);
            }
        }
        else
            logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                   << " no data for pd";

        return make_shared<EvaluationRequirementResult::SingleDetection>(
                    "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                    eval_man_, sum_uis, missed_uis, max_gap_uis, no_ref_uis, details);
    }

    bool Detection::isMiss (float d_tod)
    {
        if (use_miss_tolerance_)
            return d_tod > (update_interval_s_ + miss_tolerance_s_);
        else
            return d_tod > update_interval_s_;
    }

    bool Detection::isMaxGap (float d_tod)
    {
        if (use_max_gap_interval_)
            return d_tod > max_gap_interval_s_;
        else
            return false;
    }

}
