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

#include "jsondatamapping.h"
#include "dbovariable.h"
#include "dbobjectmanager.h"
#include "dbobject.h"
#include "atsdb.h"
#include "jsonobjectparser.h"
#include "jsondatamappingwidget.h"

JSONDataMapping::JSONDataMapping (const std::string& class_id, const std::string& instance_id,
                                  JSONObjectParser& parent)
    : Configurable (class_id, instance_id, &parent)
{
    logdbg << "JSONDataMapping: constructor: this " << this;

    registerParameter("active", &active_, false);
    registerParameter("json_key", &json_key_, "");

    registerParameter("db_object_name", &db_object_name_, "");
    registerParameter("dbovariable_name", &dbovariable_name_, "");

    registerParameter("comment", &comment_, "");

    registerParameter("mandatory", &mandatory_, false);

    registerParameter ("format_data_type", &format_data_type_, "");
    registerParameter ("json_value_format", &json_value_format_, "");

    registerParameter("dimension", &dimension_, "");
    registerParameter("unit", &unit_, "");

    registerParameter("append_value", &append_value_, false);

    logdbg << "JSONDataMapping: ctor: dbo " << db_object_name_ << " var " << dbovariable_name_
           << " dim " << dimension_ << " unit " << unit_;

    sub_keys_ = Utils::String::split(json_key_, '.');
    has_sub_keys_ = sub_keys_.size() > 1;
    num_sub_keys_ = sub_keys_.size();

    logdbg << "JSONDataMapping: ctor: key " << json_key_ << " num subkeys " << sub_keys_.size();

    createSubConfigurables ();
}

JSONDataMapping& JSONDataMapping::operator=(JSONDataMapping&& other)
{
    logdbg << "JSONDataMapping: operator=: this " << this << " other " << &other;

    active_ = other.active_;
    json_key_ = other.json_key_;
    db_object_name_ = other.db_object_name_;
    dbovariable_name_ = other.dbovariable_name_;
    variable_ = other.variable_;

    mandatory_ = other.mandatory_;
    comment_ = other.comment_;
    //json_value_format_ = other.json_value_format_;
    format_data_type_ = other.format_data_type_;
    json_value_format_ = std::move(other.json_value_format_);

    dimension_ = other.dimension_;
    unit_ = other.unit_;

    append_value_ = other.append_value_;

    has_sub_keys_ = other.has_sub_keys_;
    sub_keys_ = std::move(other.sub_keys_);
    num_sub_keys_ = other.num_sub_keys_;

    other.configuration().updateParameterPointer ("active", &active_);
    other.configuration().updateParameterPointer ("json_key", &json_key_);
    other.configuration().updateParameterPointer ("db_object_name", &db_object_name_);
    other.configuration().updateParameterPointer ("dbovariable_name", &dbovariable_name_);
    other.configuration().updateParameterPointer ("mandatory", &mandatory_);
    other.configuration().updateParameterPointer ("comment", &comment_);
    other.configuration().updateParameterPointer ("format_data_type", &format_data_type_);
    other.configuration().updateParameterPointer ("json_value_format", &json_value_format_);
    other.configuration().updateParameterPointer ("dimension", &dimension_);
    other.configuration().updateParameterPointer ("unit", &unit_);
    other.configuration().updateParameterPointer ("append_value", &append_value_);

    widget_ = std::move(other.widget_);
    if (widget_)
        widget_->setMapping(*this);
    other.widget_ = nullptr;

    return static_cast<JSONDataMapping&>(Configurable::operator=(std::move(other)));
}

JSONDataMapping::~JSONDataMapping()
{
    logdbg << "JSONDataMapping: destructor: this " << this;
}

void JSONDataMapping::initializeIfRequired ()
{
    if (!initialized_)
        initialize();

    assert (initialized_);
}

std::string& JSONDataMapping::formatDataTypeRef()
{
    return format_data_type_;
}

bool JSONDataMapping::initialized() const
{
    return initialized_;
}

std::string JSONDataMapping::comment() const
{
    return comment_;
}

void JSONDataMapping::comment(const std::string &comment)
{
    comment_ = comment;
}

bool JSONDataMapping::appendValue() const
{
    return append_value_;
}

void JSONDataMapping::appendValue(bool append_value)
{
    append_value_ = append_value;
}

DBOVariable& JSONDataMapping::variable() const
{
    assert (initialized_);
    assert (variable_);
    return *variable_;
}

bool JSONDataMapping::mandatory() const
{
    return mandatory_;
}

void JSONDataMapping::mandatory(bool mandatory)
{
    loginf << "JSONDataMapping: mandatory: " << mandatory;
    mandatory_ = mandatory;
}

Format JSONDataMapping::jsonValueFormat() const
{
    assert (initialized_);
    //assert (json_value_format_);
    return json_value_format_;
}

