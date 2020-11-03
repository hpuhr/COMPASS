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

#ifndef DBOSCHEMAMETATABLEDEFINITION_H
#define DBOSCHEMAMETATABLEDEFINITION_H

#include "configurable.h"

/**
 * @brief Definition of a meta table in a schema in a DBObject
 *
 * Simple storage class for a schema and a meta table, as strings. Used in a DBObject to save the
 * definitions into the configuration, and generate the pointers to the defined structures at from
 * them.
 */
class DBOSchemaMetaTableDefinition : public Configurable
{
  public:
    /// @brief Constructor, registers parameters
    DBOSchemaMetaTableDefinition(const std::string& class_id, const std::string& instance_id,
                                 Configurable* parent)
        : Configurable(class_id, instance_id, parent)
    {
        registerParameter("schema", &schema_, "");
        registerParameter("meta_table", &meta_table_, "");
    }
    /// @brief Destructor
    virtual ~DBOSchemaMetaTableDefinition() {}

    const std::string& schema() const { return schema_; }
    const std::string& metaTable() const { return meta_table_; }

  protected:
    /// DBSchema identifier
    std::string schema_;
    /// MetaDBTable identifier
    std::string meta_table_;
};

#endif  // DBOSCHEMAMETATABLEDEFINITION_H
