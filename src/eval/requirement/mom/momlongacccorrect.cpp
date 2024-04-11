#include "momlongacccorrect.h"

namespace EvaluationRequirement
{


MomLongAccCorrect::MomLongAccCorrect(const std::string& name, const std::string& short_name, const std::string& group_name,
                                     float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man)
: Generic(name, short_name, group_name, prob, prob_check_type, eval_man)
{
    result_type_ = "MomLongAccCorrect";

    value_name_ = "MoM Longitudinal Acceleration";
    value_name_short_ = "MoM Long";
    value_name_plural_ = "MoM Longitudinal Accelerations";

    probability_name_ = "Probability of Correct MoM Longitudinal Acceleration";
    probability_name_short_ = "PCLAcc";

     value_compare_func_ = [this] (const dbContent::TargetReport::Chain::DataID& id,
                             const EvaluationTargetData& target_data,
                             boost::posix_time::time_duration max_ref_time_diff)
    { return compareMomLongAcc(id, target_data, max_ref_time_diff); };
}


}