Format& JSONDataMapping::jsonValueFormatRef()
{
    assert (initialized_);
    //assert (json_value_format_);
    return json_value_format_;
}

//void JSONDataMapping::jsonValueFormat(const Format &json_value_format)
//{
//    json_value_format_ = json_value_format;
//}

std::string JSONDataMapping::dbObjectName() const
{
    return db_object_name_;
}

void JSONDataMapping::dboVariableName(const std::string& name)
{
    loginf << "JSONDataMapping: dboVariableName: " << name;

    dbovariable_name_ = name;
    initialized_ = false;

    initialize ();
}

std::string JSONDataMapping::dboVariableName() const
{
    return dbovariable_name_;
}

const std::string& JSONDataMapping::jsonKey() const
{
    return json_key_;
}

void JSONDataMapping::jsonKey(const std::string &json_key)
{
    loginf << "JSONDataMapping: jsonKey: " << json_key;

    json_key_ = json_key;

    sub_keys_ = Utils::String::split(json_key_, '.');
    has_sub_keys_ = sub_keys_.size() > 1;
    num_sub_keys_ = sub_keys_.size();
}

//JSONDataMappingWidget* JSONDataMapping::widget ()
//{
//    if (!widget_)
//    {
//        widget_.reset(new JSONDataMappingWidget (*this));
//        assert (widget_);
//    }

//    return widget_.get(); // needed for qt integration, not pretty
//}

bool JSONDataMapping::active() const
{
    return active_;
}

void JSONDataMapping::active(bool active)
{
    loginf << "JSONDataMapping: active: " << active;
    active_ = active;
}

void JSONDataMapping::initialize ()
{
    loginf << "JSONDataMapping: initialize";

    assert (!initialized_);

    DBObjectManager& obj_man = ATSDB::instance().objectManager();

    if (db_object_name_.size() && !obj_man.existsObject(db_object_name_))
        logwrn << "JSONDataMapping: initialize: dbobject '" << db_object_name_ << "' does not exist";

    if (db_object_name_.size() && obj_man.existsObject(db_object_name_)
            && dbovariable_name_.size() && !obj_man.object(db_object_name_).hasVariable(dbovariable_name_))
        logwrn << "JSONDataMapping: initialize: dbobject " << db_object_name_ << " variable '" << dbovariable_name_
               << "' does not exist";

    if (db_object_name_.size() && obj_man.existsObject(db_object_name_)
            && dbovariable_name_.size() && obj_man.object(db_object_name_).hasVariable(dbovariable_name_))
        variable_ = &obj_man.object(db_object_name_).variable(dbovariable_name_);

    if (format_data_type_.size())
    {
        loginf << "JSONDataMapping: initialize: setting format from dt " << format_data_type_
               << " format " << json_value_format_;
        json_value_format_ = Format (Property::asDataType(format_data_type_), json_value_format_);
    }
    else if (variable_)
    {
        loginf << "JSONDataMapping: initialize: setting format from variable "<< variable_->name();
        json_value_format_ = Format (variable_->dataType(), json_value_format_);
    }
    else
        loginf << "JSONDataMapping: initialize: variable not set";

    if (append_value_)
    {
        if (sub_keys_.size() < 2)
            logwrn << "JSONDataMapping: initialize: append set but olny " << sub_keys_.size() << " sub keys";

        if (!variable_)
            logwrn << "JSONDataMapping: initialize: append set not variable set";

    }

    initialized_ =  true;
}

template<typename T>
bool JSONDataMapping::findAndSetValue(const nlohmann::json& j, NullableVector<T>& array_list, size_t row_cnt) const
{
    const nlohmann::json* val_ptr = findKey(j);

    if (val_ptr == nullptr || *val_ptr == nullptr)
    {
        if (mandatory_)
            return true;

        //array_list.setNull(row_cnt);
        return false;
    }
    else
    {
        try
        {
            if (append_value_)
                appendValue(val_ptr, array_list, row_cnt);
            else
                setValue(val_ptr, array_list, row_cnt);

            return false; // everything ok
        }
        catch (nlohmann::json::exception& e)
        {
            logerr  <<  "JSONDataMapping: setValue: key " << json_key_ << " json exception "
                     << e.what() << " property " << array_list.propertyName();
            array_list.setNull(row_cnt);
            return true; // last entry might be wrong
        }
    }
}

//template void foo::do<int>(const int&);

