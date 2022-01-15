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

#ifndef UTNFILTER_H
#define UTNFILTER_H

#include "dbfilter.h"

class UTNFilter : public DBFilter
{
public:
    UTNFilter(const std::string& class_id, const std::string& instance_id,
              Configurable* parent);
    virtual ~UTNFilter();

    virtual std::string getConditionString(const std::string& dbo_name, bool& first,
                                           std::vector<DBContentVariable*>& filtered_variables);

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    virtual bool filters(const std::string& dbo_name);
    virtual void reset();

    virtual void saveViewPointConditions (nlohmann::json& filters);
    virtual void loadViewPointConditions (const nlohmann::json& filters);

    std::string utns() const;
    void utns(const std::string& utns);

protected:
    std::string utns_str_;
    std::vector<unsigned int> utns_;

    virtual void checkSubConfigurables();

    bool updateUTNSFromStr(const std::string& utns); // returns success
};

#endif // UTNFILTER_H
