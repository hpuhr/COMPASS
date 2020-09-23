#include "evaluationrequirementconfig.h"
#include "evaluationrequirementgroup.h"

EvaluationRequirementConfig::EvaluationRequirementConfig(const std::string& class_id, const std::string& instance_id,
                                                         EvaluationRequirementGroup& group)
    : Configurable(class_id, instance_id, &group), EvaluationStandardTreeItem(&group), group_(group)
{
    registerParameter("name", &name_, "");

    assert (name_.size());

    createSubConfigurables();
}

EvaluationRequirementConfig::~EvaluationRequirementConfig()
{

}


void EvaluationRequirementConfig::generateSubConfigurable(const std::string& class_id,
                                                          const std::string& instance_id)
{
    assert(false);
}

std::string EvaluationRequirementConfig::name() const
{
    return name_;
}

void EvaluationRequirementConfig::checkSubConfigurables()
{
}

EvaluationStandardTreeItem* EvaluationRequirementConfig::child(int row)
{
    return nullptr;
}

int EvaluationRequirementConfig::childCount() const
{
    return 0;
}

int EvaluationRequirementConfig::columnCount() const
{
    return 1;
}

QVariant EvaluationRequirementConfig::data(int column) const
{
    assert (column == 0);

    return name_.c_str();
}

int EvaluationRequirementConfig::row() const
{
    return 0;
}
