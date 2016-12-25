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
 * FilterManager.cpp
 *
 *  Created on: Jan 15, 2012
 *      Author: sk
 */

#include "ConfigurationManager.h"
#include "DBConnectionInfo.h"
#include "DBFilter.h"
#include "DBObject.h"
#include "DBObjectManager.h"
#include "DBOVariable.h"
#include "ATSDB.h"
#include "FilterManager.h"
#include "Logger.h"
#include "SensorFilter.h"


FilterManager::FilterManager()
: Configurable ("FilterManager", "FilterManager0", 0, "conf/config_filter.xml")
{
    logdbg  << "FilterManager: constructor";
    changed_=false;

    registerParameter ("db_id", &db_id_, "");


    const std::map <std::string, DBObject*> &objects = DBObjectManager::getInstance().getDBObjects ();
    std::map <std::string, DBObject*>::const_iterator it;

    for (it = objects.begin(); it != objects.end(); it++)
    {
        std::string dbo_type = it->first;

        load_[dbo_type]= new bool (true);
        registerParameter ("Load"+ DBObjectManager::getInstance().getDBObject(it->first)->getInstanceId(), load_[dbo_type], true);
    }

    createSubConfigurables ();

    std::string tmpstr = ATSDB::getInstance().getDBInfo()->getIdString();
    replace(tmpstr.begin(), tmpstr.end(), ' ', '_');

    if (db_id_.compare (tmpstr) != 0)
    {
        loginf  << "FilterManager: constructor: different db id, resetting filters";
        reset();
        db_id_ = ATSDB::getInstance().getDBInfo()->getIdString();
        replace(db_id_.begin(), db_id_.end(), ' ', '_');
    }

}

FilterManager::~FilterManager()
{
    for (unsigned int cnt=0; cnt < filters_.size(); cnt++)
        delete filters_.at(cnt);
    filters_.clear();

    std::map <std::string, bool*>::iterator it;
    for (it=load_.begin(); it != load_.end(); it++)
        delete it->second;
    load_.clear();

}

void FilterManager::generateSubConfigurable (std::string class_id, std::string instance_id)
{
    if (class_id.compare ("DBFilter") == 0)
    {
        DBFilter *filter = new DBFilter (class_id, instance_id, this);
        filters_.push_back (filter);
    }
    else if (class_id.compare ("PlotSensorNumberFilter") == 0)
    {
        logerr  << "FilterManager: generateSubConfigurable: PlotSensorNumberFilter returned from the grave ";
    }
    else if (class_id.compare ("SensorFilter") == 0)
    {
        SensorFilter *filter = new SensorFilter (class_id, instance_id, this);
        filters_.push_back (filter);
    }
    else
        throw std::runtime_error ("FilterManager: generateSubConfigurable: unknown class_id "+class_id );
}
void FilterManager::checkSubConfigurables ()
{

    if (filters_.size() == 0)
    {
        loginf << "FilterManager: checkSubConfigurables: generating sensor filters";
        // sensor filters
        const std::map <std::string, DBObject*> &objects =  DBObjectManager::getInstance().getDBObjects ();
        std::map <std::string, DBObject*>::const_iterator it;

        for (it = objects.begin(); it != objects.end(); it++)
        {
            if (!it->second->hasCurrentDataSource())
                continue;

            std::string instance_id = "Sensors"+it->second->getName();
            Configuration &sensorfilter_configuration = addNewSubConfiguration ("SensorFilter", instance_id);
            sensorfilter_configuration.addParameterString ("dbo_type", it->first);
            generateSubConfigurable ("SensorFilter", instance_id);
        }

        // FIX META VARIABLES
        assert (false);

//        loginf << "FilterManager: checkSubConfigurables: generating frame time filter";
//        // frame time filter
//        if (DBObjectManager::getInstance().existsDBOVariable (DBO_UNDEFINED, "frame_time"))
//        {
//            DBOVariable *frame_time = DBObjectManager::getInstance().getDBOVariable (DBO_UNDEFINED, "frame_time");

//            Configuration &frametime_configuration = addNewSubConfiguration ("DBFilter", "FrameTime0");
//            Configuration &frametime_condition_configuration = frametime_configuration.addNewSubConfiguration ("DBFilterCondition", "FrameTime0Condition1");
//            frametime_condition_configuration.addParameterString ("operator", ">=");
//            frametime_condition_configuration.addParameterString ("variable_name", "frame_time");
//            frametime_condition_configuration.addParameterUnsignedInt ("variable_type", 0);
//            frametime_condition_configuration.addParameterString ("value", frame_time->getRepresentationFromValue(ATSDB::getInstance().getMinAsString (frame_time)));
//            frametime_condition_configuration.addParameterString ("reset_value", "MIN");

//            Configuration &frametime_condition_configuration2 = frametime_configuration.addNewSubConfiguration ("DBFilterCondition", "FrameTime0Condition2");
//            frametime_condition_configuration2.addParameterString ("operator", "<=");
//            frametime_condition_configuration2.addParameterString ("variable_name", "frame_time");
//            frametime_condition_configuration2.addParameterUnsignedInt ("variable_type", 0);
//            frametime_condition_configuration2.addParameterString ("value", frame_time->getRepresentationFromValue(ATSDB::getInstance().getMaxAsString (frame_time)));
//            frametime_condition_configuration2.addParameterString ("reset_value", "MAX");

//            generateSubConfigurable ("DBFilter", "FrameTime0");
//        }

//        loginf << "FilterManager: checkSubConfigurables: generating mode 3a filter";

//        // mode 3a code filter
//        if (DBObjectManager::getInstance().existsDBOVariable (DBO_UNDEFINED, "mode_3a_code"))
//        {
//            DBOVariable *mode_3a_code = DBObjectManager::getInstance().getDBOVariable (DBO_UNDEFINED, "mode_3a_code");

//            Configuration &modea_configuration = addNewSubConfiguration ("DBFilter", "ModeA0");
//            Configuration &modea_condition_configuration = modea_configuration.addNewSubConfiguration ("DBFilterCondition", "ModeA0Condition0");
//            modea_condition_configuration.addParameterString ("operator", "|=");
//            modea_condition_configuration.addParameterString ("variable_name", "mode_3a_code");
//            modea_condition_configuration.addParameterUnsignedInt ("variable_type", 0);
//            modea_condition_configuration.addParameterString ("value", mode_3a_code->getRepresentationFromValue(ATSDB::getInstance().getMaxAsString (mode_3a_code)));
//            modea_condition_configuration.addParameterString ("reset_value", "MAX");
//            generateSubConfigurable ("DBFilter", "ModeA0");
//        }
    }
}

