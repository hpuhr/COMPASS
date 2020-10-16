#include "eval/results/joined.h"

namespace EvaluationRequirementResult
{


Joined::Joined(const std::string& type, const std::string& result_id,
               std::shared_ptr<EvaluationRequirement::Base> requirement, const SectorLayer& sector_layer,
               EvaluationManager& eval_man)
    : Base(type, result_id, requirement, sector_layer, eval_man)
{
}

void Joined::join(std::shared_ptr<Base> other)
{
    results_.push_back(other);
}

}
