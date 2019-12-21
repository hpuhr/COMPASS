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

#ifndef DBOEDITDATASOURCEACTIONOPTIONS_H
#define DBOEDITDATASOURCEACTIONOPTIONS_H

#include <string>
#include <map>
#include <memory>

#include "dboeditdatasourceaction.h"

class DBObject;
class DBODataSource;
class StoredDBODataSource;
class DBOEditDataSourceActionOptionsWidget;

class DBOEditDataSourceActionOptions
{
public:
    DBOEditDataSourceActionOptions (DBObject& object, const std::string& sourceType, const std::string& sourceId);
    // none action is added by default with action_id 0
    DBOEditDataSourceActionOptions () = default;

    void addPossibleAction (const std::string& action, const std::string& target_type,
                            const std::string& target_id);

    using PossibleActionIterator = typename std::map<unsigned int, DBOEditDataSourceAction>::iterator;
    PossibleActionIterator begin() { return possible_actions_.begin(); }
    PossibleActionIterator end() { return possible_actions_.end(); }
    size_t numOptions () { return possible_actions_.size(); }

    bool performFlag() const;
    void performFlag(bool perform_flag);

    std::string sourceType() const;
    void sourceType(const std::string &source_type);

    std::string sourceId() const;
    void sourceId(const std::string &source_id);

    unsigned int currentActionId() const;
    void currentActionId(unsigned int current_action_id);

    DBOEditDataSourceAction& currentAction();

    DBOEditDataSourceActionOptionsWidget* widget ();

    void perform ();

private:
    DBObject* object_ {nullptr};

    bool perform_ {false};

    std::string source_type_; // "cfg", "db"
    std::string source_id_; // id

    unsigned int current_action_id_;

    std::map<unsigned int, DBOEditDataSourceAction> possible_actions_; // action_id -> action

    std::unique_ptr<DBOEditDataSourceActionOptionsWidget> widget_;
};

using DBOEditDataSourceActionOptionsCollection = typename std::map<unsigned int, DBOEditDataSourceActionOptions>;

namespace DBOEditDataSourceActionOptionsCreator
{
    DBOEditDataSourceActionOptions getSyncOptionsFromDB (DBObject& object, const DBODataSource& source);
    DBOEditDataSourceActionOptions getSyncOptionsFromCfg (DBObject& object, const StoredDBODataSource& source);
}


#endif // DBOEDITDATASOURCEACTIONOPTIONS_H
