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

#include "jsonobjectparser.h"

#include <algorithm>

#include "compass.h"
#include "buffer.h"
#include "configuration.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "stringconv.h"
#include "unit.h"
#include "unitmanager.h"
#include "util/json.h"

using namespace nlohmann;
using namespace Utils;

JSONObjectParser::JSONObjectParser(const std::string& class_id, const std::string& instance_id,
                                   Configurable* parent)
    : Configurable(class_id, instance_id, parent)
{
    registerParameter("name", &name_, "");
    registerParameter("db_object_name", &db_object_name_, "");

    registerParameter("json_container_key", &json_container_key_, "");
    registerParameter("json_key", &json_key_, "*");
    registerParameter("json_value", &json_value_, "");

    registerParameter("override_data_source", &override_data_source_, false);
    registerParameter("data_source_variable_name", &data_source_variable_name_, "");

    assert(db_object_name_.size());

    if (!name_.size())
        name_ = db_object_name_;

    assert(name_.size());

    createSubConfigurables();

    json_values_vector_ = String::split(json_value_, ',');
}

JSONObjectParser& JSONObjectParser::operator=(JSONObjectParser&& other)
{
    name_ = other.name_;
    db_object_name_ = other.db_object_name_;
    db_object_ = other.db_object_;

    json_container_key_ = other.json_container_key_;
    json_key_ = other.json_key_;
    json_value_ = other.json_value_;
    json_values_vector_ = other.json_values_vector_;

    data_mappings_ = std::move(other.data_mappings_);

    var_list_ = other.var_list_;

    override_data_source_ = other.override_data_source_;
    data_source_variable_name_ = other.data_source_variable_name_;

    initialized_ = other.initialized_;

    not_parse_all_ = other.not_parse_all_;

    list_ = other.list_;

    other.configuration().updateParameterPointer("name", &name_);
    other.configuration().updateParameterPointer("db_object_name", &db_object_name_);
    other.configuration().updateParameterPointer("json_key", &json_key_);
    other.configuration().updateParameterPointer("json_value", &json_value_);
    other.configuration().updateParameterPointer("override_data_source", &override_data_source_);

    widget_ = std::move(other.widget_);
    if (widget_)
        widget_->setParser(*this);
    other.widget_ = nullptr;

    return static_cast<JSONObjectParser&>(Configurable::operator=(std::move(other)));
}

void JSONObjectParser::generateSubConfigurable(const std::string& class_id,
                                               const std::string& instance_id)
{
    if (class_id == "JSONDataMapping")
    {
        data_mappings_.emplace_back(class_id, instance_id, *this);
    }
    else
        throw std::runtime_error("DBObject: generateSubConfigurable: unknown class_id " + class_id);
}

DBObject& JSONObjectParser::dbObject() const
{
    assert(db_object_);
    return *db_object_;
}

std::string JSONObjectParser::JSONKey() const { return json_key_; }

void JSONObjectParser::JSONKey(const std::string& json_key)
{
    loginf << "JSONObjectParser: JSONKey: " << json_key;

    json_key_ = json_key;
}

std::string JSONObjectParser::JSONValue() const { return json_value_; }

void JSONObjectParser::JSONValue(const std::string& json_value)
{
    loginf << "JSONObjectParser: JSONValue: " << json_value;

    json_value_ = json_value;
    json_values_vector_ = String::split(json_value_, ',');
}

std::string JSONObjectParser::JSONContainerKey() const { return json_container_key_; }

void JSONObjectParser::JSONContainerKey(const std::string& key)
{
    loginf << "JSONObjectParser: JSONContainerKey: " << key;

    json_container_key_ = key;
}

