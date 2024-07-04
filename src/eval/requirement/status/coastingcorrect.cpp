#include "coastingcorrect.h"

namespace EvaluationRequirement
{


CoastingCorrect::CoastingCorrect(const std::string& name, const std::string& short_name, const std::string& group_name,
                                     float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man)
: GenericInteger(name, short_name, group_name, prob, prob_check_type, eval_man)
{
    result_type_ = "CoastingCorrect";

    value_name_ = "Track Coasting";
    value_name_short_ = "Coast";
    value_name_plural_ = "Track Coasting";

    value_compare_func_ = [this] (const dbContent::TargetReport::Chain::DataID& id,
                             const EvaluationTargetData& target_data,
                             boost::posix_time::time_duration max_ref_time_diff)
    { return compareCoasting(id, target_data, max_ref_time_diff); };
}


}