bool FilterManager::getChanged ()
{
    for (unsigned int cnt=0; cnt < filters_.size(); cnt++)
    {
        if (filters_.at(cnt)->getActive())
            changed_ |= filters_.at(cnt)->getChanged();
    }
    return changed_;
}

void FilterManager::setChanged ()
{
    changed_=true;
}

void FilterManager::clearChanged ()
{
    for (unsigned int cnt=0; cnt < filters_.size(); cnt++)
    {
        if (filters_.at(cnt)->getActive())
            filters_.at(cnt)->setChanged(false);
    }
    changed_=false;
}

void FilterManager::setLoad (const std::string &dbo_type, bool show)
{
    assert (DBObjectManager::getInstance().existsDBObject (dbo_type));
    *load_.at(dbo_type)=show;
    changed_=true;
}

bool FilterManager::getLoad (const std::string &dbo_type)
{
    assert (DBObjectManager::getInstance().existsDBObject (dbo_type));
    return *load_.at(dbo_type);
}

std::string FilterManager::getSQLCondition (const std::string &dbo_type, std::vector<std::string> &variable_names)
{
    assert (DBObjectManager::getInstance().getDBObject(dbo_type)->isLoadable());
    //assert (type == DBO_PLOTS || type == DBO_SYSTEM_TRACKS || DBO_ADS_B || type == DBO_MLAT);

    std::string sql = getActiveFilterSQLCondition (dbo_type, variable_names);
    logdbg  << "FilterManager: getSQLCondition: type " << DBObjectManager::getInstance().getDBObject(dbo_type)->getName ()
            << " '" << sql << "'";
    return sql;
}

std::string FilterManager::getActiveFilterSQLCondition (const std::string &dbo_type, std::vector<std::string> &variable_names)
{
    std::stringstream ss;

    bool first=true;

    for (unsigned int cnt=0; cnt < filters_.size(); cnt++)
    {
        if (filters_.at(cnt)->getActive())
        {
            ss << filters_.at(cnt)->getConditionString (dbo_type, first, variable_names);
        }
    }

    return ss.str();
}

unsigned int FilterManager::getNumFilters ()
{
    return filters_.size();
}
DBFilter *FilterManager::getFilter (unsigned int index)
{
    assert (index < filters_.size());

    return filters_.at(index);
}

void FilterManager::reset ()
{
    for (unsigned int cnt=0; cnt < filters_.size(); cnt++)
    {
        filters_.at(cnt)->reset();
    }
}

void FilterManager::deleteFilter (DBFilter *filter)
{
    std::vector <DBFilter*>::iterator it;

    it = find (filters_.begin(), filters_.end(), filter);
    if (it == filters_.end())
        throw std::runtime_error ("FilterManager: deleteFilter: called with unknown filter");
    else
    {
        filters_.erase (it);
        delete filter;
    }

}

