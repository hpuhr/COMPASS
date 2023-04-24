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

#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"

#include <QFormLayout>
#include <QLineEdit>

using namespace std;
using namespace EvaluationResultsReport;

namespace EvaluationRequirement
{

std::string comparisonTypeString(COMPARISON_TYPE type)
{
    if (type == LESS_THAN)
        return "<";
    else if (type == LESS_THAN_OR_EQUAL)
        return "<=";
    else if (type == GREATER_THAN)
        return ">";
    else if (type == GREATER_THAN_OR_EUQAL)
        return ">=";
    else
        throw std::runtime_error("EvaluationRequirement: comparisonTypeString: unkown type "
                                 + std::to_string((unsigned int) type));
}

std::string comparisonTypeLongString(COMPARISON_TYPE type)
{
    if (type == LESS_THAN)
        return "Less Than (<)";
    else if (type == LESS_THAN_OR_EQUAL)
        return "Less Than or Equal (<=)";
    else if (type == GREATER_THAN)
        return "Greater Than (>)";
    else if (type == GREATER_THAN_OR_EUQAL)
        return "Greater Than or Equal (>=)";
    else
        throw std::runtime_error("EvaluationRequirement: comparisonTypeString: unkown type "
                                 + std::to_string((unsigned int) type));
}

BaseConfig::BaseConfig(
        const std::string& class_id, const std::string& instance_id,
        Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
    : Configurable(class_id, instance_id, &group), EvaluationStandardTreeItem(&group),
      group_(group), standard_(standard), eval_man_(eval_man)
{
    registerParameter("name", &name_, "");
    registerParameter("short_name", &short_name_, "");
    registerParameter("comment", &comment_, "");

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

BaseConfigWidget* BaseConfig::widget()
{
    if (!widget_)
    {
        createWidget();
        assert (widget_);
    }

    return widget_.get();
}

std::string BaseConfig::comment() const
{
    return comment_;
}

void BaseConfig::comment(const std::string &comment)
{
    comment_ = comment;
}

void BaseConfig::createWidget()
{
    assert (!widget_);
    widget_.reset(new BaseConfigWidget(*this));
    assert (widget_);
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

void BaseConfig::name(const std::string& name)
{
    loginf << "BaseConfig: name: value '" << name << "'";

    name_ = name;

    emit group_.configsChangedSignal();
}

void BaseConfig::shortName(const std::string& short_name)
{
    loginf << "BaseConfig: shortName: value '" << short_name << "'";

    short_name_ = short_name;
}

std::string BaseConfig::shortName() const
{
    return short_name_;
}



void BaseConfig::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    Section& section = root_item->getSection("Appendix:Requirements:"+group_.name()+":"+name_);

   section.addTable("req_table", 3, {"Name", "Comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& table = section.getTable("req_table");

    table.addRow({"Name", "Requirement name", name_.c_str()}, nullptr);
    table.addRow({"Short Name", "Requirement short name", short_name_.c_str()}, nullptr);
    table.addRow({"Comment", "", comment_.c_str()}, nullptr);
    table.addRow({"Group", "Group name", group_.name().c_str()}, nullptr);

    // prob & check type added in subclass
}

}