void JSONObjectParser::initialize()
{
    assert(!db_object_);

    DBObjectManager& obj_man = COMPASS::instance().objectManager();

    if (!obj_man.existsObject(db_object_name_))
        logwrn << "JSONObjectParser: initialize: dbobject '" << db_object_name_
               << "' does not exist";
    else
        db_object_ = &obj_man.object(db_object_name_);

    assert(db_object_);

    if (!initialized_)
    {
        for (auto& mapping : data_mappings_)
        {
            if (!mapping.active())
            {
                assert(!mapping.mandatory());
                continue;
            }

            mapping.initializeIfRequired();

            list_.addProperty(mapping.variable().name(), mapping.variable().dataType());
            var_list_.add(mapping.variable());
        }

        if (override_data_source_)
        {
            assert(data_source_variable_name_.size());
            assert(db_object_->hasVariable(data_source_variable_name_));

            list_.addProperty(data_source_variable_name_, PropertyDataType::INT);
            var_list_.add(db_object_->variable(data_source_variable_name_));
        }

        not_parse_all_ = (json_key_ != "*") && (json_value_ != "*");

        initialized_ = true;
    }
}

std::shared_ptr<Buffer> JSONObjectParser::getNewBuffer() const
{
    assert(initialized_);
    assert(db_object_);
    return std::make_shared<Buffer>(list_, db_object_->name());
}

void JSONObjectParser::appendVariablesToBuffer(Buffer& buffer) const
{
    assert(initialized_);
    assert(db_object_);

    for (auto& p_it : list_.properties())
    {
        if (!buffer.properties().hasProperty(p_it.name()))
        {
            buffer.addProperty(p_it.name(), p_it.dataType());
        }
    }
}

bool JSONObjectParser::parseJSON(nlohmann::json& j, Buffer& buffer) const
{
    assert(initialized_);

    size_t row_cnt = buffer.size();
    size_t skipped_cnt = 0;

    bool parsed_any = false;

    if (json_container_key_.size())
    {
        bool parsed = false;

        if (j.contains(json_container_key_))
        {
            json& ac_list = j.at(json_container_key_);
            assert(ac_list.is_array());

            // loginf << "found target report array in '" << json_container_key_  << "', parsing";

            for (auto tr_it = ac_list.begin(); tr_it != ac_list.end(); ++tr_it)
            {
                // logdbg << "new target report";

                json& tr = tr_it.value();
                assert(tr.is_object());

                parsed = parseTargetReport(tr, buffer, row_cnt);

                if (parsed)
                    ++row_cnt;
                else
                    ++skipped_cnt;

                parsed_any |= parsed;
            }
        }
        else  // parsed stays false
            loginf << "JSONObjectParser: parseJSON: found target report array but '"
                   << json_container_key_ << "' not found";
    }
    else
    {
        logdbg << "JSONObjectParser: parseJSON: found single target report";
        assert(j.is_object());

        parsed_any = parseTargetReport(j, buffer, row_cnt);
    }

    return parsed_any;
}

void JSONObjectParser::createMappingStubs(nlohmann::json& j)
{
    assert(initialized_);

    if (json_container_key_.size())
    {
        if (j.contains(json_container_key_))
        {
            json& ac_list = j.at(json_container_key_);
            assert(ac_list.is_array());

            // loginf << "found target report array in '" << json_container_key_  << "', parsing";

            for (auto tr_it = ac_list.begin(); tr_it != ac_list.end(); ++tr_it)
            {
                // logdbg << "new target report";

                json& tr = tr_it.value();
                assert(tr.is_object());

                createMappingsFromTargetReport(tr);
            }
        }
        else  // parsed stays false
            loginf << "JSONObjectParser: createMappingStubs: found target report array but '"
                   << json_container_key_ << "' not found";
    }
    else
    {
        logdbg << "JSONObjectParser: createMappingStubs: found single target report";
        assert(j.is_object());

        createMappingsFromTargetReport(j);
    }

    return;
}

