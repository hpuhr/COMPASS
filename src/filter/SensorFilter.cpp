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

#include "ATSDB.h"
#include "SensorFilter.h"
#include "SensorFilterWidget.h"
#include "Logger.h"
#include "DBObjectManager.h"
#include "DBObject.h"

#include "String.h"

using namespace Utils;

SensorFilter::SensorFilter(std::string class_id, std::string instance_id, Configurable *parent)
: DBFilter(class_id, instance_id, parent, false)
{
    registerParameter ("dbo_type", &dbo_type_, "");

    assert (DBObjectManager::getInstance().existsDBObject (dbo_type_));
    DBObject *object = DBObjectManager::getInstance().getDBObject (dbo_type_);
    assert (object->hasCurrentDataSource());

    assert (false);
    // TODO FIX OBSERVER
//    object->addActiveSourcesObserver(this);

//    sensor_column_name_ = DBObjectManager::getInstance().getDBObject ((DB_OBJECT_TYPE) dbo_type_int_)->getCurrentDataSource()->getLocalKey();

//    updateDataSources ();

//    if (object->hasActiveDataSourcesInfo())
//        updateDataSourcesActive();

    createSubConfigurables();
}

SensorFilter::~SensorFilter()
{
    assert (false);
    // TODO FIX OBSERVER
//    DBObject *object = DBObjectManager::getInstance().getDBObject ((DB_OBJECT_TYPE) dbo_type_int_);
//    object->removeActiveSourcesObserver(this);
}

std::string SensorFilter::getConditionString (const std::string &dbo_type, bool &first, std::vector<std::string> &variable_names)
{
    logdbg  << "SensorFilter: getConditionString ";

    std::stringstream ss;

    if (active_)
    {

        if (dbo_type == dbo_type_)
        {
            bool got_radars=false;
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
                    got_radars=true;
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
            if (got_radars)
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
    if (!ATSDB::getInstance().hasDataSources (dbo_type_))
    {
        logerr  << "SensorFilter: updateDataSources: type " << dbo_type_ << " has no data sources";
        return;
    }

    std::map<int, std::string> &sources = ATSDB::getInstance().getDataSources (dbo_type_);
    std::map<int, std::string>::iterator it;

    for (it = sources.begin(); it != sources.end(); it++)
    {
        if (data_sources_.find(it->first) == data_sources_.end())
        {
            data_sources_[it->first].setNumber(it->first);
            data_sources_[it->first].setName(it->second);
            data_sources_[it->first].setActiveInFilter(true);
            data_sources_[it->first].setActiveInData(true);

            registerParameter ("LoadRadarNumber"+String::intToString(it->first), data_sources_[it->first].getActiveInFilterPointer(),
                    true);
        }
    }
}

void SensorFilter::updateDataSourcesActive ()
{
    logdbg << "SensorFilter: updateDataSourcesActive";

    DBObject *object = DBObjectManager::getInstance().getDBObject (dbo_type_);

    assert (false);
    // TODO FIX OBSERVER

//    if (!object->hasActiveDataSourcesInfo())
//    {
//        logerr  << "SensorFilter: updateDataSourcesActive: type " << dbo_type_int_ << " has no active data sources info";
//        return;
//    }

//    std::map<int, SensorFilterDataSource>::iterator srcit;
//    for (srcit =  data_sources_.begin(); srcit !=  data_sources_.end(); srcit++)
//        srcit->second.setActiveInData(false);

//    std::set<int> active_radars = object->getActiveDataSources();
//    std::set<int>::iterator it;

//    for (it = active_radars.begin(); it != active_radars.end(); it++)
//    {
//        assert (data_sources_.find(*it) != data_sources_.end());
//        SensorFilterDataSource &src = data_sources_[*it];
//        src.setActiveInData(true);
//    }

//    for (srcit =  data_sources_.begin(); srcit !=  data_sources_.end(); srcit++)
//    {
//        if (!srcit->second.isActiveInData())
//            srcit->second.setActiveInFilter(false);
//    }
}


void SensorFilter::generateSubConfigurable (std::string class_id, std::string instance_id)
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
        DBFilterWidget *filter = sub_filters_.at(cnt)->getWidget();
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

void SensorFilter::notifyActiveSources ()
{
    logdbg << "SensorFilter: notifyActiveSources";
    updateDataSourcesActive();

    if (widget_)
        widget_->update();
}

