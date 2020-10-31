/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "evaluationstandard.h"
#include "evaluationmanager.h"
#include "eval/requirement/group.h"
#include "evaluationstandardwidget.h"
#include "evaluationstandardtreemodel.h"
#include "logger.h"

#include <QTreeView>
#include <QInputDialog>
#include <QMessageBox>

using namespace std;

EvaluationStandard::EvaluationStandard(const std::string& class_id, const std::string& instance_id,
                                       EvaluationManager& eval_man)
    : Configurable(class_id, instance_id, &eval_man), EvaluationStandardTreeItem(&root_item_), eval_man_(eval_man),
      root_item_(*this)
{
    registerParameter("name", &name_, "");

    assert (name_.size());

    createSubConfigurables();

    // menu creation
    {
        QAction* add_action = menu_.addAction("Add Group");
        connect(add_action, &QAction::triggered, this, &EvaluationStandard::addGroupSlot);
    }
}

EvaluationStandard::~EvaluationStandard()
{
}


void EvaluationStandard::generateSubConfigurable(const std::string& class_id,
                                                 const std::string& instance_id)
{
    if (class_id.compare("EvaluationRequirementGroup") == 0)
    {
        Group* group = new Group(class_id, instance_id, *this, eval_man_);
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

    if (widget_)
        beginModelReset();

    std::string instance = "EvaluationRequirementGroup" + name + "0";

    Configuration& config = addNewSubConfiguration("EvaluationRequirementGroup", instance);
    config.addParameterString("name", name);

    generateSubConfigurable("EvaluationRequirementGroup", instance);

    assert (hasGroup(name));

    if (widget_)
        endModelReset();

    emit groupsChangedSignal();
}

Group& EvaluationStandard::group (const std::string& name)
{
    assert (hasGroup(name));
    return *groups_.at(name);
}

void EvaluationStandard::removeGroup (const std::string& name)
{
    assert (hasGroup(name));

    if (widget_)
        beginModelReset();

    groups_.erase(name);

    if (widget_)
        endModelReset();

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

void EvaluationStandard::showMenu ()
{
    menu_.exec(QCursor::pos());
}

void EvaluationStandard::beginModelReset()
{
    widget()->model().beginReset();
}

void EvaluationStandard::endModelReset()
{
    widget()->model().endReset();
    widget()->expandAll();
}


void EvaluationStandard::addGroupSlot()
{
    loginf << "EvaluationRequirementGroup " << name_ << ": addGroupSlot: " << groups_.size() << " groups" ;

    bool ok;
    QString text =
        QInputDialog::getText(nullptr, tr("Group Name"),
                              tr("Specify a (unique) group name:"), QLineEdit::Normal, "", &ok);

    if (ok && !text.isEmpty())
    {
        std::string name = text.toStdString();

        if (!name.size())
        {
            QMessageBox m_warning(QMessageBox::Warning, "Adding Group Failed",
            "Group has to have a non-empty name.", QMessageBox::Ok);
            m_warning.exec();
            return;
        }

        if (hasGroup(name))
        {
            QMessageBox m_warning(QMessageBox::Warning, "Adding Group Failed",
            "Group with this name already exists.", QMessageBox::Ok);
            m_warning.exec();
            return;
        }

        addGroup(name);

        loginf << "EvaluationRequirementGroup " << name_ << ": addGroupSlot: added " << name << ", "
               << groups_.size() << " groups" ;
    }
}


