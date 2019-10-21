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
        loginf << "JSONDataMapping: updateVariable: setting format from variable "<< variable_->name();
        json_value_format_ = Format (variable_->dataType(), json_value_format_);
    }
    else
        loginf << "JSONDataMapping: updateVariable: variable not set";

    initialized_ =  true;
}

