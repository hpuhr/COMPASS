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

#include "datasourcesfilter.h"

#include "atsdb.h"
#include "datasourcesfilterwidget.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbovariable.h"
#include "logger.h"
#include "stringconv.h"

using namespace Utils;
using namespace nlohmann;
using namespace std;

DataSourcesFilter::DataSourcesFilter(const std::string& class_id, const std::string& instance_id,
                                     Configurable* parent)
    : DBFilter(class_id, instance_id, parent, false)
{
    registerParameter("dbo_name", &dbo_name_, "");
    registerParameter("active_sources", &active_sources_, json::object());

    if (!ATSDB::instance().objectManager().existsObject(dbo_name_))
        throw std::invalid_argument("DataSourcesFilter: DataSourcesFilter: instance " +
                                    instance_id + " has non-existing object " + dbo_name_);

    object_ = &ATSDB::instance().objectManager().object(dbo_name_);

    if (!object_->hasCurrentDataSourceDefinition())
    {
        logerr << "DataSourcesFilter: DataSourcesFilter: instance " + instance_id + " object "
               << dbo_name_ + " has no data sources";
        disabled_ = true;
        return;
    }

    if (!object_->hasDataSources())
    {
        disabled_ = true;
        return;
    }

    if (!object_->existsInDB())
    {
        disabled_ = true;
        return;
    }

    ds_column_name_ = object_->currentDataSourceDefinition().localKey();

    if (object_->hasDataSources())
        updateDataSources();

    if (object_->hasActiveDataSourcesInfo())
        updateDataSourcesActive();

    createSubConfigurables();

    assert(widget_);

    if (object_->count() == 0)
    {
        active_ = false;
        widget_->setInvisible();
        widget_->update();
        widget_->setDisabled(true);
    }
}

DataSourcesFilter::~DataSourcesFilter() {}

bool DataSourcesFilter::filters(const std::string& dbo_type) { return dbo_name_ == dbo_type; }

std::string DataSourcesFilter::getConditionString(const std::string& dbo_name, bool& first,
                                                  std::vector<DBOVariable*>& filtered_variables)
{
    logdbg << "DataSourcesFilter: getConditionString ";

    std::stringstream ss;

    if (active_)
    {
        if (dbo_name == dbo_name_)
        {
            assert(object_->hasVariable(ds_column_name_));

            if (!object_->existsInDB())
            {
                logwrn << "DataSourcesFilter: getConditionString: object " << object_->name()
                       << " not in db";
                return "";
            }

            if (!object_->variable(ds_column_name_).existsInDB())
            {
                logwrn << "DataSourcesFilter: getConditionString: variable " << ds_column_name_
                       << " not in db";
                return "";
            }

            filtered_variables.push_back(&object_->variable(ds_column_name_));

            bool got_one = false;
            bool got_all = true;

            std::stringstream values;

            std::map<int, DataSourcesFilterDataSource>::iterator it;

            for (auto& it : data_sources_)
            {
                if (it.second.isActiveInFilter())  // in selection
                {
                    if (values.str().size() > 0)
                        values << ",";
                    values << it.first;
                    got_one = true;
                }
                else
                    got_all = false;
            }

            if (got_all)
                return ss.str();

            if (!first)
            {
                ss << " AND";
            }

            // WHERE column_name IN (value1,value2,...)
            if (got_one)
                ss << " " << ds_column_name_ << " IN (" << values.str() << ")";
            else
            {
                ss << " " + ds_column_name_ + " IS NULL";
            }
            first = false;
        }
    }

    logdbg << "DataSourcesFilter: getConditionString: here '" << ss.str() << "'";

    return ss.str();
}

