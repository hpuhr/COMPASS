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

#include "dbovariable.h"

#include <algorithm>

#include "compass.h"
#include "configuration.h"
#include "configurationmanager.h"
#include "dbinterface.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbovariablewidget.h"
#include "stringconv.h"
#include "unit.h"
#include "unitmanager.h"

using namespace Utils;

std::map<DBOVariable::Representation, std::string> DBOVariable::representation_2_string_{
    {DBOVariable::Representation::STANDARD, "STANDARD"},
    {DBOVariable::Representation::SECONDS_TO_TIME, "SECONDS_TO_TIME"},
    {DBOVariable::Representation::DEC_TO_OCTAL, "DEC_TO_OCTAL"},
    {DBOVariable::Representation::DEC_TO_HEX, "DEC_TO_HEX"},
    {DBOVariable::Representation::FEET_TO_FLIGHTLEVEL, "FEET_TO_FLIGHTLEVEL"},
    {DBOVariable::Representation::DATA_SRC_NAME, "DATA_SRC_NAME"}};

std::map<std::string, DBOVariable::Representation> DBOVariable::string_2_representation_{
    {"STANDARD", DBOVariable::Representation::STANDARD},
    {"SECONDS_TO_TIME", DBOVariable::Representation::SECONDS_TO_TIME},
    {"DEC_TO_OCTAL", DBOVariable::Representation::DEC_TO_OCTAL},
    {"DEC_TO_HEX", DBOVariable::Representation::DEC_TO_HEX},
    {"FEET_TO_FLIGHTLEVEL", DBOVariable::Representation::FEET_TO_FLIGHTLEVEL},
    {"DATA_SRC_NAME", DBOVariable::Representation::DATA_SRC_NAME}};

DBOVariable::Representation DBOVariable::stringToRepresentation(
    const std::string& representation_str)
{
    assert(string_2_representation_.count(representation_str) == 1);
    return string_2_representation_.at(representation_str);
}

std::string DBOVariable::representationToString(Representation representation)
{
    assert(representation_2_string_.count(representation) == 1);
    return representation_2_string_.at(representation);
}

//#include <boost/algorithm/string.hpp>

DBOVariable::DBOVariable(const std::string& class_id, const std::string& instance_id,
                         DBObject* parent)
    : Property(), Configurable(class_id, instance_id, parent), db_object_(parent)
{
    registerParameter("name", &name_, "");
    registerParameter("description", &description_, "");
    registerParameter("db_column_name", &db_column_name_, "");
    registerParameter("data_type_str", &data_type_str_, "");
    registerParameter("is_key", &is_key_, false);
    registerParameter("representation_str", &representation_str_, "");
    registerParameter("dimension", &dimension_, "");
    registerParameter("unit", &unit_, "");

    if (name_.size() == 0)
        logerr << "DBOVariable: constructor: instance " << instance_id << " has no name";


    assert(name_.size() > 0);

    if (data_type_str_.size() == 0)
        logerr << "DBOVariable: constructor: name " << name_ << " has no data type";

    assert(data_type_str_.size() > 0);
    data_type_ = Property::asDataType(data_type_str_);

    if (representation_str_.size() == 0)
    {
        representation_str_ = representationToString(Representation::STANDARD);
    }

    representation_ = stringToRepresentation(representation_str_);

    // loginf  << "DBOVariable: constructor: name " << id_ << " unitdim '" << unit_dimension_ << "'
    // unitunit '" << unit_unit_ << "'";

    assert (db_column_name_.size());
    //boost::algorithm::to_lower(db_column_name_); // modifies str

    createSubConfigurables();
}

