#include "momcorrect.h"

namespace EvaluationRequirement
{


MomLongAccCorrect::MomLongAccCorrect(const std::string& name, const std::string& short_name, const std::string& group_name,
                                     float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man)
: GenericInteger(name, short_name, group_name, prob, prob_check_type, eval_man)
{
    result_type_ = "MomLongAccCorrect";

    value_name_ = "MoM Longitudinal Acceleration";
    value_name_short_ = "MoM Long";
    value_name_plural_ = "MoM Longitudinal Acceleration";

    probability_name_ = "Probability of Correct MoM Longitudinal Acceleration";
    probability_name_short_ = "PCLAcc";

     value_compare_func_ = [this] (const dbContent::TargetReport::Chain::DataID& id,
                             const EvaluationTargetData& target_data,
                             boost::posix_time::time_duration max_ref_time_diff)
    { return compareMomLongAcc(id, target_data, max_ref_time_diff); };
}


MomTransAccCorrect::MomTransAccCorrect(const std::string& name, const std::string& short_name, const std::string& group_name,
                                     float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man)
    : GenericInteger(name, short_name, group_name, prob, prob_check_type, eval_man)
{
     result_type_ = "MomTransAccCorrect";

     value_name_ = "MoM Transversal Acceleration";
     value_name_short_ = "MoM Trans";
     value_name_plural_ = "MoM Transversal Acceleration";

     probability_name_ = "Probability of Correct MoM Transversal Acceleration";
     probability_name_short_ = "PCTAcc";

     value_compare_func_ = [this] (const dbContent::TargetReport::Chain::DataID& id,
                                  const EvaluationTargetData& target_data,
                                  boost::posix_time::time_duration max_ref_time_diff)
     { return compareMomTransAcc(id, target_data, max_ref_time_diff); };
}

MomVertRateCorrect::MomVertRateCorrect(const std::string& name, const std::string& short_name, const std::string& group_name,
                                       float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man)
    : GenericInteger(name, short_name, group_name, prob, prob_check_type, eval_man)
{
     result_type_ = "MomVertRateCorrect";

     value_name_ = "MoM Vertical Rate";
     value_name_short_ = "MoM Vert";
     value_name_plural_ = "MoM Vertical Rate";

     probability_name_ = "Probability of Correct MoM Vertical Rate";
     probability_name_short_ = "PCVRt";

     value_compare_func_ = [this] (const dbContent::TargetReport::Chain::DataID& id,
                                  const EvaluationTargetData& target_data,
                                  boost::posix_time::time_duration max_ref_time_diff)
     { return compareMomVertRate(id, target_data, max_ref_time_diff); };
}

}
