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

#ifndef DBOSPECIFICVALUESDBFILTER_H
#define DBOSPECIFICVALUESDBFILTER_H

#include "dbfilter.h"

class DBContent;

namespace dbContent
{
class DBContentVariable;
}

class DBOSpecificValuesDBFilter : public DBFilter
{
public:
    DBOSpecificValuesDBFilter(const std::string& class_id, const std::string& instance_id,
                              Configurable* parent);
    virtual ~DBOSpecificValuesDBFilter() override;

    virtual std::string getConditionString(const std::string& dbo_name, bool& first,
                                           std::vector<dbContent::DBContentVariable*>& filtered_variables) override;

    virtual bool filters(const std::string& dbo_name) override;

    const std::string& dbObjectName() { return dbo_name_; }

protected:
  std::string dbo_name_;
  std::string variable_name_;
  std::string condition_operator_; // operator to be used in generated conditions

  DBContent* object_{nullptr};
  dbContent::DBContentVariable* variable_ {nullptr};

  std::string ds_column_name_;

  virtual void checkSubConfigurables() override;
};

#endif // DBOSPECIFICVALUESDBFILTER_H