DBOVariable& DBOVariable::operator=(DBOVariable&& other)
//: Configurable(std::move(other))
{
    loginf << "DBOVariable: move operator: moving";

    data_type_ = other.data_type_;
    data_type_str_ = other.data_type_str_;

    name_ = other.name_;
    other.name_ = "";

    db_object_ = other.db_object_;
    other.db_object_ = nullptr;

    representation_str_ = other.representation_str_;
    other.representation_str_ = "";

    representation_ = other.representation_;
    other.representation_ = Representation::STANDARD;

    description_ = other.description_;
    other.description_ = "";

    db_column_name_ = other.db_column_name_;
    other.db_column_name_ = "";

//    min_max_set_ = other.min_max_set_;
//    other.min_max_set_ = false;

//    min_ = other.min_;
//    other.min_ = "";

//    max_ = other.max_;
//    other.max_ = "";

    dimension_ = other.dimension_;
    other.dimension_ = "";

    unit_ = other.unit_;
    other.unit_ = "";

    widget_ = other.widget_;
    if (widget_)
        widget_->setVariable(*this);
    other.widget_ = nullptr;

    other.configuration().updateParameterPointer("name", &name_);
    other.configuration().updateParameterPointer("description", &description_);
    other.configuration().updateParameterPointer("variable_identifier", &db_column_name_);
    other.configuration().updateParameterPointer("data_type_str", &data_type_str_);
    other.configuration().updateParameterPointer("representation_str", &representation_str_);
    other.configuration().updateParameterPointer("dimension", &dimension_);
    other.configuration().updateParameterPointer("unit", &unit_);

    // return *this;
    return static_cast<DBOVariable&>(Configurable::operator=(std::move(other)));
}

DBOVariable::~DBOVariable()
{
    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }
}

void DBOVariable::generateSubConfigurable(const std::string& class_id,
                                          const std::string& instance_id)
{
    //"variable_identifier": "sd_ads.ALT_REPORTING_CAPABILITY_FT"

//    if (class_id == "DBOSchemaVariableDefinition")
//    {
//        std::string var_id = configuration()
//                                   .getSubConfiguration(class_id, instance_id)
//                                   .getParameterConfigValueString("variable_identifier");
//        if (var_id.size())
//            variable_identifier_ = var_id;

//        //configuration().removeSubConfiguration(class_id, instance_id);
//    }
//    else
        throw std::runtime_error("DBOVariable: generateSubConfigurable: unknown class_id " +
                                     class_id);
}

bool DBOVariable::operator==(const DBOVariable& var)
{
    if (dboName() != var.dboName())
        return false;
    if (data_type_ != var.data_type_)
        return false;
    if (name_.compare(var.name_) != 0)
        return false;

    return true;
}

void DBOVariable::print()
{
    loginf << "DBOVariable: print: dbo " << Configurable::parent().instanceId() << " id " << name_
           << " data type " << data_type_str_;
}



void DBOVariable::checkSubConfigurables()
{
}

const std::string& DBOVariable::dboName() const
{
    assert(db_object_);
    return db_object_->name();
}

void DBOVariable::name(const std::string& name)
{
    loginf << "DBOVariable: name: old " << name_ << " new " << name;
    name_ = name;
}

//const std::string& DBOVariable::metaTable() const
//{
//    assert(db_object_);
//    return db_object_->currentMetaTable();
//}

std::string DBOVariable::dbColumnName() const
{
    return db_column_name_;
}

void DBOVariable::dbColumnName(const std::string& value)
{
    db_column_name_ = value;
}

std::string DBOVariable::dbTableName() const
{
    assert (db_object_);
    return db_object_->dbTableName();
}

std::string DBOVariable::dbColumnIdentifier() const
{
    return dbTableName()+":"+dbColumnName();
}

//void DBOVariable::setMinMax()
//{
//    assert(!min_max_set_);

//    assert(db_object_);
//    logdbg << "DBOVariable " << db_object_->name() << " " << name_ << ": setMinMax";

//    if (!dbObject().existsInDB() || !dbObject().count())
//    {
//        min_ = NULL_STRING;
//        max_ = NULL_STRING;
//    }
//    else
//    {
//        std::pair<std::string, std::string> min_max =
//            COMPASS::instance().interface().getMinMaxString(*this);

//        min_ = min_max.first;
//        max_ = min_max.second;
//    }

//    min_max_set_ = true;

//    logdbg << "DBOVariable: setMinMax: min " << min_ << " max " << max_;
//}

//std::string DBOVariable::getMinString()
//{
//    if (!min_max_set_)
//        setMinMax();  // already unit transformed

//    assert(min_max_set_);

//    logdbg << "DBOVariable: getMinString: object " << dboName() << " name " << name()
//           << " returning " << min_;
//    return min_;
//}

//std::string DBOVariable::getMaxString()
//{
//    if (!min_max_set_)
//        setMinMax();  // is already unit transformed

//    assert(min_max_set_);

//    logdbg << "DBOVariable: getMaxString: object " << dboName() << " name " << name()
//           << " returning " << max_;
//    return max_;
//}

