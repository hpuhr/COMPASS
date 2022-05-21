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

#include "dbcontent/variable/variable.h"

#include <algorithm>

#include "compass.h"
#include "configuration.h"
#include "configurationmanager.h"
#include "dbinterface.h"
#include "dbcontent/dbcontent.h"
#include "datasourcemanager.h"
#include "dbcontent/variable/variablewidget.h"
#include "stringconv.h"
#include "unit.h"
#include "unitmanager.h"

using namespace Utils;

namespace dbContent
{

std::map<Variable::Representation, std::string> Variable::representation_2_string_{
    {Variable::Representation::STANDARD, "STANDARD"},
    {Variable::Representation::SECONDS_TO_TIME, "SECONDS_TO_TIME"},
    {Variable::Representation::DEC_TO_OCTAL, "DEC_TO_OCTAL"},
    {Variable::Representation::DEC_TO_HEX, "DEC_TO_HEX"},
    {Variable::Representation::FEET_TO_FLIGHTLEVEL, "FEET_TO_FLIGHTLEVEL"},
    {Variable::Representation::DATA_SRC_NAME, "DATA_SRC_NAME"},
    {Variable::Representation::CLIMB_DESCENT, "CLIMB_DESCENT"},
    {Variable::Representation::FLOAT_PREC0, "FLOAT_PREC0"},
    {Variable::Representation::FLOAT_PREC1, "FLOAT_PREC1"},
    {Variable::Representation::FLOAT_PREC2, "FLOAT_PREC2"},
    {Variable::Representation::FLOAT_PREC4, "FLOAT_PREC4"}};

std::map<std::string, Variable::Representation> Variable::string_2_representation_{
    {"STANDARD", Variable::Representation::STANDARD},
    {"SECONDS_TO_TIME", Variable::Representation::SECONDS_TO_TIME},
    {"DEC_TO_OCTAL", Variable::Representation::DEC_TO_OCTAL},
    {"DEC_TO_HEX", Variable::Representation::DEC_TO_HEX},
    {"FEET_TO_FLIGHTLEVEL", Variable::Representation::FEET_TO_FLIGHTLEVEL},
    {"DATA_SRC_NAME", Variable::Representation::DATA_SRC_NAME},
    {"CLIMB_DESCENT", Variable::Representation::CLIMB_DESCENT},
    {"FLOAT_PREC0", Variable::Representation::FLOAT_PREC0},
    {"FLOAT_PREC1", Variable::Representation::FLOAT_PREC1},
    {"FLOAT_PREC2", Variable::Representation::FLOAT_PREC2},
    {"FLOAT_PREC4", Variable::Representation::FLOAT_PREC4}};

Variable::Representation Variable::stringToRepresentation(
    const std::string& representation_str)
{
    assert(string_2_representation_.count(representation_str) == 1);
    return string_2_representation_.at(representation_str);
}

std::string Variable::representationToString(Representation representation)
{
    assert(representation_2_string_.count(representation) == 1);
    return representation_2_string_.at(representation);
}

//#include <boost/algorithm/string.hpp>

Variable::Variable(const std::string& class_id, const std::string& instance_id,
                         DBContent* parent)
    : Property(), Configurable(class_id, instance_id, parent), dbcontent_(parent)
{
    registerParameter("name", &name_, "");
    registerParameter("short_name", &short_name_, "");
    registerParameter("description", &description_, "");
    registerParameter("db_column_name", &db_column_name_, "");
    registerParameter("data_type_str", &data_type_str_, "");
    registerParameter("is_key", &is_key_, false);
    registerParameter("representation_str", &representation_str_, "");
    registerParameter("dimension", &dimension_, "");
    registerParameter("unit", &unit_, "");

    if (name_.size() == 0)
        logerr << "Variable: constructor: instance " << instance_id << " has no name";


    assert(name_.size() > 0);

    if (data_type_str_.size() == 0)
        logerr << "Variable: constructor: name " << name_ << " has no data type";

    assert(data_type_str_.size() > 0);
    data_type_ = Property::asDataType(data_type_str_);

    if (representation_str_.size() == 0)
    {
        representation_str_ = representationToString(Representation::STANDARD);
    }

    representation_ = stringToRepresentation(representation_str_);

    // loginf  << "Variable: constructor: name " << id_ << " unitdim '" << unit_dimension_ << "'
    // unitunit '" << unit_unit_ << "'";

    assert (db_column_name_.size());
    //boost::algorithm::to_lower(db_column_name_); // modifies str

    createSubConfigurables();
}

//Variable& Variable::operator=(Variable&& other)
////: Configurable(std::move(other))
//{
//    loginf << "Variable: move operator: moving";

//    data_type_ = other.data_type_;
//    data_type_str_ = other.data_type_str_;

//    name_ = other.name_;
//    other.name_ = "";

//    dbcontent_ = other.dbcontent_;
//    other.dbcontent_ = nullptr;

//    representation_str_ = other.representation_str_;
//    other.representation_str_ = "";

//    representation_ = other.representation_;
//    other.representation_ = Representation::STANDARD;

//    description_ = other.description_;
//    other.description_ = "";

//    db_column_name_ = other.db_column_name_;
//    other.db_column_name_ = "";

////    min_max_set_ = other.min_max_set_;
////    other.min_max_set_ = false;

////    min_ = other.min_;
////    other.min_ = "";

////    max_ = other.max_;
////    other.max_ = "";

//    dimension_ = other.dimension_;
//    other.dimension_ = "";

//    unit_ = other.unit_;
//    other.unit_ = "";

//    widget_ = other.widget_;
//    if (widget_)
//        widget_->setVariable(*this);
//    other.widget_ = nullptr;

//    other.configuration().updateParameterPointer("name", &name_);
//    other.configuration().updateParameterPointer("description", &description_);
//    other.configuration().updateParameterPointer("variable_identifier", &db_column_name_);
//    other.configuration().updateParameterPointer("data_type_str", &data_type_str_);
//    other.configuration().updateParameterPointer("representation_str", &representation_str_);
//    other.configuration().updateParameterPointer("dimension", &dimension_);
//    other.configuration().updateParameterPointer("unit", &unit_);

//    // return *this;
//    return static_cast<Variable&>(Configurable::operator=(std::move(other)));
//}

Variable::~Variable()
{
    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }
}

