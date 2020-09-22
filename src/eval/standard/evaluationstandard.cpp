#include "evaluationstandard.h"
#include "evaluationmanager.h"
#include "evaluationrequirementgroup.h"
#include "evaluationstandardwidget.h"
#include "logger.h"

using namespace std;

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
    if (class_id.compare("EvaluationRequirementGroup") == 0)
    {
        EvaluationRequirementGroup* group = new EvaluationRequirementGroup(class_id, instance_id, *this);
        logdbg << "EvaluationStandard: generateSubConfigurable: adding group " << group->name();

        assert(groups_.find(group->name()) == groups_.end());
        groups_.insert(std::pair<std::string, EvaluationRequirementGroup*>(group->name(), group));
    }
    else
        throw std::runtime_error("EvaluationStandard: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

std::string EvaluationStandard::name() const
{
    return name_;
}

bool EvaluationStandard::hasGroup (const std::string& name)
{
    return groups_.count(name);
}

void EvaluationStandard::addGroup (const std::string& name)
{
    assert (!hasGroup(name));

    generateSubConfigurable("EvaluationRequirementGroup", "EvaluationRequirementGroup"+name+"0");

    assert (hasGroup(name));

    emit groupsChangedSignal();
}

EvaluationRequirementGroup& EvaluationStandard::group (const std::string& name)
{
    assert (hasGroup(name));
    return *groups_.at(name);
}

void EvaluationStandard::removeGroup (const std::string& name)
{
    assert (hasGroup(name));
    delete groups_.at(name);
    groups_.erase(name);

    emit groupsChangedSignal();
}

EvaluationStandardWidget* EvaluationStandard::widget()
{
    if (!widget_)
        widget_.reset(new EvaluationStandardWidget(*this));

    return widget_.get();
}

void EvaluationStandard::checkSubConfigurables()
{

}