//std::string DBOVariable::getMinStringRepresentation()
//{
//    if (representation_ == Representation::STANDARD)
//        return getMinString();
//    else
//        return getRepresentationStringFromValue(getMinString());
//}

//std::string DBOVariable::getMaxStringRepresentation()
//{
//    if (representation_ == Representation::STANDARD)
//        return getMaxString();
//    else
//        return getRepresentationStringFromValue(getMaxString());
//}

DBOVariableWidget* DBOVariable::widget()
{
    if (!widget_)
    {
        widget_ = new DBOVariableWidget(*this);
        assert(widget_);
    }

    return widget_;
}

DBOVariable::Representation DBOVariable::representation() const { return representation_; }

const std::string& DBOVariable::representationString() const { return representation_str_; }

void DBOVariable::representation(const DBOVariable::Representation& representation)
{
    representation_str_ = representationToString(representation);
    representation_ = representation;
}

std::string DBOVariable::getRepresentationStringFromValue(const std::string& value_str) const
{
    logdbg << "DBOVariable: getRepresentationStringFromValue: value " << value_str << " data_type "
           << Property::asString(data_type_) << " representation "
           << representationToString(representation_);

    if (value_str == NULL_STRING)
        return value_str;

    if (representation_ == DBOVariable::Representation::STANDARD)
        return value_str;

    switch (data_type_)
    {
        case PropertyDataType::BOOL:
        {
            bool value = std::stoul(value_str);
            return getAsSpecialRepresentationString(value);
        }
        case PropertyDataType::CHAR:
        {
            char value = std::stoi(value_str);
            return getAsSpecialRepresentationString(value);
        }
        case PropertyDataType::UCHAR:
        {
            unsigned char value = std::stoul(value_str);
            return getAsSpecialRepresentationString(value);
        }
        case PropertyDataType::INT:
        {
            int value = std::stoi(value_str);
            return getAsSpecialRepresentationString(value);
        }
        case PropertyDataType::UINT:
        {
            unsigned int value = std::stoul(value_str);
            return getAsSpecialRepresentationString(value);
        }
        case PropertyDataType::LONGINT:
        {
            long value = std::stol(value_str);
            return getAsSpecialRepresentationString(value);
        }
        case PropertyDataType::ULONGINT:
        {
            unsigned long value = std::stoul(value_str);
            return getAsSpecialRepresentationString(value);
        }
        case PropertyDataType::FLOAT:
        {
            float value = std::stof(value_str);
            return getAsSpecialRepresentationString(value);
        }
        case PropertyDataType::DOUBLE:
        {
            double value = std::stod(value_str);
            return getAsSpecialRepresentationString(value);
        }
        case PropertyDataType::STRING:
            throw std::invalid_argument(
                "DBOVariable: getRepresentationStringFromValue: representation of string variable"
                " impossible");
        default:
            logerr << "DBOVariable: getRepresentationStringFromValue:: unknown property type "
                   << Property::asString(data_type_);
            throw std::runtime_error(
                "DBOVariable: getRepresentationStringFromValue:: unknown property type " +
                Property::asString(data_type_));
    }
}

std::string DBOVariable::getValueStringFromRepresentation(
    const std::string& representation_str) const
{
    if (representation_str == NULL_STRING)
        return representation_str;

    assert(representation_ != DBOVariable::Representation::STANDARD);

    if (representation_ == DBOVariable::Representation::SECONDS_TO_TIME)
    {
        return String::getValueString(Utils::String::timeFromString(representation_str));
    }
    else if (representation_ == DBOVariable::Representation::DEC_TO_OCTAL)
    {
        return String::getValueString(Utils::String::intFromOctalString(representation_str));
    }
    else if (representation_ == DBOVariable::Representation::DEC_TO_HEX)
    {
        return String::getValueString(Utils::String::intFromHexString(representation_str));
    }
    else if (representation_ == DBOVariable::Representation::FEET_TO_FLIGHTLEVEL)
    {
        return String::getValueString(std::stod(representation_str) * 100.0);
    }
    else if (representation_ == DBOVariable::Representation::DATA_SRC_NAME)
    {
        assert(db_object_);

        TODO_ASSERT

//        if (db_object_->hasDataSources())
//        {
//            for (auto ds_it = db_object_->dsBegin(); ds_it != db_object_->dsEnd(); ++ds_it)
//            {
//                if ((ds_it->second.hasShortName() &&
//                     representation_str == ds_it->second.shortName()) ||
//                    representation_str == ds_it->second.name())
//                {
//                    return std::to_string(ds_it->first);
//                }
//            }
//            // not found, return original
//        }
        // has no datasources, return original

        return representation_str;
    }
    else
    {
        throw std::runtime_error(
            "Utils: String: getValueStringFromRepresentation: unknown representation " +
            std::to_string((int)representation_));
    }
}

