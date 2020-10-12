#include "eval/requirement/identification/identification.h"
#include "eval/results/identification/single.h"
#include "evaluationdata.h"
#include "logger.h"
#include "stringconv.h"

using namespace std;
using namespace Utils;

namespace EvaluationRequirement
{

    Identification::Identification(
            const std::string& name, const std::string& short_name, const std::string& group_name,
            EvaluationManager& eval_man, float minimum_probability)
        : Base(name, short_name, group_name, eval_man), minimum_probability_(minimum_probability)
    {

    }


    float Identification::minimumProbability() const
    {
        return minimum_probability_;
    }

    std::shared_ptr<EvaluationRequirementResult::Single> Identification::evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance)
    {
        logdbg << "EvaluationRequirementIdentification '" << name_ << "': evaluate: utn " << target_data.utn_
               << " minimum_probability " << minimum_probability_;

        const std::multimap<float, unsigned int>& tst_data = target_data.tstData();

        float tod{0};

        float ref_lower{0}, ref_upper{0};

        int num_updates {0};
        int num_no_ref {0};
        int num_unknown_id {0};
        int num_correct_id {0};
        int num_false_id {0};

        //vector<IdentificationDetail> details;
        EvaluationTargetPosition pos_current;
        string callsign;
        bool callsign_ok;

        for (const auto& tst_id : tst_data)
        {
            tod = tst_id.first;
            pos_current = target_data.tstPosForTime(tod);

            if (target_data.hasTstCallsignForTime(tod))
            {
                callsign = target_data.tstCallsignForTime(tod);

                tie(ref_lower, ref_upper) = target_data.refTimesFor(tod, 4);

                if ((ref_lower != -1 || ref_upper != -1)) // ref times possible
                {
                    if ((ref_lower != -1 && target_data.hasRefCallsignForTime(ref_lower))
                            || (ref_upper != -1 && target_data.hasRefCallsignForTime(ref_upper))) // ref value(s) exist
                    {
                        callsign_ok = false;

                        if (ref_lower != -1 && target_data.hasRefCallsignForTime(ref_lower))
                            callsign_ok = target_data.refCallsignForTime(ref_lower) == callsign;

                        if (!callsign_ok && ref_upper != -1 && target_data.hasRefCallsignForTime(ref_upper))
                            callsign_ok = target_data.refCallsignForTime(ref_upper) == callsign;

                        if (callsign_ok)
                            ++num_correct_id;
                        else
                            ++num_false_id;
                    }
                    else
                        ++num_no_ref;
                }
                else
                    ++num_no_ref;
            }
            else
                ++num_unknown_id;

            ++num_updates;
        }

        assert (num_updates == num_no_ref+num_unknown_id+num_correct_id+num_false_id);

        //assert (details.size() == tst_data.size());

        //float sum_uis = floor(tod - first_tod);

        loginf << "EvaluationRequirementIdentification '" << name_ << "': evaluate: utn " << target_data.utn_
               << " num_updates " << num_updates << " num_no_ref " << num_no_ref
               << " num_unknown_id " << num_unknown_id << " num_correct_id " << num_correct_id
               << " num_false_id " << num_false_id;

        if (num_updates)
        {
            float pid = (float)num_correct_id/(float)(num_correct_id+num_false_id);

            loginf << "EvaluationRequirementIdentification '" << name_ << "': evaluate: utn " << target_data.utn_
                   << " pid " << String::percentToString(100.0 * pid) << " passed " << (pid >= minimum_probability_);
        }
        else
            loginf << "EvaluationRequirementIdentification '" << name_ << "': evaluate: utn " << target_data.utn_
                   << " no data for pid";

        return make_shared<EvaluationRequirementResult::SingleIdentification>(
                    "UTN:"+to_string(target_data.utn_), instance, target_data.utn_, &target_data,
                    eval_man_, num_updates, num_no_ref, num_unknown_id, num_correct_id, num_false_id); // , details
    }
}
