#include "rocdcorrect.h"

namespace EvaluationRequirement {

ROCDCorrect::ROCDCorrect(const std::string& name, const std::string& short_name, const std::string& group_name,
                         float prob, COMPARISON_TYPE prob_check_type, double threshold, EvaluationManager& eval_man)
: GenericDouble(name, short_name, group_name, prob, prob_check_type, threshold, eval_man)
{
    result_type_ = "ROCDCorrect";

    value_name_ = "Rate of Climb/Descent";
    value_name_short_ = "ROCD";
    value_name_plural_ = "Rate of Climb/Descent";

    value_compare_func_ = [this] (const dbContent::TargetReport::Chain::DataID& id,
                                 const EvaluationTargetData& target_data,
                                 boost::posix_time::time_duration max_ref_time_diff, double threshold)
    { return compareROCD(id, target_data, max_ref_time_diff, threshold); };
}

} // namespace EvaluationRequirement