void Variable::generateSubConfigurable(const std::string& class_id,
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
        throw std::runtime_error("Variable: generateSubConfigurable: unknown class_id " +
                                     class_id);
}

bool Variable::operator==(const Variable& var)
{
    if (dbContentName() != var.dbContentName())
        return false;
    if (data_type_ != var.data_type_)
        return false;
    if (name_.compare(var.name_) != 0)
        return false;

    return true;
}

void Variable::print()
{
    loginf << "Variable: print: dbo " << Configurable::parent().instanceId() << " id " << name_
           << " data type " << data_type_str_;
}



void Variable::checkSubConfigurables()
{
}

DBContent& Variable::object() const
{
    assert(dbcontent_);
    return *dbcontent_;
}

const std::string& Variable::dbContentName() const
{
    assert(dbcontent_);
    return dbcontent_->name();
}

void Variable::name(const std::string& name)
{
    loginf << "Variable: name: old " << name_ << " new " << name;
    name_ = name;
}

//const std::string& Variable::metaTable() const
//{
//    assert(dbcontent_);
//    return dbcontent_->currentMetaTable();
//}

std::string Variable::dbColumnName() const
{
    return db_column_name_;
}

void Variable::dbColumnName(const std::string& value)
{
    db_column_name_ = value;
}

std::string Variable::dbTableName() const
{
    assert (dbcontent_);
    return dbcontent_->dbTableName();
}

std::string Variable::dbColumnIdentifier() const
{
    return dbTableName()+":"+dbColumnName();
}

//void Variable::setMinMax()
//{
//    assert(!min_max_set_);

//    assert(dbcontent_);
//    logdbg << "Variable " << dbcontent_->name() << " " << name_ << ": setMinMax";

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

//    logdbg << "Variable: setMinMax: min " << min_ << " max " << max_;
//}

//std::string Variable::getMinString()
//{
//    if (!min_max_set_)
//        setMinMax();  // already unit transformed

//    assert(min_max_set_);

//    logdbg << "Variable: getMinString: object " << dbContentName() << " name " << name()
//           << " returning " << min_;
//    return min_;
//}

//std::string Variable::getMaxString()
//{
//    if (!min_max_set_)
//        setMinMax();  // is already unit transformed

//    assert(min_max_set_);

//    logdbg << "Variable: getMaxString: object " << dbContentName() << " name " << name()
//           << " returning " << max_;
//    return max_;
//}

//std::string Variable::getMinStringRepresentation()
//{
//    if (representation_ == Representation::STANDARD)
//        return getMinString();
//    else
//        return getRepresentationStringFromValue(getMinString());
//}

//std::string Variable::getMaxStringRepresentation()
//{
//    if (representation_ == Representation::STANDARD)
//        return getMaxString();
//    else
//        return getRepresentationStringFromValue(getMaxString());
//}

std::string Variable::dimensionUnitStr()
{
    if (dimension_.size())
        return dimension_ + ":" + unit_;
    else
        return "";
}

VariableWidget* Variable::widget()
{
    if (!widget_)
    {
        widget_ = new VariableWidget(*this);
        assert(widget_);
    }

    return widget_;
}

Variable::Representation Variable::representation() const { return representation_; }

const std::string& Variable::representationString() const { return representation_str_; }

void Variable::representation(const Variable::Representation& representation)
{
    representation_str_ = representationToString(representation);
    representation_ = representation;
}

