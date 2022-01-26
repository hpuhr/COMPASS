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

#include "dbfilter.h"

#include <QVBoxLayout>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include "dbfiltercondition.h"
#include "dbfilterwidget.h"
#include "filtermanager.h"
#include "logger.h"

using namespace nlohmann;

DBFilter::DBFilter(const std::string& class_id, const std::string& instance_id,
                   Configurable* parent, bool is_generic)
    : Configurable(class_id, instance_id, parent),
      is_generic_(is_generic)  // filter_manager_(*filter_manager),
{
    registerParameter("active", &active_, false);
    registerParameter("changed", &changed_, false);
    registerParameter("visible", &visible_, false);
    registerParameter("name", &name_, instance_id);

    if (classId().compare("DBFilter") == 0)  // else do it in subclass
        createSubConfigurables();
}

DBFilter::~DBFilter()
{
    logdbg << "DBFilter: destructor: instance_id " << instanceId();

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    for (unsigned int cnt = 0; cnt < conditions_.size(); cnt++)
    {
        delete conditions_.at(cnt);
    }
    conditions_.clear();
}

void DBFilter::setActive(bool active)
{
    //    if (active_ && !active)
    //        FilterManager::getInstance().setChanged();

    assert(!disabled_);

    active_ = active;

    changed_ = true;

    if (widget_)
        widget_->update();
}

bool DBFilter::getActive() { return active_; }

bool DBFilter::getChanged()
{
    assert(!disabled_);

    bool ret = changed_;

    for (unsigned int cnt = 0; cnt < conditions_.size(); cnt++)
    {
        ret |= conditions_.at(cnt)->getChanged();
    }

    return ret;
}

void DBFilter::setChanged(bool changed)
{
    assert(!disabled_);

    changed_ = changed;

    for (unsigned int cnt = 0; cnt < conditions_.size(); cnt++)
    {
        conditions_.at(cnt)->setChanged(changed);
    }

}

bool DBFilter::getVisible() { return visible_; }
void DBFilter::setVisible(bool visible)
{
    assert(!disabled_);

    visible_ = visible;
}

void DBFilter::setName(const std::string& name)
{
    assert(!disabled_);

    name_ = name;

    if (widget_)
        widget_->update();
}

bool DBFilter::filters(const std::string& dbo_type)
{
    if (disabled_)
        return false;

    bool ret = false;

    for (unsigned int cnt = 0; cnt < conditions_.size(); cnt++)
    {
        ret |= conditions_.at(cnt)->filters(dbo_type);
    }

    logdbg << "DBFilter: filters: object " << dbo_type << " " << ret;

    return ret;
}


//  If active, returns concatenated condition strings from all sub-conditions and sub-filters, else
//  returns empty string.
std::string DBFilter::getConditionString(const std::string& dbo_name, bool& first,
                                         std::vector<std::string>& extra_from_parts,
                                         std::vector<dbContent::Variable*>& filtered_variables)
{
    assert(!disabled_);

    std::stringstream ss;

    if (active_)
    {
        for (unsigned int cnt = 0; cnt < conditions_.size(); cnt++)
        {
            if (conditions_.at(cnt)->valueInvalid())
            {
                logwrn << "DBFilter " << instanceId()
                       << ": getConditionString: invalid condition, will be skipped";
                continue;
            }

            std::string text =
                conditions_.at(cnt)->getConditionString(dbo_name, first, extra_from_parts, filtered_variables);
            ss << text;
        }

    }

    loginf << "DBFilter " << instanceId() << ": getConditionString: object " << dbo_name
           << " here '" << ss.str() << "' first " << first;

    return ss.str();
}

void DBFilter::generateSubConfigurable(const std::string& class_id, const std::string& instance_id)
{
    logdbg << "DBFilter: generateSubConfigurable: " << classId() << " instance " << instanceId();

    if (class_id == "DBFilterWidget")
    {
        logdbg << "DBFilter: generateSubConfigurable: generating widget";
        assert(!widget_);
        widget_ = new DBFilterWidget(class_id, instance_id, *this);
    }
    else if (class_id == "DBFilterCondition")
    {
        logdbg << "DBFilter: generateSubConfigurable: generating condition";
        DBFilterCondition* condition = new DBFilterCondition(class_id, instance_id, this);
        conditions_.push_back(condition);

        disabled_ = disabled_ | !condition->usable();

        if (widget_)
        {
            if (disabled_)  // bit of a hack. think about order of generation.
            {
                widget_->setInvisible();
                widget_->setDisabled(true);
            }
            else
                widget_->updateChildWidget();
        }
    }
    else
        throw std::runtime_error("DBFilter: generateSubConfigurable: unknown class_id " + class_id);
}

void DBFilter::checkSubConfigurables()
{
    logdbg << "DBFilter: checkSubConfigurables: " << classId();

    if (!widget_)
    {
        logdbg << "DBFilter: checkSubConfigurables: generating generic filter widget";
        widget_ = new DBFilterWidget("DBFilterWidget", instanceId() + "Widget0", *this);

        if (disabled_)
        {
            widget_->setInvisible();
            widget_->setDisabled(true);
        }
    }
    assert(widget_);
}

/**
 * Calls reset on all sub-conditions or sub-filters, sets changed_ flag.
 */
void DBFilter::reset()
{
    if (disabled_)
        return;

    for (unsigned int cnt = 0; cnt < conditions_.size(); cnt++)
    {
        conditions_.at(cnt)->reset();
    }

    changed_ = true;
}

void DBFilter::deleteCondition(DBFilterCondition* condition)
{
    std::vector<DBFilterCondition*>::iterator it =
        find(conditions_.begin(), conditions_.end(), condition);
    assert(it != conditions_.end());
    conditions_.erase(it);
    delete condition;
}

DBFilterWidget* DBFilter::widget()
{
    assert(widget_);
    return widget_;
}

void DBFilter::saveViewPointConditions (nlohmann::json& filters)
{
    assert (!filters.contains(name_));
    filters[name_] = json::object();
    json& filter = filters.at(name_);

    for (auto cond_it : conditions_)
    {
        assert (!filter.contains(cond_it->instanceId()));
        filter[cond_it->instanceId()] = cond_it->getValue();
    }
}

void DBFilter::loadViewPointConditions (const nlohmann::json& filters)
{
    assert (filters.is_object());
    assert (filters.contains(name_));
    const nlohmann::json& filter = filters.at(name_);
    assert (filter.is_object());

    // clear previous conditions
    for (auto cond_it : conditions_)
        cond_it->setValue("");

    for (auto& cond_it : filter.get<json::object_t>())
    {
        std::string cond_name = cond_it.first;
        logdbg << "DBFilter: loadViewPointConditions: cond_name '"
               << cond_name << "' value '" << cond_it.second.dump() << "'";

        assert (cond_it.second.is_string());
        std::string value = cond_it.second;

        auto it = find_if(conditions_.begin(), conditions_.end(),
                          [cond_name] (const DBFilterCondition* c) { return c->instanceId() == cond_name; } );

        if (it == conditions_.end())
            logerr << "DBFilter " << name_ << ": loadViewPointConditions: cond_name '" << cond_name << "' not found";
        else
            (*it)->setValue(value);
    }
}