bool JSONObjectParser::parseTargetReport(const nlohmann::json& tr, Buffer& buffer,
                                         size_t row_cnt) const
{
    // check key match
    if (not_parse_all_)
    {
        if (tr.contains(json_key_))
        {
            if (std::find(json_values_vector_.begin(), json_values_vector_.end(),
                          Utils::JSON::toString(tr.at(json_key_))) == json_values_vector_.end())
            {
                logdbg << "JSONObjectParser: parseTargetReport: skipping because of wrong key "
                       << tr.at(json_key_) << " value " << Utils::JSON::toString(tr.at(json_key_));
                return false;
            }
            else
                logdbg << "JSONObjectParser: parseTargetReport: parsing with correct key and value";
        }
        else
        {
            logdbg << "JSONObjectParser: parseTargetReport: skipping because of missing key '"
                   << json_key_ << "'";
            return false;
        }
    }

    PropertyDataType data_type;
    std::string current_var_name;

    bool mandatory_missing{false};

    for (const auto& map_it : data_mappings_)
    {
        if (!map_it.active())
        {
            assert(!map_it.mandatory());
            continue;
        }

        // logdbg << "setting data mapping key " << data_it.jsonKey();

        data_type = map_it.variable().dataType();
        current_var_name = map_it.variable().name();

        switch (data_type)
        {
            case PropertyDataType::BOOL:
            {
                logdbg << "JSONObjectParser: parseTargetReport: bool " << current_var_name
                       << " format '" << map_it.jsonValueFormat() << "'";
                assert(buffer.has<bool>(current_var_name));
                mandatory_missing =
                    map_it.findAndSetValue(tr, buffer.get<bool>(current_var_name), row_cnt);

                break;
            }
            case PropertyDataType::CHAR:
            {
                logdbg << "JSONObjectParser: parseTargetReport: char " << current_var_name
                       << " format '" << map_it.jsonValueFormat() << "'";
                assert(buffer.has<char>(current_var_name));
                mandatory_missing =
                    map_it.findAndSetValue(tr, buffer.get<char>(current_var_name), row_cnt);

                break;
            }
            case PropertyDataType::UCHAR:
            {
                logdbg << "JSONObjectParser: parseTargetReport: uchar " << current_var_name
                       << " format '" << map_it.jsonValueFormat() << "'";
                assert(buffer.has<unsigned char>(current_var_name));
                mandatory_missing = map_it.findAndSetValue(
                    tr, buffer.get<unsigned char>(current_var_name), row_cnt);

                break;
            }
            case PropertyDataType::INT:
            {
                logdbg << "JSONObjectParser: parseTargetReport: int " << current_var_name
                       << " format '" << map_it.jsonValueFormat() << "'";
                assert(buffer.has<int>(current_var_name));
                mandatory_missing =
                    map_it.findAndSetValue(tr, buffer.get<int>(current_var_name), row_cnt);

                break;
            }
            case PropertyDataType::UINT:
            {
                logdbg << "JSONObjectParser: parseTargetReport: uint " << current_var_name
                       << " format '" << map_it.jsonValueFormat() << "'";
                assert(buffer.has<unsigned int>(current_var_name));
                mandatory_missing =
                    map_it.findAndSetValue(tr, buffer.get<unsigned int>(current_var_name), row_cnt);

                break;
            }
            case PropertyDataType::LONGINT:
            {
                logdbg << "JSONObjectParser: parseTargetReport: long " << current_var_name
                       << " format '" << map_it.jsonValueFormat() << "'";
                assert(buffer.has<long int>(current_var_name));
                mandatory_missing =
                    map_it.findAndSetValue(tr, buffer.get<long int>(current_var_name), row_cnt);

                break;
            }
            case PropertyDataType::ULONGINT:
            {
                logdbg << "JSONObjectParser: parseTargetReport: ulong " << current_var_name
                       << " format '" << map_it.jsonValueFormat() << "'";
                assert(buffer.has<unsigned long>(current_var_name));
                mandatory_missing = map_it.findAndSetValue(
                    tr, buffer.get<unsigned long>(current_var_name), row_cnt);

                break;
            }
            case PropertyDataType::FLOAT:
            {
                logdbg << "JSONObjectParser: parseTargetReport: float " << current_var_name
                       << " format '" << map_it.jsonValueFormat() << "'";
                assert(buffer.has<float>(current_var_name));
                mandatory_missing =
                    map_it.findAndSetValue(tr, buffer.get<float>(current_var_name), row_cnt);

                break;
            }
            case PropertyDataType::DOUBLE:
            {
                logdbg << "JSONObjectParser: parseTargetReport: double " << current_var_name
                       << " format '" << map_it.jsonValueFormat() << "'";
                assert(buffer.has<double>(current_var_name));
                mandatory_missing =
                    map_it.findAndSetValue(tr, buffer.get<double>(current_var_name), row_cnt);

                break;
            }
            case PropertyDataType::STRING:
            {
                logdbg << "JSONObjectParser: parseTargetReport: string " << current_var_name
                       << " format '" << map_it.jsonValueFormat() << "'";
                assert(buffer.has<std::string>(current_var_name));

//                if (buffer.dboName() == "Tracker" && current_var_name == "ground_bit")
//                {
//                    loginf << "JSONObjectParser: parseTargetReport: string " << current_var_name
//                           << " format '" << map_it.jsonValueFormat() << "' mand " << mandatory_missing;

//                    mandatory_missing =
//                        map_it.findAndSetValue(tr, buffer.get<std::string>(current_var_name), row_cnt, true);
//                }
//                else
                mandatory_missing =
                        map_it.findAndSetValue(tr, buffer.get<std::string>(current_var_name), row_cnt);

                break;
            }
            default:
                logerr << "JSONObjectParser: parseTargetReport: impossible for property type "
                       << Property::asString(data_type);
                throw std::runtime_error(
                    "JsonMapping: parseTargetReport: impossible property type " +
                    Property::asString(data_type));
        }

        if (mandatory_missing)
        {
            // TODO make configurable
            logdbg << "JSONObjectParser '" << name_ << "': parseTargetReport: mandatory variable '"
                   << current_var_name << "' missing in: \n"
                   << tr.dump(4);
            break;
        }
    }

    if (mandatory_missing)
    {
        // cleanup
        if (buffer.size() > row_cnt)
            buffer.cutToSize(row_cnt);
    }

    return !mandatory_missing;
}