std::string Variable::getRepresentationStringFromValue(const std::string& value_str) const
{
    logdbg << "Variable: getRepresentationStringFromValue: value " << value_str << " data_type "
           << Property::asString(data_type_) << " representation "
           << representationToString(representation_);

    if (value_str == NULL_STRING)
        return value_str;

    if (representation_ == Variable::Representation::STANDARD)
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
                "Variable: getRepresentationStringFromValue: representation of string variable"
                " impossible");
        case PropertyDataType::JSON:
            throw std::invalid_argument(
                "Variable: getRepresentationStringFromValue: representation of json variable"
                " impossible");
        default:
            logerr << "Variable: getRepresentationStringFromValue:: unknown property type "
                   << Property::asString(data_type_);
            throw std::runtime_error(
                "Variable: getRepresentationStringFromValue:: unknown property type " +
                Property::asString(data_type_));
    }
}

std::string Variable::getValueStringFromRepresentation(
    const std::string& representation_str) const
{
    if (representation_str == NULL_STRING)
        return representation_str;

    assert(representation_ != Variable::Representation::STANDARD);

    if (representation_ == Variable::Representation::SECONDS_TO_TIME)
    {
        return String::getValueString(Utils::String::timeFromString(representation_str));
    }
    else if (representation_ == Variable::Representation::DEC_TO_OCTAL)
    {
        return String::getValueString(Utils::String::intFromOctalString(representation_str));
    }
    else if (representation_ == Variable::Representation::DEC_TO_HEX)
    {
        return String::getValueString(Utils::String::intFromHexString(representation_str));
    }
    else if (representation_ == Variable::Representation::FEET_TO_FLIGHTLEVEL)
    {
        return String::getValueString(std::stod(representation_str) * 100.0);
    }
    else if (representation_ == Variable::Representation::DATA_SRC_NAME)
    {
        DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

        if (ds_man.hasDBDataSource(representation_str))
            return std::to_string(ds_man.getDBDataSourceDSID(representation_str));

        // not found, return original

        return representation_str;
    }
    else
    {
        throw std::runtime_error(
            "Utils: String: getValueStringFromRepresentation: unknown representation " +
            std::to_string((int)representation_));
    }
}

std::string Variable::multiplyString(const std::string& value_str, double factor) const
{
    logdbg << "Variable: multiplyString: value " << value_str << " factor " << factor
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
                "Variable: multiplyString: multiplication of string variable impossible");
        case PropertyDataType::JSON:
            throw std::invalid_argument(
                "Variable: multiplyString: multiplication of json variable impossible");
        default:
            logerr << "Variable: multiplyString:: unknown property type "
                   << Property::asString(data_type_);
            throw std::runtime_error("Variable: multiplyString:: unknown property type " +
                                     Property::asString(data_type_));
    }

    logdbg << "Variable: multiplyString: return value " << return_string;

    return return_string;
}

const std::string& Variable::getLargerValueString(const std::string& value_a_str,
                                                     const std::string& value_b_str) const
{
    logdbg << "Variable: getLargerValueString: value a " << value_a_str << " b " << value_b_str
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
                "Variable: getLargerValueString: operation on string variable impossible");
        case PropertyDataType::JSON:
            throw std::invalid_argument(
                "Variable: getLargerValueString: operation on json variable impossible");
        default:
            logerr << "Variable: getLargerValueString:: unknown property type "
                   << Property::asString(data_type_);
            throw std::runtime_error("Variable: getLargerValueString:: unknown property type " +
                                     Property::asString(data_type_));
    }
}

const std::string& Variable::getSmallerValueString(const std::string& value_a_str,
                                                      const std::string& value_b_str) const
{
    logdbg << "Variable: getSmallerValueString: value a " << value_a_str << " b " << value_b_str
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
                "Variable: getSmallerValueString: operation on string variable impossible");
        case PropertyDataType::JSON:
            throw std::invalid_argument(
                "Variable: getSmallerValueString: operation on json variable impossible");
        default:
            logerr << "Variable: getSmallerValueString:: unknown property type "
                   << Property::asString(data_type_);
            throw std::runtime_error("Variable: getSmallerValueString:: unknown property type " +
                                     Property::asString(data_type_));
    }
}

bool Variable::hasShortName() const
{
    return short_name_.size();
}

std::string Variable::shortName() const
{
    return short_name_;
}

void Variable::shortName(const std::string& short_name)
{
    short_name_ = short_name;
}

bool Variable::isKey() const
{
    return is_key_;
}

void Variable::isKey(bool value)
{
    is_key_ = value;
}

std::string Variable::getDataSourcesAsString(const std::string& value) const
{
    assert(dbcontent_);

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    unsigned int ds_id = stoi(value);

    if (ds_man.hasDBDataSource(ds_id))
    {
        dbContent::DBDataSource& ds = ds_man.dbDataSource(ds_id);

        if (ds.hasShortName())
            return ds.shortName();
        else
            return ds.name();
    }

    // has no datasources, return original

    return value;
}

}
