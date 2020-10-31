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

#include "dboeditdatasourceaction.h"

#include "compass.h"
#include "dbobject.h"
#include "logger.h"
#include "managedatasourcestask.h"
#include "stringconv.h"
#include "taskmanager.h"

using namespace Utils;

DBOEditDataSourceAction::DBOEditDataSourceAction(const std::string& action,
                                                 const std::string& target_type,
                                                 const std::string& target_id)
    : action_(action), target_type_(target_type), target_id_(target_id)
{
    //    std::string action_; // "None", "Add", "Overwrite"
    assert(action_ == "None" || action_ == "Add" || action_ == "Overwrite");
    //    std::string target_type_; // "Config", "DB"
    assert(target_type_ == "" || target_type_ == "Config" || target_type_ == "DB");
    //    std::string target_id_; // "New" ("Add"),id ("Overwrite")
    assert(target_id_ == "" || target_id_ == "New" || String::isNumber(target_id_));

    if (action_ == "None")
        assert(target_type_ == "" && target_id_ == "");
    if (action_ == "Add")
        assert(target_type_ != "" && target_id_ == "New");
    if (action == "Overwrite")
        assert(target_type_ != "" && String::isNumber(target_id_));

    refreshActionString();
}

void DBOEditDataSourceAction::perform(DBObject& object, const std::string& source_type,
                                      const std::string& source_id)
{
    loginf << "DBOEditDataSourceAction: perform: object src " << source_type << ", " << source_id
           << " action " << action_ << " tgt " << target_type_ << ", " << target_id_;

    assert(action_ != "None");

    ManageDataSourcesTask& manage_ds_task = COMPASS::instance().taskManager().manageDataSourcesTask();

    std::string dbo_name = object.name();

    if (action_ == "Add")
    {
        if (source_type == "DB" && target_type_ == "Config")
        {
            assert(String::isNumber(source_id));
            unsigned int id = std::stoi(source_id);
            assert(object.hasDataSource(id));
            manage_ds_task.addNewStoredDataSource(dbo_name) = object.getDataSource(id);
        }
        else
            logerr << "DBOEditDataSourceAction: perform: unsupported action Add src " << source_type
                   << " tgt " << target_type_;
    }
    else if (action_ == "Overwrite")
    {
        if (source_type == "DB" && target_type_ == "Config")
        {
            assert(String::isNumber(source_id));
            unsigned int src_id = std::stoi(source_id);
            assert(object.hasDataSource(src_id));

            assert(String::isNumber(target_id_));
            unsigned int tgt_id = std::stoi(target_id_);
            assert(manage_ds_task.hasStoredDataSource(dbo_name, tgt_id));

            manage_ds_task.storedDataSource(object.name(), tgt_id) = object.getDataSource(src_id);
        }
        else if (source_type == "Config" && target_type_ == "DB")
        {
            assert(String::isNumber(source_id));
            unsigned int src_id = std::stoi(source_id);
            assert(manage_ds_task.hasStoredDataSource(dbo_name, src_id));

            assert(String::isNumber(target_id_));
            unsigned int tgt_id = std::stoi(target_id_);
            assert(object.hasDataSource(tgt_id));

            object.getDataSource(tgt_id) = manage_ds_task.storedDataSource(dbo_name, src_id);
            object.updateDataSource(tgt_id);
        }
        else
            logerr << "DBOEditDataSourceAction: perform: unsupported action Add src " << source_type
                   << " tgt " << target_type_;
    }
}

std::string DBOEditDataSourceAction::targetType() const { return target_type_; }

void DBOEditDataSourceAction::refreshActionString()
{
    action_str_ = action_;

    if (action_ != "None")
    {
        if (action_ == "Add")
        {
            action_str_ += " new id";  // " to "+target_type_+
        }
        else if ("Overwrite")
        {
            action_str_ += " id " + target_id_;  // " in "+target_type_+
        }
    }
}
