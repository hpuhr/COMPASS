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

#include "dboeditdatasourceactionoptions.h"
#include "dbobject.h"
#include "dbodatasource.h"
#include "storeddbodatasource.h"
#include "dboeditdatasourceactionoptionswidget.h"

DBOEditDataSourceActionOptions::DBOEditDataSourceActionOptions(DBObject& object, const std::string& source_type,
                                                               const std::string& source_id)
    : object_(&object), source_type_(source_type), source_id_(source_id)
{
    addPossibleAction ("None", "", "");
}

void DBOEditDataSourceActionOptions::addPossibleAction (const std::string& action, const std::string& target_type,
                                                        const std::string& target_id)
{
    possible_actions_[possible_actions_.size()] = DBOEditDataSourceAction(action, target_type, target_id);
}

bool DBOEditDataSourceActionOptions::performFlag() const
{
    return perform_;
}

void DBOEditDataSourceActionOptions::performFlag(bool perform_flag)
{
    perform_ = perform_flag;

    if (widget_)
        widget_->update();
}

std::string DBOEditDataSourceActionOptions::sourceType() const
{
    return source_type_;
}

void DBOEditDataSourceActionOptions::sourceType(const std::string &source_type)
{
    source_type_ = source_type;
}

std::string DBOEditDataSourceActionOptions::sourceId() const
{
    return source_id_;
}

void DBOEditDataSourceActionOptions::sourceId(const std::string &source_id)
{
    source_id_ = source_id;
}

unsigned int DBOEditDataSourceActionOptions::currentActionId() const
{
    return current_action_id_;
}

void DBOEditDataSourceActionOptions::currentActionId(unsigned int current_action_id)
{
    current_action_id_ = current_action_id;
}

DBOEditDataSourceActionOptionsWidget* DBOEditDataSourceActionOptions::widget ()
{
    if (!widget_)
    {
        widget_.reset(new DBOEditDataSourceActionOptionsWidget(*this));
    }
    return widget_.get();
}

void DBOEditDataSourceActionOptions::perform ()
{
    assert (object_);
    assert (perform_);

    DBOEditDataSourceAction& selected_action = possible_actions_.at(current_action_id_);

    loginf << "DBOEditDataSourceActionOptions: perform: obj " << object_->name() << " src " << source_type_
           << " src_id " << source_id_ << " action " << selected_action.getActionString();

    selected_action.perform(*object_, source_type_, source_id_);
}

namespace DBOEditDataSourceActionOptionsCreator
{

DBOEditDataSourceActionOptions getSyncOptionsFromDB (DBObject& object, DBODataSource& source)
{
    DBOEditDataSourceActionOptions options {object, "DB", std::to_string(source.id())};
    options.addPossibleAction("Add", "Config", "New");

    bool found_equivalent = false;

    for (auto stored_it = object.storedDSBegin(); stored_it != object.storedDSEnd(); ++stored_it)
    {
        if (stored_it->second.sac() == source.sac() && stored_it->second.sic() == source.sic())
        {
            if (stored_it->second != source)
                options.addPossibleAction("Overwrite", "Config", std::to_string(stored_it->second.id()));
            else
                found_equivalent = true;
        }
    }

    assert (options.numOptions() > 0);

    if (found_equivalent)
    {
        options.currentActionId(0);
        options.performFlag(false);
    }
    else
    {
        options.currentActionId(options.numOptions()-1);
        options.performFlag(true);
    }

    return options;
}

DBOEditDataSourceActionOptions getSyncOptionsFromCfg (DBObject& object, StoredDBODataSource& source)
{
    DBOEditDataSourceActionOptions options {object, "Config", std::to_string(source.id())};
    //options.addPossibleAction("Add", "DB", "New");

    for (auto stored_it = object.dsBegin(); stored_it != object.dsEnd(); ++stored_it)
    {
        if (stored_it->second.sac() == source.sac() && stored_it->second.sic() == source.sic())
        {
            if (stored_it->second != source)
                options.addPossibleAction("Overwrite", "DB", std::to_string(stored_it->second.id()));
        }
    }

    assert (options.numOptions() > 0);
    options.currentActionId(options.numOptions()-1);
    options.performFlag(options.currentActionId() != 0);

    return options;
}

}
