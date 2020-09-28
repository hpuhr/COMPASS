#include "evaluationrequirementdetection.h"
#include "evaluationrequirementdetectionresult.h"
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

std::shared_ptr<EvaluationRequirementResult> EvaluationRequirementDetection::evaluate (
        const EvaluationTargetData& target_data, std::shared_ptr<EvaluationRequirement> instance)
{
    logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
           << " update_interval " << update_interval_s_ << " minimum_probability " << minimum_probability_
           << " use_max_gap_interval " << use_max_gap_interval_ << " max_gap_interval " << max_gap_interval_s_
           << " use_miss_tolerance " << use_miss_tolerance_ << " miss_tolerance " << miss_tolerance_s_;


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

            logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                   << " first tod " << String::timeStringFromDouble(first_tod);

            continue;
        }

        if (!target_data.hasRefDataForTime(tod, 4)) // seconds max time difference
        {
            if (!no_ref_exists)
                no_ref_first_tod = tod; // save first occurance of no ref

            no_ref_exists = true;

            logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
                   << " no ref at " << String::timeStringFromDouble(tod);

            continue; // try next time
        }

        if (no_ref_exists) // previously no ref existed
        {
            assert (no_ref_first_tod);
            assert (tod >= no_ref_first_tod);
            no_ref_uis = floor(tod - no_ref_first_tod);

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
        }
    }

    float sum_uis = floor(tod - first_tod);

    logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
           << " sum_uis " << sum_uis << " max_gap_uis " << max_gap_uis << " no_ref_uis " << no_ref_uis
           << " max_gap_uis+no_ref_uis " << max_gap_uis+no_ref_uis;

    if (!first && sum_uis)
    {
        assert (sum_uis >= max_gap_uis+no_ref_uis);

        float pd = 1.0 - (missed_uis/(sum_uis-max_gap_uis-no_ref_uis));

        logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
               << " pd " << String::percentToString(100.0 * pd) << " passed " << (pd >= minimum_probability_);
    }
    else
        logdbg << "EvaluationRequirementDetection '" << name_ << "': evaluate: utn " << target_data.utn_
               << " no data for pd";

    vector<unsigned int> utns = {target_data.utn_};

    return make_shared<EvaluationRequirementDetectionResult>(
                instance, utns, sum_uis, missed_uis, max_gap_uis, no_ref_uis);
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
