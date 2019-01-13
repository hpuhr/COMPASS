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

#ifndef JSONUTILS_H
#define JSONUTILS_H

#include "json.hpp"

namespace Utils
{

namespace JSON
{

inline std::string toString(const nlohmann::json& j)
{
    if (j.type() == nlohmann::json::value_t::string) {
        return j.get<std::string>();
    }

    return j.dump();
}

}
}

#endif // JSONUTILS_H