template bool JSONDataMapping::findAndSetValue(const nlohmann::json& j,
NullableVector<bool>& array_list, size_t row_cnt) const;
template bool JSONDataMapping::findAndSetValue(const nlohmann::json& j,
NullableVector<char>& array_list, size_t row_cnt) const;
template bool JSONDataMapping::findAndSetValue(const nlohmann::json& j,
NullableVector<unsigned char>& array_list, size_t row_cnt) const;
template bool JSONDataMapping::findAndSetValue(const nlohmann::json& j,
NullableVector<int>& array_list, size_t row_cnt) const;
template bool JSONDataMapping::findAndSetValue(const nlohmann::json& j,
NullableVector<unsigned int>& array_list, size_t row_cnt) const;
template bool JSONDataMapping::findAndSetValue(const nlohmann::json& j,
NullableVector<long int>& array_list, size_t row_cnt) const;
template bool JSONDataMapping::findAndSetValue(const nlohmann::json& j,
NullableVector<unsigned long int>& array_list, size_t row_cnt) const;
template bool JSONDataMapping::findAndSetValue(const nlohmann::json& j,
NullableVector<float>& array_list, size_t row_cnt) const;
template bool JSONDataMapping::findAndSetValue(const nlohmann::json& j,
NullableVector<double>& array_list, size_t row_cnt) const;
template bool JSONDataMapping::findAndSetValue(const nlohmann::json& j,
NullableVector<std::string>& array_list, size_t row_cnt) const;

const nlohmann::json* JSONDataMapping::findKey (const nlohmann::json& j) const
{
    const nlohmann::json* val_ptr = &j;

    if (has_sub_keys_)
    {
        for (const std::string& sub_key : sub_keys_)
        {
            if (val_ptr->contains (sub_key))
            {
                if (sub_key == sub_keys_.back()) // last found
                {
                    val_ptr = &val_ptr->at(sub_key);
                    break;
                }

                if (val_ptr->at(sub_key).is_object()) // not last, step in
                    val_ptr = &val_ptr->at(sub_key);
                else // not last key, and not object
                {
                    val_ptr = nullptr;
                    break;
                }
            }
            else // not found
            {
                val_ptr = nullptr;
                break;
            }
        }
    }
    else
    {
        if (val_ptr->contains (json_key_))
            val_ptr = &val_ptr->at(json_key_);
        else
            val_ptr = nullptr;
    }
    return val_ptr;
}

template<typename T>
void JSONDataMapping::setValue(const nlohmann::json* val_ptr, NullableVector<T>& array_list, size_t row_cnt) const
{
    assert (val_ptr);

    logdbg << "JSONDataMapping: setValue: key " << json_key_ << " json " << val_ptr->type_name()
           << " '" << val_ptr->dump() << "' format '" << json_value_format_ << "'";

    if (json_value_format_ == "")
        array_list.set(row_cnt, *val_ptr);
    else
        array_list.setFromFormat(row_cnt, json_value_format_, Utils::JSON::toString(*val_ptr));

    logdbg << "JSONDataMapping: setValue: key " << json_key_ << " json " << *val_ptr
           << " buffer " << array_list.get(row_cnt);
}

template<typename T>
void JSONDataMapping::appendValue(const nlohmann::json* val_ptr, NullableVector<T>& array_list, size_t row_cnt) const
{
    assert (val_ptr);

    logdbg << "JSONDataMapping: appendValue: key " << json_key_ << " json " << val_ptr->type_name()
           << " '" << val_ptr->dump() << "' format '" << json_value_format_ << "'";

    if (json_value_format_ == "")
        array_list.append(row_cnt, *val_ptr);
    else
        array_list.appendFromFormat(row_cnt, json_value_format_, Utils::JSON::toString(*val_ptr));

    logdbg << "JSONDataMapping: appendValue: key " << json_key_ << " json " << *val_ptr
           << " buffer " << array_list.get(row_cnt);
}

void JSONDataMapping::setValue(const nlohmann::json* val_ptr, NullableVector<bool>& array_list, size_t row_cnt) const
{
    assert (val_ptr);

    bool tmp_bool;

    if (val_ptr->is_number())
    {
        unsigned int tmp = *val_ptr;
        assert (tmp == 0 || tmp == 1);
        tmp_bool = static_cast<bool> (tmp);
    }
    else
    {
        tmp_bool = *val_ptr; // works for bool, throws for rest
    }

    if (json_value_format_ == "")
        array_list.set(row_cnt, tmp_bool);
    else
        array_list.setFromFormat(row_cnt, json_value_format_, Utils::JSON::toString(tmp_bool));

    logdbg << "JSONDataMapping: setValue(bool): json " << tmp_bool << " buffer "
           << array_list.get(row_cnt);
}

void JSONDataMapping::appendValue(const nlohmann::json* val_ptr, NullableVector<bool>& array_list, size_t row_cnt) const
{
    assert (val_ptr);

    bool tmp_bool;

    if (val_ptr->is_number())
    {
        unsigned int tmp = *val_ptr;
        assert (tmp == 0 || tmp == 1);
        tmp_bool = static_cast<bool> (tmp);
    }
    else
        tmp_bool = *val_ptr; // works for bool, throws for rest

    if (json_value_format_ == "")
        array_list.append(row_cnt, tmp_bool);
    else
        array_list.appendFromFormat(row_cnt, json_value_format_, Utils::JSON::toString(tmp_bool));

    logdbg << "JSONDataMapping: appendValue(bool): json " << tmp_bool << " buffer "
           << array_list.get(row_cnt);
}

