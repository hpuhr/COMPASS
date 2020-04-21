/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
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

/**
 * Initializes members, registers parameters and creates sub-configurables if class id is DBFilter.
 */
DBFilter::DBFilter(const std::string& class_id, const std::string& instance_id,
                   Configurable* parent, bool is_generic)
    : Configurable(class_id, instance_id, parent),
      is_generic_(is_generic)  // filter_manager_(*filter_manager),
{
    registerParameter("active", &active_, false);
    registerParameter("op_and_", &op_and_, true);
    registerParameter("changed", &changed_, false);
    registerParameter("visible", &visible_, false);
    registerParameter("name", &name_, instance_id);

    if (classId().compare("DBFilter") == 0)  // else do it in subclass
        createSubConfigurables();
}

/**
 * Deletes and removes sub-filters, deletes and removes conditions, deletes widget if required.
 */
DBFilter::~DBFilter()
{
    logdbg << "DBFilter: destructor: instance_id " << instanceId();

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    for (unsigned int cnt = 0; cnt < sub_filters_.size(); cnt++)
    {
        delete sub_filters_.at(cnt);
    }
    sub_filters_.clear();

    for (unsigned int cnt = 0; cnt < conditions_.size(); cnt++)
    {
        delete conditions_.at(cnt);
    }
    conditions_.clear();
}

/**
 * Sets changed in FilterManager if required, overwrites active_ and distributes the change to the
 * sub-filters. Updates widget if existing.
 */
void DBFilter::setActive(bool active)
{
    //    if (active_ && !active)
    //        FilterManager::getInstance().setChanged();

    assert(!disabled_);

    active_ = active;

    for (unsigned int cnt = 0; cnt < sub_filters_.size(); cnt++)
    {
        sub_filters_.at(cnt)->setActive(active_);
    }

    changed_ = true;

    if (widget_)
        widget_->update();
}
bool DBFilter::getActive() { return active_; }

/**
 * Returns if any change occurred in this filter, a sub-condition or a sub-filter.
 */
bool DBFilter::getChanged()
{
    assert(!disabled_);

    bool ret = changed_;

    for (unsigned int cnt = 0; cnt < conditions_.size(); cnt++)
    {
        ret |= conditions_.at(cnt)->getChanged();
    }

    for (unsigned int cnt = 0; cnt < sub_filters_.size(); cnt++)
    {
        ret |= sub_filters_.at(cnt)->getChanged();
    }

    return ret;
}

/**
 * Sets changed_ and propagates function to all sub-conditions and sub-filters.
 *
 */
void DBFilter::setChanged(bool changed)
{
    assert(!disabled_);

    changed_ = changed;

    for (unsigned int cnt = 0; cnt < conditions_.size(); cnt++)
    {
        conditions_.at(cnt)->setChanged(changed);
    }

    for (unsigned int cnt = 0; cnt < sub_filters_.size(); cnt++)
    {
        sub_filters_.at(cnt)->setChanged(changed);
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

void DBFilter::addSubFilter(DBFilter* filter) { sub_filters_.push_back(filter); }

/**
 * Checks if any sub-filter or sub-condition filter the DBO of the supplied type.
 */
bool DBFilter::filters(const std::string& dbo_type)
{
    if (disabled_)
        return false;

    bool ret = false;

    for (unsigned int cnt = 0; cnt < conditions_.size(); cnt++)
    {
        ret |= conditions_.at(cnt)->filters(dbo_type);
    }

    for (unsigned int cnt = 0; cnt < sub_filters_.size(); cnt++)
    {
        ret |= sub_filters_.at(cnt)->filters(dbo_type);
    }

    logdbg << "DBFilter: filters: object " << dbo_type << " " << ret;

    return ret;
}

/**
 * If active, returns concatenated condition strings from all sub-conditions and sub-filters, else
 * returns empty string.
 */
std::string DBFilter::getConditionString(const std::string& dbo_name, bool& first,
                                         std::vector<DBOVariable*>& filtered_variables)
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
                conditions_.at(cnt)->getConditionString(dbo_name, first, filtered_variables);
            ss << text;
        }

        for (unsigned int cnt = 0; cnt < sub_filters_.size(); cnt++)
        {
            std::string text =
                sub_filters_.at(cnt)->getConditionString(dbo_name, first, filtered_variables);
            ss << text;
        }
    }

    loginf << "DBFilter " << instanceId() << ": getConditionString: object " << dbo_name
           << " here '" << ss.str() << "' first " << first;

    return ss.str();
}

void DBFilter::setAnd(bool op_and)
{
    assert(!disabled_);

    if (op_and_ != op_and)
    {
        op_and_ = op_and;

        changed_ = true;

        if (widget_)
            widget_->update();
    }
}

void DBFilter::invert()
{
    assert(!disabled_);

    op_and_ = !op_and_;

    for (unsigned int cnt = 0; cnt < conditions_.size(); cnt++)
    {
        conditions_.at(cnt)->invert();
    }

    for (unsigned int cnt = 0; cnt < sub_filters_.size(); cnt++)
    {
        sub_filters_.at(cnt)->invert();
    }
    changed_ = true;

    if (widget_)
        widget_->update();
}

/**
 * Can generate instances of DBFilterWidget, DBFilterCondition or DBFilter.
 *
 * \exception std::runtime_error if unknown class_id
 */
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
    else if (class_id == "DBFilter")
    {
        DBFilter* filter = new DBFilter(class_id, instance_id, this);
        addSubFilter(filter);
    }
    else
        throw std::runtime_error("DBFilter: generateSubConfigurable: unknown class_id " + class_id);
}

/**
 * Creates widget_ if required and adds all sub-filter widgets to it.
 */
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

    // TODO
    for (unsigned int cnt = 0; cnt < sub_filters_.size(); cnt++)
    {
        DBFilterWidget* filter = sub_filters_.at(cnt)->widget();
        QObject::connect((QWidget*)filter, SIGNAL(possibleFilterChange()), (QWidget*)widget_,
                         SLOT(possibleSubFilterChange()));
        widget_->addChildWidget(filter);
    }
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

    for (unsigned int cnt = 0; cnt < sub_filters_.size(); cnt++)
    {
        sub_filters_.at(cnt)->reset();
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

void DBFilter::saveViewPointConditions (nlohmann::json& cond_array)
{
    for (auto cond_it : conditions_)
    {
        nlohmann::json& condition = cond_array[cond_array.size()];
        condition["name"] = cond_it->instanceId();
        condition["value"] = cond_it->getValue();
    }
}

void DBFilter::loadViewPointConditions (nlohmann::json& cond_array)
{
    assert (cond_array.is_array());

    for (auto& cond_it : cond_array.get<json::array_t>())
    {

        assert (cond_it.contains("name"));
        assert (cond_it.contains("value"));

        const std::string& cond_name = cond_it.at("name");

        auto it = find_if(conditions_.begin(), conditions_.end(),
                          [cond_name] (const DBFilterCondition* c) { return c->instanceId() == cond_name; } );

        assert (it != conditions_.end());
        (*it)->setValue(cond_it.at("value"));

    }
}