std::string DBOVariable::multiplyString(const std::string& value_str, double factor) const
{
    logdbg << "DBOVariable: multiplyString: value " << value_str << " factor " << factor
           << " data_type " << Property::asString(data_type_);

    if (value_str == NULL_STRING)
        return value_str;

    std::string return_string;

    switch (data_type_)
    {
        case PropertyDataType::BOOL:
        {
            bool value = std::stoul(value_str);
            value = value && factor;
            return_string = std::to_string(value);
            break;
        }
        case PropertyDataType::CHAR:
        {
            char value = std::stoi(value_str);
            value *= factor;
            return_string = std::to_string(value);
            break;
        }
        case PropertyDataType::UCHAR:
        {
            unsigned char value = std::stoul(value_str);
            value *= factor;
            return_string = std::to_string(value);
            break;
        }
        case PropertyDataType::INT:
        {
            int value = std::stoi(value_str);
            value *= factor;
            return_string = std::to_string(value);
            break;
        }
        case PropertyDataType::UINT:
        {
            unsigned int value = std::stoul(value_str);
            value *= factor;
            return_string = std::to_string(value);
            break;
        }
        case PropertyDataType::LONGINT:
        {
            long value = std::stol(value_str);
            value *= factor;
            return_string = std::to_string(value);
            break;
        }
        case PropertyDataType::ULONGINT:
        {
            unsigned long value = std::stoul(value_str);
            value *= factor;
            return_string = std::to_string(value);
            break;
        }
        case PropertyDataType::FLOAT:
        {
            float value = std::stof(value_str);
            value *= factor;
            return_string = String::getValueString(value);
            break;
        }
        case PropertyDataType::DOUBLE:
        {
            double value = std::stod(value_str);
            value *= factor;
            return_string = String::getValueString(value);
            break;
        }
        case PropertyDataType::STRING:
            throw std::invalid_argument(
                "DBOVariable: multiplyString: multiplication of string variable impossible");
        default:
            logerr << "DBOVariable: multiplyString:: unknown property type "
                   << Property::asString(data_type_);
            throw std::runtime_error("DBOVariable: multiplyString:: unknown property type " +
                                     Property::asString(data_type_));
    }

    logdbg << "DBOVariable: multiplyString: return value " << return_string;

    return return_string;
}

const std::string& DBOVariable::getLargerValueString(const std::string& value_a_str,
                                                     const std::string& value_b_str) const
{
    logdbg << "DBOVariable: getLargerValueString: value a " << value_a_str << " b " << value_b_str
           << " data_type " << Property::asString(data_type_);

    if (value_a_str == NULL_STRING || value_b_str == NULL_STRING)
    {
        if (value_a_str != NULL_STRING)
            return value_a_str;
        if (value_b_str != NULL_STRING)
            return value_b_str;
        return NULL_STRING;
    }

    switch (data_type_)
    {
        case PropertyDataType::BOOL:
        case PropertyDataType::UCHAR:
        case PropertyDataType::UINT:
        case PropertyDataType::ULONGINT:
        {
            if (std::stoul(value_a_str) > std::stoul(value_b_str))
                return value_a_str;
            else
                return value_b_str;
        }
        case PropertyDataType::CHAR:
        case PropertyDataType::INT:
        {
            if (std::stoi(value_a_str) > std::stoi(value_b_str))
                return value_a_str;
            else
                return value_b_str;
        }
        case PropertyDataType::LONGINT:
        {
            if (std::stol(value_a_str) > std::stol(value_b_str))
                return value_a_str;
            else
                return value_b_str;
        }
        case PropertyDataType::FLOAT:
        {
            if (std::stof(value_a_str) > std::stof(value_b_str))
                return value_a_str;
            else
                return value_b_str;
        }
        case PropertyDataType::DOUBLE:
        {
            if (std::stod(value_a_str) > std::stod(value_b_str))
                return value_a_str;
            else
                return value_b_str;
        }
        case PropertyDataType::STRING:
            throw std::invalid_argument(
                "DBOVariable: getLargerValueString: operation on string variable impossible");
        default:
            logerr << "DBOVariable: getLargerValueString:: unknown property type "
                   << Property::asString(data_type_);
            throw std::runtime_error("DBOVariable: getLargerValueString:: unknown property type " +
                                     Property::asString(data_type_));
    }
}

