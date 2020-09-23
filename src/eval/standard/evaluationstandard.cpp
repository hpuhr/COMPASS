#include "evaluationstandard.h"
#include "evaluationmanager.h"
#include "evaluationrequirementgroup.h"
#include "evaluationstandardwidget.h"
#include "logger.h"

#include <QTreeView>

using namespace std;

EvaluationStandard::EvaluationStandard(const std::string& class_id, const std::string& instance_id,
                                       EvaluationManager& eval_man)
    : Configurable(class_id, instance_id, &eval_man), EvaluationStandardTreeItem(&root_item_), eval_man_(eval_man),
      root_item_(*this)
{
    registerParameter("name", &name_, "");

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
        groups_[group->name()].reset(group);
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

    std::string instance = "EvaluationRequirementGroup" + name + "0";

    Configuration& config = addNewSubConfiguration("EvaluationRequirementGroup", instance);
    config.addParameterString("name", name);

    generateSubConfigurable("EvaluationRequirementGroup", instance);

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
    if (!hasGroup("All"))
        addGroup("All");
}

EvaluationStandardTreeItem* EvaluationStandard::child(int row)
{
    if (row < 0 || row >= groups_.size())
        return nullptr;

    auto group_it = groups_.begin();

    std::advance(group_it, row);

    assert (group_it != groups_.end());

    return group_it->second.get();
}

int EvaluationStandard::childCount() const
{
    return groups_.size();
}

int EvaluationStandard::columnCount() const
{
    return 1;
}

QVariant EvaluationStandard::data(int column) const
{
    assert (column == 0);

    return name_.c_str();
}

int EvaluationStandard::row() const
{
    return 0;
}

EvaluationStandardRootItem& EvaluationStandard::rootItem()
{
    return root_item_;
}


