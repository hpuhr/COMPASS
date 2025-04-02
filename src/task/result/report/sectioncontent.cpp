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

#include "task/result/report/sectioncontent.h"
#include "task/result/report/section.h"

#include <cassert>

namespace ResultReport
{

const std::string  SectionContent::DBTableName         = "report_viewables";
const Property     SectionContent::DBColumnContentID   = Property("content_id"  , PropertyDataType::UINT);
const Property     SectionContent::DBColumnResultID    = Property("result_id"   , PropertyDataType::UINT);
const Property     SectionContent::DBColumnType        = Property("type"        , PropertyDataType::INT );
const Property     SectionContent::DBColumnJSONContent = Property("json_content", PropertyDataType::JSON);
const PropertyList SectionContent::DBPropertyList      = PropertyList({ SectionContent::DBColumnContentID,
                                                                        SectionContent::DBColumnResultID,
                                                                        SectionContent::DBColumnType,
                                                                        SectionContent::DBColumnJSONContent });
const std::string SectionContent::FieldType = "type";
const std::string SectionContent::FieldID   = "id";
const std::string SectionContent::FieldName = "name";

/**
 */
SectionContent::SectionContent(Type type,
                               unsigned int id,
                               const std::string& name, 
                               Section* parent_section)
:   type_          (type          )
,   id_            (id            )
,   name_          (name          )
,   parent_section_(parent_section)
{
    assert (parent_section_);

    report_ = parent_section_->report();
}

/**
 */
SectionContent::SectionContent(Type type,
                               Section* parent_section)
:   type_          (type          )
,   parent_section_(parent_section)
{
    assert (parent_section_);

    report_ = parent_section_->report();
}

/**
 */
std::string SectionContent::typeAsString(Type type)
{
    switch(type)
    {
        case Type::Figure:
            return "figure";
        case Type::Table:
            return "table";
        case Type::Text:
            return "text";
        default:
            return "";
    }
    return "";
}

/**
 */
boost::optional<SectionContent::Type> SectionContent::typeFromString(const std::string& type_str)
{
    if (type_str == "figure")
        return Type::Figure;
    else if (type_str == "table")
        return Type::Table;
    else if (type_str == "text")
        return Type::Text;

    return boost::optional<SectionContent::Type>();
}

/**
 */
SectionContent::Type SectionContent::type() const
{
    return type_;
}

/**
 */
std::string SectionContent::typeAsString() const
{
    return SectionContent::typeAsString(type_);
}

/**
 */
unsigned int SectionContent::id() const
{
    return id_;
}

/**
 */
std::string SectionContent::name() const
{
    return name_;
}

/**
 */
nlohmann::json SectionContent::toJSON() const
{
    nlohmann::json root;

    root[ FieldType ] = typeAsString(type_);
    root[ FieldID   ] = id_;
    root[ FieldName ] = name_;

    toJSON_impl(root);

    return root;
}

/**
 */
bool SectionContent::fromJSON(const nlohmann::json& j)
{
    if (!j.is_object() ||
        !j.contains(FieldType) ||
        !j.contains(FieldID)   ||
        !j.contains(FieldName))
    {
        logerr << "SectionContent: fromJSON: Error: Section content does not obtain needed fields";
        return false;
    }

    try
    {
        std::string t_str = j[ FieldType ];
        auto t = typeFromString(t_str);
        if (!t.has_value())
        {
            logerr << "SectionContent: fromJSON: Error: Could not deduce section content type";
            return false;
        }

        type_ = t.value();
        id_   = j[ FieldID   ];
        name_ = j[ FieldName ];

        if (!fromJSON_impl(j))
            return false;
    }
    catch(const std::exception& ex)
    {
        logerr << "SectionContent: fromJSON: Error: " << ex.what();
        return false;
    }
    catch(...)
    {
        logerr << "SectionContent: fromJSON: Unknown error";
        return false;
    }

    return true;
}

}
