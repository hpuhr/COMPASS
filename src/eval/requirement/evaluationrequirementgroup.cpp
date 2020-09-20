#include "evaluationrequirementgroup.h"
#include "evaluationstandard.h"
#include "evaluationrequirementconfig.h"
#include "logger.h"

EvaluationRequirementGroup::EvaluationRequirementGroup(const std::string& class_id, const std::string& instance_id,
                                                       EvaluationStandard& standard)
    : Configurable(class_id, instance_id, &standard), standard_(standard)
{
    registerParameter("name_", &name_, "");

    assert (name_.size());

    createSubConfigurables();
}

EvaluationRequirementGroup::~EvaluationRequirementGroup()
{

}


void EvaluationRequirementGroup::generateSubConfigurable(const std::string& class_id,
                                                         const std::string& instance_id)
{
    if (class_id.compare("EvaluationRequirementConfig") == 0)
    {
        EvaluationRequirementConfig* config = new EvaluationRequirementConfig(class_id, instance_id, *this);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(configs_.find(config->name()) == configs_.end());
        configs_.insert(std::pair<std::string, EvaluationRequirementConfig*>(config->name(), config));
    }
    else
        throw std::runtime_error("EvaluationStandard: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

std::string EvaluationRequirementGroup::name() const
{
    return name_;
}


bool EvaluationRequirementGroup::hasRequirementConfig (const std::string& name)
{
    return configs_.count(name);
}

void EvaluationRequirementGroup::addRequirementConfig (const std::string& name)
{
    assert (!hasRequirementConfig(name));

    generateSubConfigurable("EvaluationRequirementConfig", "EvaluationRequirementConfig"+name+"0");

    assert (hasRequirementConfig(name));

    emit configsChangedSignal();
}

EvaluationRequirementConfig& EvaluationRequirementGroup::requirementConfig (const std::string& name)
{
    assert (hasRequirementConfig(name));
    return *configs_.at(name);
}

void EvaluationRequirementGroup::removeRequirementConfig (const std::string& name)
{
    assert (hasRequirementConfig(name));
    delete configs_.at(name);
    configs_.erase(name);

    emit configsChangedSignal();
}

void EvaluationRequirementGroup::checkSubConfigurables()
{

}
