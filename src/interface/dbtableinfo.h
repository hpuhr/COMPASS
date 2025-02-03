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

#include "property.h"
#include "propertylist.h"

#include <map>
#include <string>

#include <boost/optional.hpp>

/**
 */
class DBTableColumnInfo
{
public:
    DBTableColumnInfo(const std::string& name, 
                      PropertyDataType type,
                      bool key,
                      bool null_allowed = true,
                      const std::string& comment = "")
    :   name_        (name)
    ,   type_        (type)
    ,   null_allowed_(null_allowed)
    ,   key_         (key)
    ,   comment_     (comment)
    {
    }

    DBTableColumnInfo(const std::string& name, 
                      const std::string& type_db,
                      bool key,
                      bool null_allowed = true,
                      const std::string& comment = "")
    :   name_        (name)
    ,   type_db_     (type_db)
    ,   null_allowed_(null_allowed)
    ,   key_         (key)
    ,   comment_     (comment)
    {
    }

    virtual ~DBTableColumnInfo() {}

    std::string name() const { return name_; }
    const boost::optional<PropertyDataType>& type() const { return type_; }
    std::string dbTypeString(bool precise_types) const 
    {
        return type_db_.has_value() ? type_db_.value() : Property::asDBString(type_.value(), precise_types);
    }
    bool nullAllowed() const { return null_allowed_; }
    bool key() const { return key_; }
    std::string comment() const { return comment_; }

protected:
    std::string                       name_;
    boost::optional<PropertyDataType> type_;
    boost::optional<std::string>      type_db_;
    bool                              null_allowed_;
    bool                              key_;
    std::string                       comment_;
};

/**
 */
class DBTableInfo
{
public:
    DBTableInfo(const std::string name) : name_(name) {}
    virtual ~DBTableInfo() {}

    bool hasColumn(const std::string& name) const { return columns_.count(name) > 0; }
    const DBTableColumnInfo& column(const std::string& name) const { return columns_.at(name); }
    void addColumn(const std::string& name, const std::string& type, bool key, bool null_allowed,
                   const std::string& comment)
    {
        columns_.insert(std::pair<std::string, DBTableColumnInfo>(
            name, DBTableColumnInfo(name, type, key, null_allowed, comment)));
    }

    unsigned int size() { return columns_.size(); }

    const std::map<std::string, DBTableColumnInfo>& columns() const { return columns_; }

    PropertyList tableProperties() const
    {
        PropertyList p;

        for (const auto& c : columns_)
            p.addProperty(c.first, c.second.type().value());
        
        return p;
    }

protected:
    std::string name_;

    std::map<std::string, DBTableColumnInfo> columns_;
};
