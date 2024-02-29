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

#include "dbospecificvaluesdbfilter.h"
//#include "dbospecificvaluesdbfilterwidget.h"
#include "compass.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variable.h"
#include "logger.h"
//#include "stringconv.h"
#include "dbfiltercondition.h"
#include "global.h"

using namespace Utils;
using namespace nlohmann;
using namespace std;

DBOSpecificValuesDBFilter::DBOSpecificValuesDBFilter(const std::string& class_id, const std::string& instance_id,
                                                     Configurable* parent)
    : DBFilter(class_id, instance_id, parent, false)
{
    logdbg << "DBOSpecificValuesDBFilter: constructor";

    registerParameter("dbcontent_name", &dbcontent_name_, std::string());
    registerParameter("variable_name", &variable_name_, std::string());
    registerParameter("condition_operator", &condition_operator_, std::string());

    // dbobject
    if (!COMPASS::instance().dbContentManager().existsDBContent(dbcontent_name_))
        throw std::invalid_argument("DataSourcesFilter: DataSourcesFilter: instance " +
                                    instance_id + " has non-existing object " + dbcontent_name_);

    object_ = &COMPASS::instance().dbContentManager().dbContent(dbcontent_name_);
    assert (object_);

    TODO_ASSERT

//    if (!object_->hasCurrentDataSourceDefinition())
//    {
//        logerr << "DataSourcesFilter: DataSourcesFilter: instance " + instance_id + " object "
//               << dbcontent_name_ + " has no data sources";
//        disabled_ = true;
//        return;
//    }

//    if (!object_->hasDataSources())
//    {
//        disabled_ = true;
//        return;
//    }

//    if (!object_->existsInDB())
//    {
//        disabled_ = true;
//        return;
//    }

//    ds_column_name_ = object_->currentDataSourceDefinition().localKey();

    // variable
    assert (variable_name_.size());
    if (!object_->hasVariable(variable_name_))
        throw std::invalid_argument("DataSourcesFilter: DataSourcesFilter: instance " +
                                    instance_id + " has non-existing variable " + variable_name_);

    variable_ = &object_->variable(variable_name_);
    assert (variable_);

    assert (condition_operator_.size());

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

DBOSpecificValuesDBFilter::~DBOSpecificValuesDBFilter() {}

bool DBOSpecificValuesDBFilter::filters(const std::string& dbo_type) { return dbcontent_name_ == dbo_type; }

std::string DBOSpecificValuesDBFilter::getConditionString(const std::string& dbcontent_name, bool& first)
{
    assert(!unusable_);

    TODO_ASSERT
    //assert (object_->hasDataSources());

    std::stringstream ss;

    bool condition_set = false;

    if (active_)
    {
        if (dbcontent_name == dbcontent_name_)
        {
            if (!first)
            {
                ss << " AND ";
            }

            ss << "("; // first condition

            for (unsigned int cnt = 0; cnt < conditions_.size(); cnt++)
            {
                if (!conditions_.at(cnt)->getValue().size())
                    continue; // skip for empty strings

                if (conditions_.at(cnt)->valueInvalid())
                {
                    logwrn << "DBOSpecificValuesDBFilter " << instanceId()
                           << ": getConditionString: invalid condition, will be skipped";
                    continue;
                }

                if (condition_set)
                    ss << " OR ";

                string cond_name = conditions_.at(cnt)->instanceId();

                TODO_ASSERT

//                DBContent:: DataSourceIterator it = find_if(object_->dsBegin(), object_->dsEnd(),
//                                                           [cond_name, this] (const pair<int, DBODataSource>& s) {
//                    return s.second.hasShortName() ?
//                                (s.second.shortName()+" "+variable_name_) == cond_name
//                              : (s.second.name()+" "+variable_name_) == cond_name; } );

//                DBContent:: DataSourceIterator it = find_if(object_->dsBegin(), object_->dsEnd(),
//                                                           [cond_name, this] (const pair<int, DBODataSource>& s) {
//                    return (s.second.name()+" "+variable_name_) == cond_name; } );

//                assert (it != object_->dsEnd());
//                int ds_id = it->first;

//                if (conditions_.at(cnt)->getValue() == "*")
//                {
//                    ss << "(" << ds_column_name_ << "=" << ds_id << ")";
//                }
//                else
//                {
//                    bool cond_first = true;
//                    std::string text =
//                            conditions_.at(cnt)->getConditionString(dbcontent_name, cond_first, filtered_variables);

//                    ss << "(" << ds_column_name_ << "=" << ds_id << " AND " << text << ")";
//                }


//                if (!condition_set) // first time only
//                    filtered_variables.push_back(&object_->variable(ds_column_name_));

                condition_set = true;
                first = false;
            }

            ss << ")"; // there be conditions


            //        for (unsigned int cnt = 0; cnt < sub_filters_.size(); cnt++)
            //        {
            //            std::string text =
            //                sub_filters_.at(cnt)->getConditionString(dbcontent_name, first, filtered_variables);
            //            ss << text;
            //        }
        }
    }

    logdbg << "DBOSpecificValuesDBFilter " << instanceId() << ": getConditionString: object " << dbcontent_name
           << " here '" << ss.str() << "' first " << first << " condition_set " << condition_set;

    if (condition_set)
        return ss.str();
    else
        return "";
}

void DBOSpecificValuesDBFilter::checkSubConfigurables()
{
    logdbg << "DBOSpecificValuesDBFilter: checkSubConfigurables";

    // widget

    TODO_ASSERT

    // data sources
    //assert (object_->hasDataSources());

    // find and delete outdated data source conditions
    vector<DBFilterCondition*> conditions_to_delete;

    for (auto cond_it : conditions_)
    {
        string cond_name = cond_it->instanceId();

//        if (find_if(object_->dsBegin(), object_->dsEnd(),
//                    [cond_name, this] (const pair<int, DBODataSource>& s) {
//                    return s.second.hasShortName() ?
//                    (s.second.shortName()+" "+variable_name_) == cond_name
//                    : (s.second.name()+" "+variable_name_) == cond_name; } )
//                == object_->dsEnd())
//            conditions_to_delete.push_back(cond_it);

//        if (find_if(object_->dsBegin(), object_->dsEnd(),
//                    [cond_name, this] (const pair<int, DBODataSource>& s) {
//                    return (s.second.name()+" "+variable_name_) == cond_name; } )
//                == object_->dsEnd())
//            conditions_to_delete.push_back(cond_it);
    }

    for (auto cond_it : conditions_to_delete)
    {
        logdbg << "DBOSpecificValuesDBFilter: checkSubConfigurables: deleting condition " << cond_it->instanceId();
        deleteCondition(cond_it);
    }

    conditions_to_delete.clear();

    TODO_ASSERT

    // create new ones for data sources
//    for (auto ds_it = object_->dsBegin(); ds_it != object_->dsEnd(); ++ds_it)
//    {
//        string ds_name = ds_it->second.name();
////        if (ds_it->second.hasShortName())
////            ds_name = ds_it->second.shortName();

//        ds_name += " "+variable_name_;

//        auto it = find_if(conditions_.begin(), conditions_.end(),
//                          [ds_name] (const DBFilterCondition* s) { return s->instanceId() == ds_name; } );

//        if (it == conditions_.end()) // add
//        {
//            logdbg << "DBOSpecificValuesDBFilter: checkSubConfigurables: creating new condition " << ds_name;
//            Configuration& config = addNewSubConfiguration("DBFilterCondition", ds_name);
//            config.addParameter<std::string>("reset_value", "4227");
//            config.addParameter<std::string>("value", "4227");
//            config.addParameter<std::string>("variable_dbcontent_name", dbcontent_name_);
//            config.addParameter<std::string>("variable_name", variable_name_);
//            config.addParameter<bool>("op_and", false);
//            config.addParameter<std::string>("operator", condition_operator_);
//            config.addParameter<bool>("display_instance_id", true);
//            generateSubConfigurable("DBFilterCondition", config.getInstanceId());
//        }
//    }
}