void DataSourcesFilter::updateDataSources()
{
    if (!object_->hasDataSources())
    {
        logerr << "DataSourcesFilter: updateDataSources: type " << dbo_name_
               << " has no data sources";
        return;
    }

    for (auto ds_it = object_->dsBegin(); ds_it != object_->dsEnd(); ++ds_it)
    {
        if (data_sources_.find(ds_it->first) == data_sources_.end())
        {
            if (!active_sources_.contains(to_string(ds_it->first)))
                active_sources_[to_string(ds_it->first)] = true; // init with default true

            data_sources_.emplace(std::piecewise_construct,
                                  std::forward_as_tuple(ds_it->first),  // args for key
                                  std::forward_as_tuple(ds_it->first, ds_it->second.name(),
                                                        active_sources_[to_string(ds_it->first)]));
        }
    }
}

void DataSourcesFilter::updateDataSourcesActive()
{
    logdbg << "DataSourcesFilter: updateDataSourcesActive";

    if (!object_->hasActiveDataSourcesInfo())
    {
        logerr << "DataSourcesFilter: updateDataSourcesActive: type " << object_->name()
               << " has no active data sources info";
        return;
    }

    for (auto& srcit : data_sources_)
        srcit.second.setActiveInData(false);

    for (auto& it : object_->getActiveDataSources())
    {
        assert(data_sources_.find(it) != data_sources_.end());
        DataSourcesFilterDataSource& src = data_sources_.at(it);
        src.setActiveInData(true);
    }

    for (auto& srcit : data_sources_)
    {
        if (!srcit.second.isActiveInData())
            srcit.second.setActiveInFilter(false);
    }
}

void DataSourcesFilter::generateSubConfigurable(const std::string& class_id,
                                                const std::string& instance_id)
{
    logdbg << "DataSourcesFilter: generateSubConfigurable: class_id " << class_id;

    if (class_id.compare("DataSourcesFilterWidget") == 0)
    {
        assert(!widget_);
        widget_ = new DataSourcesFilterWidget(*this, class_id, instance_id);
    }
    else
        throw std::runtime_error("DataSourcesFilter: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

void DataSourcesFilter::checkSubConfigurables()
{
    logdbg << "DataSourcesFilter: checkSubConfigurables";

    if (!widget_)
    {
        logdbg << "DataSourcesFilter: checkSubConfigurables: generating my filter widget";
        widget_ =
            new DataSourcesFilterWidget(*this, "DataSourcesFilterWidget", instanceId() + "Widget0");
    }
    assert(widget_);

    for (unsigned int cnt = 0; cnt < sub_filters_.size(); cnt++)
    {
        DBFilterWidget* filter_widget = sub_filters_.at(cnt)->widget();
        QObject::connect((QWidget*)filter_widget, SIGNAL(possibleFilterChange()), (QWidget*)widget_,
                         SLOT(possibleSubFilterChange()));
        widget_->addChildWidget(filter_widget);
    }
}

void DataSourcesFilter::reset()
{
    for (auto& it : data_sources_)
        it.second.setActiveInFilter(true);

    widget_->update();
}

void DataSourcesFilter::saveViewPointConditions (nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

    assert (!filters.contains(name_));
    filters[name_] = json::object();
    json& filter = filters.at(name_);

    filter["active_sources"] = json::array();
    json& values = filter.at("active_sources");

    unsigned int cnt=0;

    for (auto& ds_it : data_sources_)
    {
        if (ds_it.second.isActiveInFilter())
        {
            values[cnt] = ds_it.second.getNumber();
            ++cnt;
        }
    }
}

void DataSourcesFilter::loadViewPointConditions (nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

    assert (filters.contains(name_));
    json& filter = filters.at(name_);

    assert (filter.contains("active_sources"));
    json& active_sources = filter.at("active_sources");

    assert (active_sources.is_array());

    // disable all sources
    for (auto& ds_it : data_sources_)
        ds_it.second.setActiveInFilter(false);

    // set active sources
    for (auto& ds_it : active_sources.get<json::array_t>())
    {
        int number = ds_it;

        if (data_sources_.count(number))
            data_sources_.at(number).setActiveInFilter(true);
        else
            logwrn << "DataSourcesFilter: loadViewPointConditions: source " << number << " not found";
    }

    if (widget())
        widget()->update();
}


