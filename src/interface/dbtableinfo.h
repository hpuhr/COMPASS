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

/*
 * DBTableInfo.h
 *
 *  Created on: Jan 8, 2016
 *      Author: sk
 */

#ifndef DBTABLEINFO_H_
#define DBTABLEINFO_H_

#include <map>

class DBTableColumnInfo
{
public:
    DBTableColumnInfo (const std::string &name, const std::string &type, bool null_allowed, bool key, const std::string &comment)
        : name_(name), type_(type), null_allowed_(null_allowed), key_(key), comment_(comment)  {}
    virtual ~DBTableColumnInfo() {}

    std::string name() const { return name_; }

    std::string type() const { return type_; }

    bool nullAllowed() const { return null_allowed_; }

    bool key() const { return key_; }

    std::string comment() const { return comment_; }

protected:
    std::string name_;
    std::string type_;
    bool null_allowed_;
    bool key_;
    std::string comment_;
};

class DBTableInfo
{
public:
    DBTableInfo (const std::string name) : name_(name) {}
    virtual ~DBTableInfo() {}

    bool hasColumn (const std::string &name) const { return columns_.count(name) > 0; }
    const DBTableColumnInfo &column (const std::string &name) const { return columns_.at(name); }
    void addColumn (const std::string &name, const std::string &type, bool null_allowed, bool key, const std::string &comment)
        {  columns_.insert(std::pair<std::string, DBTableColumnInfo> (name,DBTableColumnInfo (name, type, null_allowed, key, comment)));  }

    unsigned int size() { return columns_.size(); }

    const std::map <std::string, DBTableColumnInfo> &columns () const { return columns_; }
protected:
    std::string name_;

    std::map <std::string, DBTableColumnInfo> columns_;
};

#endif