void JSONObjectParser::createMappingsFromTargetReport(const nlohmann::json& tr)
{
    // check key match
    if (not_parse_all_)
    {
        if (tr.contains(json_key_))
        {
            if (std::find(json_values_vector_.begin(), json_values_vector_.end(),
                          Utils::JSON::toString(tr.at(json_key_))) == json_values_vector_.end())
            {
                logdbg << "JSONObjectParser: createMappingsFromTargetReport: skipping because of "
                          "wrong key "
                       << tr.at(json_key_) << " value " << Utils::JSON::toString(tr.at(json_key_));
                return;
            }
            else
                logdbg << "JSONObjectParser: createMappingsFromTargetReport: parsing with correct "
                          "key and value";
        }
        else
        {
            logdbg << "JSONObjectParser: createMappingsFromTargetReport: skipping because of "
                      "missing key '"
                   << json_key_ << "'";
            return;
        }
    }

    checkIfKeysExistsInMappings("", tr);
}

void JSONObjectParser::checkIfKeysExistsInMappings(const std::string& location,
                                                   const nlohmann::json& j, bool is_in_array)
{
    if (j.is_array())  // do map arrays
    {
        if (!j.size())  // do not map if empty
            return;

        bool j_array_contains_only_primitives = true;  // only

        for (auto& j_it : j.get<json::array_t>())  // iterate over array
        {
            if (j_it.is_object())  // only parse sub-objects
            {
                j_array_contains_only_primitives = false;
                checkIfKeysExistsInMappings(location, j_it, true);
            }
            else if (j_it.is_array())
                j_array_contains_only_primitives = false;
        }

        if (!j_array_contains_only_primitives)
            return;  // if objects inside, only parse objects
    }

    if (j.is_object())
    {
        for (auto& j_it : j.get<json::object_t>())
        {
            if (location.size())
                checkIfKeysExistsInMappings(location + "." + j_it.first, j_it.second, is_in_array);
            else
                checkIfKeysExistsInMappings(j_it.first, j_it.second, is_in_array);
        }
        return;
    }

    // loginf << "JSONObjectParser: checkIfKeysExistsInMappings: checking key '" << location << "'";

    bool found = false;

    for (auto& map_it : data_mappings_)
    {
        if (map_it.jsonKey() == location)
        {
            found = true;

            if (!map_it.comment().size())
            {
                std::stringstream ss;

                ss << "Type " << j.type_name() << ", value " << j.dump();
                map_it.comment(ss.str());
            }
            break;
        }
    }

    if (!found)
    {
        loginf << "JSONObjectParser: checkIfKeysExistsInMappings: creating new mapping for dbo "
               << db_object_name_ << "'" << location << "' type " << j.type_name() << " value "
               << j.dump() << " in array " << is_in_array;

        Configuration& new_cfg = configuration().addNewSubConfiguration("JSONDataMapping");
        new_cfg.addParameterString("json_key", location);
        new_cfg.addParameterString("db_object_name", db_object_name_);

        if (is_in_array)
            new_cfg.addParameterBool("in_array", true);

        std::stringstream ss;
        ss << "Type " << j.type_name() << ", value " << j.dump();
        new_cfg.addParameterString("comment", ss.str());

        generateSubConfigurable("JSONDataMapping", new_cfg.getInstanceId());
    }
}

