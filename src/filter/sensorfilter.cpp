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

/*
 * SensorFilter.cpp
 *
 *  Created on: Nov 4, 2012
 *      Author: sk
 */

#include "atsdb.h"
#include "sensorfilter.h"
#include "sensorfilterwidget.h"
#include "logger.h"
#include "dbobjectmanager.h"
#include "dbobject.h"

#include "stringconv.h"

using namespace Utils;

SensorFilter::SensorFilter(const std::string &class_id, const std::string &instance_id, Configurable *parent)
: DBFilter(class_id, instance_id, parent, false)
{
    registerParameter ("dbo_name", &dbo_name_, "");

    assert (ATSDB::instance().objectManager().existsObject(dbo_name_));
    object_ = &ATSDB::instance().objectManager().object(dbo_name_);
    assert (object_->hasCurrentDataSource());

    sensor_column_name_ = object_->currentDataSource().localKey();

    updateDataSources ();

    if (object_->hasActiveDataSourcesInfo())
        updateDataSourcesActive();

    createSubConfigurables();

    assert (widget_);
    if (object_->count() == 0)
    {
        active_=false;
        widget_->setInvisible();
        widget_->update();
        widget_->setDisabled(true);
    }
}

SensorFilter::~SensorFilter()
{
}

std::string SensorFilter::getConditionString (const std::string &dbo_name, bool &first, std::vector <DBOVariable*>& filtered_variables)
{
    logdbg  << "SensorFilter: getConditionString ";

    std::stringstream ss;

    if (active_)
    {
        if (dbo_name == dbo_name_)
        {
            assert (object_->hasVariable(sensor_column_name_));
            filtered_variables.push_back(&object_->variable(sensor_column_name_));

            bool got_one=false;
            bool got_all=true;

            std::stringstream values;

            std::map<int, SensorFilterDataSource>::iterator it;

            for (it = data_sources_.begin(); it != data_sources_.end(); it++)
            {
                if (it->second.isActiveInFilter()) // in selection
                {
                    if (values.str().size() > 0)
                        values << ",";
                    values << it->first;
                    got_one=true;
                }
                else
                    got_all=false;
            }

            if (got_all)
                return ss.str();

            if (!first)
            {
                ss << " AND";
            }

            //WHERE column_name IN (value1,value2,...)
            if (got_one)
                ss << " " <<sensor_column_name_ << " IN (" << values.str() << ")";
            else
            {
                ss  << " "+sensor_column_name_+" IS NULL";
            }
            first=false;
        }
    }

    logdbg  << "SensorFilter: getConditionString: here '" <<ss.str() << "'";

    return ss.str();
}


void SensorFilter::updateDataSources ()
{
    if (!object_->hasDataSources ())
    {
        logerr  << "SensorFilter: updateDataSources: type " << dbo_name_ << " has no data sources";
        return;
    }

    for (auto src_it : object_->dataSources())
    {
        if (data_sources_.find(src_it.first) == data_sources_.end())
        {
            data_sources_[src_it.first].setNumber(src_it.first);
            data_sources_[src_it.first].setName(src_it.second);
            data_sources_[src_it.first].setActiveInFilter(true);
            data_sources_[src_it.first].setActiveInData(true);

            registerParameter ("LoadSensorNumber"+std::to_string(src_it.first), &data_sources_[src_it.first].getActiveInFilterReference(), true);
        }
    }
}

void SensorFilter::updateDataSourcesActive ()
{
    loginf << "SensorFilter: updateDataSourcesActive";

    if (!object_->hasActiveDataSourcesInfo())
    {
        logerr  << "SensorFilter: updateDataSourcesActive: type " << object_->name() << " has no active data sources info";
        return;
    }

    std::map<int, SensorFilterDataSource>::iterator srcit;
    for (srcit =  data_sources_.begin(); srcit !=  data_sources_.end(); srcit++)
        srcit->second.setActiveInData(false);

    std::set<int> active_sources = object_->getActiveDataSources();
    std::set<int>::iterator it;

    for (it = active_sources.begin(); it != active_sources.end(); it++)
    {
        assert (data_sources_.find(*it) != data_sources_.end());
        SensorFilterDataSource &src = data_sources_[*it];
        src.setActiveInData(true);
    }

    for (srcit =  data_sources_.begin(); srcit !=  data_sources_.end(); srcit++)
    {
        if (!srcit->second.isActiveInData())
            srcit->second.setActiveInFilter(false);
    }
}


void SensorFilter::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    logdbg  << "SensorFilter: generateSubConfigurable: class_id " << class_id ;

    if (class_id.compare("SensorFilterWidget") == 0)
    {
        assert (!widget_);
        widget_ = new SensorFilterWidget (*this, class_id, instance_id);
    }
    else
        throw std::runtime_error ("SensorFilter: generateSubConfigurable: unknown class_id "+class_id);
}

void SensorFilter::checkSubConfigurables ()
{
    logdbg  << "SensorFilter: checkSubConfigurables";

    if (!widget_)
    {
        logdbg  << "SensorFilter: checkSubConfigurables: generating my filter widget";
        widget_ = new SensorFilterWidget (*this, "SensorFilterWidget", instance_id_+"Widget0");
    }
    assert (widget_);

    for (unsigned int cnt=0; cnt < sub_filters_.size(); cnt++)
    {
        DBFilterWidget *filter = sub_filters_.at(cnt)->widget();
        QObject::connect((QWidget*)filter, SIGNAL( possibleFilterChange() ), (QWidget*)widget_, SLOT( possibleSubFilterChange() ));
        widget_->addChildWidget (filter);
    }
}

void SensorFilter::reset ()
{
    std::map<int, SensorFilterDataSource>::iterator it;
    for (it = data_sources_.begin(); it != data_sources_.end(); it++)
    {
        it->second.setActiveInFilter(true);
    }
    widget_->update();
}

//void SensorFilter::notifyActiveSources ()
//{
//    logdbg << "SensorFilter: notifyActiveSources";
//    updateDataSourcesActive();

//    if (widget_)
//        widget_->update();
//}

