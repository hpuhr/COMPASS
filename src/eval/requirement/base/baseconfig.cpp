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

#include "eval/requirement/base/baseconfig.h"
#include "eval/requirement/group.h"
#include "logger.h"

#include <QFormLayout>
#include <QLineEdit>

using namespace std;

namespace EvaluationRequirement
{

BaseConfig::BaseConfig(
        const std::string& class_id, const std::string& instance_id,
        Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
    : Configurable(class_id, instance_id, &group), EvaluationStandardTreeItem(&group),
      group_(group), standard_(standard), eval_man_(eval_man)
{
    registerParameter("name", &name_, "");
    registerParameter("short_name", &short_name_, "");

    assert (name_.size());
    assert (short_name_.size());

    createSubConfigurables();
}

BaseConfig::~BaseConfig()
{

}


void BaseConfig::generateSubConfigurable(const std::string& class_id,
                                                          const std::string& instance_id)
{
    assert(false);
}

std::string BaseConfig::name() const
{
    return name_;
}

void BaseConfig::checkSubConfigurables()
{
}

void BaseConfig::addGUIElements(QFormLayout* layout)
{
    assert (layout);

    QLineEdit* name_edit = new QLineEdit (name_.c_str());
    connect(name_edit, &QLineEdit::textEdited, this, &BaseConfig::changedNameSlot);

    layout->addRow("Name", name_edit);

    QLineEdit* short_name_edit = new QLineEdit (short_name_.c_str());
    connect(short_name_edit, &QLineEdit::textEdited, this, &BaseConfig::changedShortNameSlot);

    layout->addRow("Short Name", short_name_edit);
}

EvaluationStandardTreeItem* BaseConfig::child(int row)
{
    return nullptr;
}

int BaseConfig::childCount() const
{
    return 0;
}

int BaseConfig::columnCount() const
{
    return 1;
}

QVariant BaseConfig::data(int column) const
{
    assert (column == 0);

    return name_.c_str();
}

int BaseConfig::row() const
{
    return 0;
}

void BaseConfig::changedNameSlot(const QString& value)
{
    string value_str = value.toStdString();

    loginf << "EvaluationRequirementConfig: changedNameSlot: name '" << value_str << "'";

    name_ = value_str;
}

void BaseConfig::changedShortNameSlot(const QString& value)
{
    string value_str = value.toStdString();

    loginf << "EvaluationRequirementConfig: changedShortNameSlot: name '" << value_str << "'";

    short_name_ = value_str;
}

void BaseConfig::name(const std::string& name)
{
    name_ = name;
}

void BaseConfig::shortName(const std::string& short_name)
{
    short_name_ = short_name;
}

std::string BaseConfig::shortName() const
{
    return short_name_;
}

}
