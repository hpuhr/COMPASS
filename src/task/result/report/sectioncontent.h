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

#include "property.h"

#include <boost/optional.hpp>

#include "json.hpp"

class TaskManager;

class QVBoxLayout;
class LatexVisitor;

namespace ResultReport
{

class Section;

/**
 */
class SectionContent
{
public:
    enum class Type
    {
        Figure = 0,
        Table,
        Text
    };

    SectionContent(Type type,
                   unsigned int id,
                   const std::string& name, 
                   Section* parent_section, 
                   TaskManager& task_man);
    SectionContent(Type type,
                   Section* parent_section, 
                   TaskManager& task_man);

    Type type() const;
    std::string typeAsString() const;
    unsigned int id() const;
    std::string name() const;

    nlohmann::json toJSON() const;
    bool fromJSON(const nlohmann::json& j);
    
    virtual void addToLayout (QVBoxLayout* layout) = 0; // add content to layout
    virtual void accept(LatexVisitor& v) = 0;           // can not be const since on-demand tables

    static std::string typeAsString(Type type);
    static boost::optional<Type> typeFromString(const std::string& type_str);

    static const std::string DBTableName;
    static const Property    DBColumnContentID;
    static const Property    DBColumnResultID;
    static const Property    DBColumnType;
    static const Property    DBColumnJSONContent;

    static const std::string FieldType;
    static const std::string FieldID;
    static const std::string FieldName;

protected:
    virtual void toJSON_impl(nlohmann::json& root_node) const = 0;
    virtual bool fromJSON_impl(const nlohmann::json& j) = 0;

    Type         type_;
    unsigned int id_ = 0;
    std::string  name_;
    Section*     parent_section_ {nullptr};
    TaskManager& task_man_;
};

}