const std::string& DBOVariable::getSmallerValueString(const std::string& value_a_str,
                                                      const std::string& value_b_str) const
{
    logdbg << "DBOVariable: getSmallerValueString: value a " << value_a_str << " b " << value_b_str
           << " data_type " << Property::asString(data_type_);

    if (value_a_str == NULL_STRING || value_b_str == NULL_STRING)
    {
        if (value_a_str != NULL_STRING)
            return value_a_str;
        if (value_b_str != NULL_STRING)
            return value_b_str;
        return NULL_STRING;
    }

    switch (data_type_)
    {
        case PropertyDataType::BOOL:
        case PropertyDataType::UCHAR:
        case PropertyDataType::UINT:
        case PropertyDataType::ULONGINT:
        {
            if (std::stoul(value_a_str) < std::stoul(value_b_str))
                return value_a_str;
            else
                return value_b_str;
        }
        case PropertyDataType::CHAR:
        case PropertyDataType::INT:
        {
            if (std::stoi(value_a_str) < std::stoi(value_b_str))
                return value_a_str;
            else
                return value_b_str;
        }
        case PropertyDataType::LONGINT:
        {
            if (std::stol(value_a_str) < std::stol(value_b_str))
                return value_a_str;
            else
                return value_b_str;
        }
        case PropertyDataType::FLOAT:
        {
            if (std::stof(value_a_str) < std::stof(value_b_str))
                return value_a_str;
            else
                return value_b_str;
        }
        case PropertyDataType::DOUBLE:
        {
            if (std::stod(value_a_str) < std::stod(value_b_str))
                return value_a_str;
            else
                return value_b_str;
        }
        case PropertyDataType::STRING:
            throw std::invalid_argument(
                "DBOVariable: getSmallerValueString: operation on string variable impossible");
        default:
            logerr << "DBOVariable: getSmallerValueString:: unknown property type "
                   << Property::asString(data_type_);
            throw std::runtime_error("DBOVariable: getSmallerValueString:: unknown property type " +
                                     Property::asString(data_type_));
    }
}

bool DBOVariable::hasShortName() const
{
    return short_name_.size();
}

std::string DBOVariable::shortName() const
{
    return short_name_;
}

void DBOVariable::shortName(const std::string& short_name)
{
    short_name_ = short_name;
}

bool DBOVariable::isKey() const
{
    return is_key_;
}

void DBOVariable::isKey(bool value)
{
    is_key_ = value;
}

std::string DBOVariable::getDataSourcesAsString(const std::string& value) const
{
    assert(db_object_);

    TODO_ASSERT


//    if (db_object_->hasDataSources())
//    {
//        for (auto ds_it = db_object_->dsBegin(); ds_it != db_object_->dsEnd(); ++ds_it)
//        {
//            if (std::to_string(ds_it->first) == value)
//            {
//                if (ds_it->second.hasShortName())
//                    return ds_it->second.shortName();
//                else
//                    return ds_it->second.name();
//            }
//        }
//        // not found, return original
//    }

//    // search for data sources in other dbos
//    for (auto& dbo_it : COMPASS::instance().objectManager())
//    {
//        if (dbo_it.second->hasDataSources())
//        {
//            for (auto ds_it = dbo_it.second->dsBegin(); ds_it != dbo_it.second->dsEnd(); ++ds_it)
//            {
//                if (std::to_string(ds_it->first) == value)
//                {
//                    if (ds_it->second.hasShortName())
//                        return ds_it->second.shortName();
//                    else
//                        return ds_it->second.name();
//                }
//            }
//            // not found, return original
//        }
//    }

    // has no datasources, return original
    //loginf << "DBOVariable: getDataSourcesAsString: ds '" << value << "' not found";

    return value;
}
