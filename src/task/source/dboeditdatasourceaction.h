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

#ifndef DBOEDITDATASOURCEACTION_H
#define DBOEDITDATASOURCEACTION_H

#include <string>

class DBObject;

class DBOEditDataSourceAction
{
  public:
    DBOEditDataSourceAction(const std::string& action = "None", const std::string& target_type = "",
                            const std::string& target_id = "");

    const std::string& getActionString() { return action_str_; }

    void perform(DBObject& object, const std::string& source_type, const std::string& source_id);

    std::string targetType() const;

  private:
    std::string action_;       // "None", "Add", "Overwrite"
    std::string target_type_;  // "Config", "DB"
    std::string target_id_;    // "New" ("Add"),id ("Overwrite")

    std::string action_str_;

    void refreshActionString();
};

#endif  // DBOEDITDATASOURCEACTION_H
