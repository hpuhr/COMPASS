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

#ifndef ACTIVEDATASOURCE_H
#define ACTIVEDATASOURCE_H

#include "json.hpp"

#include <string>

/**
 * @brief Definition for a data source in a DataSourcesFilter
 */
class ActiveDataSource
{
  public:
    /// @brief Constructor
    ActiveDataSource(unsigned int number, const std::string& name,
                                nlohmann::json::boolean_t& active_flag)
        : number_(number), name_(name), active_flag_(active_flag)
    {
    }
    /// @brief Destructor
    virtual ~ActiveDataSource() {}

  protected:
    /// Number id
    unsigned int number_ {0};
    /// Name id
    std::string name_;
    /// Flag indicating if active in data
    bool active_in_data_ {false};
    /// Flag indicating if active (to be used)
    nlohmann::json::boolean_t& active_flag_;

  public:
    bool isActiveInData() const { return active_in_data_; }

    void setActiveInData(bool active_in_data) { active_in_data_ = active_in_data; }

    bool isActive() const
    {
        return active_flag_;
    }

    void setActive(bool active_flag)
    {
        active_flag_ = active_flag;
    }

    std::string getName() const { return name_; }

    unsigned int getNumber() const { return number_; }
};

#endif // ACTIVEDATASOURCE_H
