#include "evaluationstandard.h"
#include "evaluationmanager.h"

EvaluationStandard::EvaluationStandard(const std::string& class_id, const std::string& instance_id,
                                       EvaluationManager& eval_man)
: Configurable(class_id, instance_id, &eval_man), eval_man_(eval_man)
{
    registerParameter("name_", &name_, "");

    assert (name_.size());

    createSubConfigurables();
}

EvaluationStandard::~EvaluationStandard()
{

}


void EvaluationStandard::generateSubConfigurable(const std::string& class_id,
                                                 const std::string& instance_id)
{

}


void EvaluationStandard::checkSubConfigurables()
{

}
