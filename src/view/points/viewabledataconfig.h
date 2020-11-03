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

#ifndef VIEWABLEDATACONFIG_H
#define VIEWABLEDATACONFIG_H

#include "json.hpp"

class ViewableDataConfig
{
public:
  ViewableDataConfig(const nlohmann::json::object_t& data)
  {
      data_ = data;
  }

  ViewableDataConfig(const std::string& json_str)
  {
      data_ = nlohmann::json::parse(json_str);
  }

  const nlohmann::json& data() const { return data_; }

protected:
    nlohmann::json data_;
};

#endif // VIEWABLEDATACONFIG_H
