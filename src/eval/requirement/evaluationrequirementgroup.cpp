#include "evaluationrequirementgroup.h"
#include "evaluationstandard.h"
#include "evaluationrequirementconfig.h"
#include "logger.h"

EvaluationRequirementGroup::EvaluationRequirementGroup(const std::string& class_id, const std::string& instance_id,
                                                       EvaluationStandard& standard)
    : Configurable(class_id, instance_id, &standard), EvaluationStandardTreeItem(&standard), standard_(standard)
{
    registerParameter("name", &name_, "");

    assert (name_.size());

    createSubConfigurables();

    // menu creation
    {
        QAction* del_action = menu_.addAction("Delete Group");
        connect(del_action, &QAction::triggered, this, &EvaluationRequirementGroup::deleteGroupSlot);
        QAction* add_action = menu_.addAction("Add Requirement");
        connect(add_action, &QAction::triggered, this, &EvaluationRequirementGroup::addRequirementSlot);
    }
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
        configs_[config->name()].reset(config);
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

    std::string instance = "EvaluationRequirementConfig" + name + "0";

    Configuration& config = addNewSubConfiguration("EvaluationRequirementConfig", instance);
    config.addParameterString("name", name);

    generateSubConfigurable("EvaluationRequirementConfig", instance);

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
    configs_.erase(name);

    emit configsChangedSignal();
}

void EvaluationRequirementGroup::checkSubConfigurables()
{

}

EvaluationStandardTreeItem* EvaluationRequirementGroup::child(int row)
{
    if (row < 0 || row >= configs_.size())
        return nullptr;

    auto group_it = configs_.begin();

    std::advance(group_it, row);

    assert (group_it != configs_.end());

    return group_it->second.get();
}

int EvaluationRequirementGroup::childCount() const
{
    return configs_.size();
}

int EvaluationRequirementGroup::columnCount() const
{
    return 1;
}

QVariant EvaluationRequirementGroup::data(int column) const
{
    assert (column == 0);

    return name_.c_str();
}

int EvaluationRequirementGroup::row() const
{
    return 0;
}

void EvaluationRequirementGroup::showMenu ()
{
    menu_.exec(QCursor::pos());
}

void EvaluationRequirementGroup::deleteGroupSlot()
{
    loginf << "EvaluationRequirementGroup " << name_ << ": deleteGroupSlot";
}

void EvaluationRequirementGroup::addRequirementSlot()
{
    loginf << "EvaluationRequirementGroup " << name_ << ": addRequirementSlot";
}