void JSONDataMapping::setValue(const nlohmann::json* val_ptr, NullableVector<char>& array_list, size_t row_cnt) const
{
    assert (val_ptr);

    if (json_value_format_ == "")
        array_list.set(row_cnt, static_cast<int> (*val_ptr));
    else
        array_list.setFromFormat(row_cnt, json_value_format_,
                                 Utils::JSON::toString(static_cast<int> (*val_ptr)));

    logdbg << "JSONDataMapping: setValue(char): json " << static_cast<int> (*val_ptr) << " buffer "
           << array_list.get(row_cnt);
}

void JSONDataMapping::appendValue(const nlohmann::json* val_ptr, NullableVector<char>& array_list, size_t row_cnt) const
{
    assert (val_ptr);

    if (json_value_format_ == "")
        array_list.append(row_cnt, static_cast<int> (*val_ptr));
    else
        array_list.appendFromFormat(row_cnt, json_value_format_,
                                    Utils::JSON::toString(static_cast<int> (*val_ptr)));

    logdbg << "JSONDataMapping: appendValue(char): json " << static_cast<int> (*val_ptr) << " buffer "
           << array_list.get(row_cnt);
}

template void JSONDataMapping::setValue(const nlohmann::json* val_ptr,
NullableVector<unsigned char>& array_list, size_t row_cnt) const;
template void JSONDataMapping::appendValue(const nlohmann::json* val_ptr,
NullableVector<unsigned char>& array_list, size_t row_cnt) const;

template void JSONDataMapping::setValue(const nlohmann::json* val_ptr,
NullableVector<int>& array_list, size_t row_cnt) const;
template void JSONDataMapping::appendValue(const nlohmann::json* val_ptr,
NullableVector<int>& array_list, size_t row_cnt) const;

template void JSONDataMapping::setValue(const nlohmann::json* val_ptr,
NullableVector<unsigned int>& array_list, size_t row_cnt) const;
template void JSONDataMapping::appendValue(const nlohmann::json* val_ptr,
NullableVector<unsigned int>& array_list, size_t row_cnt) const;

template void JSONDataMapping::setValue(const nlohmann::json* val_ptr,
NullableVector<long int>& array_list, size_t row_cnt) const;
template void JSONDataMapping::appendValue(const nlohmann::json* val_ptr,
NullableVector<long int>& array_list, size_t row_cnt) const;

template void JSONDataMapping::setValue(const nlohmann::json* val_ptr,
NullableVector<unsigned long int>& array_list, size_t row_cnt) const;
template void JSONDataMapping::appendValue(const nlohmann::json* val_ptr,
NullableVector<unsigned long int>& array_list, size_t row_cnt) const;

template void JSONDataMapping::setValue(const nlohmann::json* val_ptr,
NullableVector<float>& array_list, size_t row_cnt) const;
template void JSONDataMapping::appendValue(const nlohmann::json* val_ptr,
NullableVector<float>& array_list, size_t row_cnt) const;

template void JSONDataMapping::setValue(const nlohmann::json* val_ptr,
NullableVector<double>& array_list, size_t row_cnt) const;
template void JSONDataMapping::appendValue(const nlohmann::json* val_ptr,
NullableVector<double>& array_list, size_t row_cnt) const;

void JSONDataMapping::setValue(const nlohmann::json* val_ptr, NullableVector<std::string>& array_list, size_t row_cnt) const
{
    assert (val_ptr);

    if (json_value_format_ == "")
        array_list.set(row_cnt, Utils::JSON::toString(*val_ptr));
    else
        array_list.setFromFormat(row_cnt, json_value_format_, Utils::JSON::toString(*val_ptr));

    logdbg << "JSONDataMapping: setValue(string): json " << Utils::JSON::toString(*val_ptr)
           << " buffer " << array_list.get(row_cnt);
}

void JSONDataMapping::appendValue(const nlohmann::json* val_ptr, NullableVector<std::string>& array_list, size_t row_cnt) const
{
    assert (val_ptr);

    if (json_value_format_ == "")
        array_list.append(row_cnt, Utils::JSON::toString(*val_ptr));
    else
        array_list.appendFromFormat(row_cnt, json_value_format_, Utils::JSON::toString(*val_ptr));

    logdbg << "JSONDataMapping: setValue(string): json " << Utils::JSON::toString(*val_ptr)
           << " buffer " << array_list.get(row_cnt);

}