bool JSONObjectParser::hasMapping(unsigned int index) const
{
    return index < data_mappings_.size();
}
void JSONObjectParser::removeMapping(unsigned int index)
{
    assert(hasMapping(index));

    JSONDataMapping& mapping = data_mappings_.at(index);

    loginf << "JSONObjectParser: removeMapping: index " << index << " key " << mapping.jsonKey()
           << " instance " << mapping.instanceId();

    logdbg << "JSONObjectParser: removeMapping: size " << data_mappings_.size();

    if (mapping.active() && mapping.initialized())
    {
        if (list_.hasProperty(mapping.variable().name()))
            list_.removeProperty(mapping.variable().name());
        if (mapping.hasVariable() && var_list_.hasVariable(mapping.variable()))
            var_list_.removeVariable(mapping.variable());
    }

    logdbg << "JSONObjectParser: removeMapping: removing";
    data_mappings_.erase(data_mappings_.begin() + index);

    logdbg << "JSONObjectParser: removeMapping: size " << data_mappings_.size();
}

void JSONObjectParser::transformBuffer(Buffer& buffer, size_t index) const
{
    assert(db_object_);

    logdbg << "JSONObjectParser: transformBuffer: object " << db_object_->name();

    assert(index < buffer.size());

    if (override_data_source_)
    {
        logdbg << "JSONObjectParser: transformBuffer: overiding data source for object "
               << db_object_->name() << " ds id name '" << data_source_variable_name_ << "'";
        assert(data_source_variable_name_.size());
        assert(buffer.has<int>(data_source_variable_name_));

        NullableVector<int>& ds_id_vector = buffer.get<int>(data_source_variable_name_);

        ds_id_vector.set(index, 0);
    }

    for (auto& data_it : data_mappings_)
    {
        if (!data_it.active())
            continue;

        if (data_it.dimension() != data_it.variable().dimension())
            logwrn << "JSONObjectParser: transformBuffer: variable " << data_it.variable().name()
                   << " has differing dimensions " << data_it.dimension() << " "
                   << data_it.variable().dimension();
        else if (data_it.unit() != data_it.variable().unit())  // do unit conversion stuff
        {
            logdbg << "JSONObjectParser: transformBuffer: variable " << data_it.variable().name()
                   << " of same dimension has different units " << data_it.unit() << " "
                   << data_it.variable().unit();

            if (!UnitManager::instance().hasDimension(data_it.dimension()))
            {
                logerr << "JSONObjectParser: transformBuffer: unknown dimension '"
                       << data_it.dimension() << "'";
                throw std::runtime_error("JSONObjectParser: transformBuffer: unknown dimension '" +
                                         data_it.dimension() + "'");
            }

            const Dimension& dimension =
                UnitManager::instance().dimension(data_it.variable().dimension());
            double factor;

            if (!dimension.hasUnit(data_it.unit()))
                logerr << "JSONObjectParser: transformBuffer: dimension '" << data_it.dimension()
                       << "' has unknown unit '" << data_it.unit() << "'";

            if (!dimension.hasUnit(data_it.variable().unit()))
                logerr << "JSONObjectParser: transformBuffer: dimension '"
                       << data_it.variable().dimension() << "' has unknown unit '"
                       << data_it.variable().unit() << "'";

            factor = dimension.getFactor(data_it.unit(), data_it.variable().unit());
            std::string current_var_name = data_it.variable().name();
            PropertyDataType data_type = data_it.variable().dataType();

            logdbg << "JSONObjectParser: transformBuffer: variable " << data_it.variable().name()
                   << " correct unit transformation with factor " << factor << " data unit "
                   << data_it.unit() << " variable unit " << data_it.variable().unit();

            switch (data_type)
            {
                case PropertyDataType::BOOL:
                {
                    assert(buffer.has<bool>(current_var_name));

                    NullableVector<bool>& array_list = buffer.get<bool>(current_var_name);

                    if (array_list.isNull(index))
                        break;

                    logwrn << "JSONObjectParser: transformBuffer: double multiplication of boolean "
                              "variable "
                           << current_var_name << " factor " << factor;
                    array_list.set(index, array_list.get(index) * factor);
                    break;
                }
                case PropertyDataType::CHAR:
                {
                    assert(buffer.has<char>(current_var_name));
                    NullableVector<char>& array_list = buffer.get<char>(current_var_name);

                    if (array_list.isNull(index))
                        break;

                    logwrn << "JSONObjectParser: transformBuffer: double multiplication of char "
                              "variable "
                           << current_var_name << " factor " << factor;
                    array_list.set(index, array_list.get(index) * factor);
                    break;
                }
                case PropertyDataType::UCHAR:
                {
                    assert(buffer.has<unsigned char>(current_var_name));
                    NullableVector<unsigned char>& array_list =
                        buffer.get<unsigned char>(current_var_name);

                    if (array_list.isNull(index))
                        break;

                    logwrn << "JSONObjectParser: transformBuffer: double multiplication of "
                              "unsigned char variable "
                           << current_var_name << " factor " << factor;
                    array_list.set(index, array_list.get(index) * factor);
                    break;
                }
                case PropertyDataType::INT:
                {
                    assert(buffer.has<int>(current_var_name));
                    NullableVector<int>& array_list = buffer.get<int>(current_var_name);

                    if (array_list.isNull(index))
                        break;

                    logdbg << "JSONObjectParser: transformBuffer: double multiplication of int "
                              "variable "
                           << current_var_name << " factor " << factor;
                    array_list.set(index, array_list.get(index) * factor);
                    break;
                }
                case PropertyDataType::UINT:
                {
                    assert(buffer.has<unsigned int>(current_var_name));
                    NullableVector<unsigned int>& array_list =
                        buffer.get<unsigned int>(current_var_name);

                    if (array_list.isNull(index))
                        break;

                    logdbg << "JSONObjectParser: transformBuffer: double multiplication of uint "
                              "variable "
                           << current_var_name << " factor " << factor;
                    array_list.set(index, array_list.get(index) * factor);
                    break;
                }
                case PropertyDataType::LONGINT:
                {
                    assert(buffer.has<long int>(current_var_name));
                    NullableVector<long int>& array_list = buffer.get<long int>(current_var_name);

                    if (array_list.isNull(index))
                        break;

                    logdbg << "JSONObjectParser: transformBuffer: double multiplication of long "
                              "int variable "
                           << current_var_name << " factor " << factor;
                    array_list.set(index, array_list.get(index) * factor);
                    break;
                }
                case PropertyDataType::ULONGINT:
                {
                    assert(buffer.has<unsigned long>(current_var_name));
                    NullableVector<unsigned long>& array_list =
                        buffer.get<unsigned long>(current_var_name);

                    if (array_list.isNull(index))
                        break;

                    logdbg << "JSONObjectParser: transformBuffer: double multiplication of "
                              "unsigned long int variable "
                           << current_var_name << " factor " << factor;
                    array_list.set(index, array_list.get(index) * factor);
                    break;
                }
                case PropertyDataType::FLOAT:
                {
                    assert(buffer.has<float>(current_var_name));
                    NullableVector<float>& array_list = buffer.get<float>(current_var_name);

                    if (array_list.isNull(index))
                        break;

                    logdbg << "JSONObjectParser: transformBuffer: double multiplication of float "
                              "variable "
                           << current_var_name << " factor " << factor;
                    array_list.set(index, array_list.get(index) * factor);
                    break;
                }
                case PropertyDataType::DOUBLE:
                {
                    assert(buffer.has<double>(current_var_name));
                    NullableVector<double>& array_list = buffer.get<double>(current_var_name);

                    if (array_list.isNull(index))
                        break;

                    logdbg << "JSONObjectParser: transformBuffer: double multiplication of double "
                              "variable "
                           << current_var_name << " factor " << factor;
                    array_list.set(index, array_list.get(index) * factor);
                    break;
                }
                case PropertyDataType::STRING:
                    logerr << "JSONObjectParser: transformBuffer: unit transformation for string "
                              "variable "
                           << data_it.variable().name() << " impossible";
                    break;
                default:
                    logerr << "JSONObjectParser: transformBuffer: unknown property type "
                           << Property::asString(data_type);
                    throw std::runtime_error(
                        "JSONObjectParser: transformBuffer: unknown property type " +
                        Property::asString(data_type));
            }
        }
    }
}

