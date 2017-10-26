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

#ifndef SQLITE_FILE_H
#define SQLITE_FILE_H

#include "configurable.h"

class SQLiteFile : public Configurable
{
public:
    SQLiteFile(const std::string class_id, const std::string &instance_id, Configurable *parent)
        : Configurable (class_id, instance_id, parent)
    {
        registerParameter ("name", &name_, "");
    }
    virtual ~SQLiteFile () {}

    /// Returns the database server name or IP address
    const std::string &name () const { return name_; }
    void name (const std::string &name) { name_ = name; }

protected:
    std::string name_;
};

#endif // SQLITE_FILE_H
