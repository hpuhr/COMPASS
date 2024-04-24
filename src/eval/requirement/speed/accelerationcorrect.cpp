#include "accelerationcorrect.h"

namespace EvaluationRequirement {

AccelerationCorrect::AccelerationCorrect(const std::string& name, const std::string& short_name, const std::string& group_name,
                                         float prob, COMPARISON_TYPE prob_check_type, double threshold, EvaluationManager& eval_man)
: GenericDouble(name, short_name, group_name, prob, prob_check_type, threshold, eval_man)
{
    result_type_ = "AccelerationCorrect";

    value_name_ = "Acceleration";
    value_name_short_ = "Acc";
    value_name_plural_ = "Acceleration";

    probability_name_ = "Probability of Correct ACC";
    probability_name_short_ = "PCAcc";

    value_compare_func_ = [this] (const dbContent::TargetReport::Chain::DataID& id,
                                 const EvaluationTargetData& target_data,
                                 boost::posix_time::time_duration max_ref_time_diff, double threshold)
    { return compareAcceleration(id, target_data, max_ref_time_diff, threshold); };
}

} // namespace EvaluationRequirement