const DBOVariableSet& JSONObjectParser::variableList() const { return var_list_; }

bool JSONObjectParser::overrideDataSource() const { return override_data_source_; }

void JSONObjectParser::overrideDataSource(bool override)
{
    loginf << "JSONObjectParser: overrideDataSource: " << override;
    override_data_source_ = override;
}

std::string JSONObjectParser::dataSourceVariableName() const { return data_source_variable_name_; }

void JSONObjectParser::dataSourceVariableName(const std::string& name)
{
    loginf << "JSONObjectParser: dataSourceVariableName: " << name;

    data_source_variable_name_ = name;
}

JSONObjectParserWidget* JSONObjectParser::widget()
{
    if (!widget_)
    {
        widget_.reset(new JSONObjectParserWidget(*this));
        assert(widget_);
    }

    return widget_.get();  // needed for qt integration, not pretty
}

std::string JSONObjectParser::dbObjectName() const { return db_object_name_; }

void JSONObjectParser::setMappingActive(JSONDataMapping& mapping, bool active)
{
    if (!mapping.initialized())
        mapping.initializeIfRequired();

    if (active)
    {
        list_.addProperty(mapping.variable().name(), mapping.variable().dataType());
        var_list_.add(mapping.variable());
    }
    else if (mapping.hasVariable())  // remove if was added
    {
        if (list_.hasProperty(mapping.variable().name()))
            list_.removeProperty(mapping.variable().name());
        if (var_list_.hasVariable(mapping.variable()))
            var_list_.removeVariable(mapping.variable());
    }

    mapping.active(active);
}

void JSONObjectParser::updateMappings()
{
    if (widget_)
        widget_->updateMappingsGrid();
}

std::string JSONObjectParser::name() const { return name_; }

void JSONObjectParser::name(const std::string& name) { name_ = name; }
