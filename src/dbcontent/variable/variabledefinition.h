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

#pragma once

#include <string>

#include "configurable.h"

namespace dbContent
{

class VariableDefinition : public Configurable
{
  public:
    VariableDefinition(const std::string& class_id, const std::string& instance_id,
                          Configurable* parent)
        : Configurable(class_id, instance_id, parent)
    {
        registerParameter("dbcontent_name", &dbcontent_name_, std::string());
        registerParameter("variable_name", &variable_name_, std::string());

        // DBContVAR LOWERCASE HACK
        // boost::algorithm::to_lower(dbcont_variable_name_);

        assert(variable_name_.size() > 0);
    }

    VariableDefinition& operator=(VariableDefinition&& other)
    {
        dbcontent_name_ = other.dbcontent_name_;
        other.dbcontent_name_ = "";

        variable_name_ = other.variable_name_;
        other.variable_name_ = "";

        return *this;
    }

    virtual ~VariableDefinition() {}

    const std::string& dbContentName() { return dbcontent_name_; }
    void dbContentName(const std::string& dbcontent_name) { dbcontent_name_ = dbcontent_name; }

    const std::string& variableName() { return variable_name_; }
    void variableName(const std::string& dbcont_variable_name)
    {
        variable_name_ = dbcont_variable_name;
    }

  protected:
    std::string dbcontent_name_;
    std::string variable_name_;
};

}
