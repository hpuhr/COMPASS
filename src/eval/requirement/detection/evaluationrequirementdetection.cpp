#include "evaluationrequirementdetection.h"
#include "evaluationdata.h"
#include "logger.h"
#include "stringconv.h"

using namespace std;
using namespace Utils;


EvaluationRequirementDetection::EvaluationRequirementDetection(const std::string& name, const std::string& short_name,
                                                               const std::string& group_name, float update_interval_s,
                                                               float minimum_probability, bool use_max_gap_interval,
                                                               float max_gap_interval_s, bool use_miss_tolerance,
                                                               float miss_tolerance_s)
    : EvaluationRequirement(name, short_name, group_name), update_interval_s_(update_interval_s),
      minimum_probability_(minimum_probability), use_max_gap_interval_(use_max_gap_interval),
      max_gap_interval_s_(max_gap_interval_s), use_miss_tolerance_(use_miss_tolerance),
      miss_tolerance_s_(miss_tolerance_s)
{

}

float EvaluationRequirementDetection::updateInterval() const
{
    return update_interval_s_;
}

float EvaluationRequirementDetection::minimumProbability() const
{
    return minimum_probability_;
}

bool EvaluationRequirementDetection::useMaxGapInterval() const
{
    return use_max_gap_interval_;
}

float EvaluationRequirementDetection::maxGapInterval() const
{
    return max_gap_interval_s_;
}

bool EvaluationRequirementDetection::useMissTolerance() const
{
    return use_miss_tolerance_;
}

float EvaluationRequirementDetection::missTolerance() const
{
    return miss_tolerance_s_;
}

void EvaluationRequirementDetection::evaluate (const EvaluationTargetData& target_data)
{
    logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_;


    const std::multimap<float, unsigned int>& tst_data = target_data.tstData();

    bool first {true};
    float first_tod{0};

    float tod{0}, last_tod{0};
    float d_tod{0};

    float missed_uis {0};
    float max_gap_uis {0};

    bool no_ref_exists {false};
    float no_ref_first_tod {0};
    float no_ref_uis {0};

    for (const auto& tst_id : tst_data)
    {
        last_tod = tod;
        tod = tst_id.first;

        if (first)
        {
            first = false;
            first_tod = tod;
            continue;
        }

        if (!target_data.hasRefDataForTime(tod, 4)) // seconds max time difference
        {
            if (!no_ref_exists)
                no_ref_first_tod = tod; // save first occurance of no ref

            no_ref_exists = true;
            continue; // try next time
        }

        if (no_ref_exists) // previously no ref existed
        {
            assert (no_ref_first_tod);
            assert (tod >= no_ref_first_tod);
            no_ref_uis = floor(tod - no_ref_first_tod);

            no_ref_exists = false;
            no_ref_first_tod = 0;

            continue; // cannot assess this time, goto next
        }

        assert (tod >= last_tod);
        d_tod = tod - last_tod;

        if (isMaxGap(d_tod))
        {
            loginf << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                   << " max gap of " << String::timeStringFromDouble(floor(d_tod/update_interval_s_))
                   << " at " << String::timeStringFromDouble(tod);

            max_gap_uis += floor(d_tod/update_interval_s_);

            continue;
        }

        if (isMiss(d_tod))
        {
            logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                   << " miss of " << String::timeStringFromDouble(floor(d_tod/update_interval_s_))
                   << " at " << String::timeStringFromDouble(tod);

            missed_uis += floor(d_tod/update_interval_s_);
        }
    }

    float sum_uis = floor(tod - first_tod);

    if (!first && sum_uis)
    {
        assert (sum_uis >= max_gap_uis+no_ref_uis);

        float pd = 1.0 - (missed_uis/(sum_uis-max_gap_uis-no_ref_uis));

        loginf << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
               << " pd " << String::percentToString(100.0 * pd);
    }
    else
        loginf << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
               << " no data for pd";
}

bool EvaluationRequirementDetection::isMiss (float d_tod)
{
    if (use_miss_tolerance_)
        return d_tod > (update_interval_s_ + miss_tolerance_s_);
    else
        return d_tod > update_interval_s_;
}

bool EvaluationRequirementDetection::isMaxGap (float d_tod)
{
    if (use_max_gap_interval_)
        return d_tod > max_gap_interval_s_;
    else
        return false;
}
