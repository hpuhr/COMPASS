#include "jsonmapping.h"

#include "dbovariable.h"
#include "buffer.h"
#include "dbobject.h"
#include "unitmanager.h"
#include "unit.h"

using namespace nlohmann;

JsonMapping::JsonMapping (DBObject& db_object)
    : db_object_(db_object)
{

}

DBObject &JsonMapping::dbObject() const
{
    return db_object_;
}

std::string JsonMapping::JSONKey() const
{
    return json_key_;
}

void JsonMapping::JSONKey(const std::string& json_key)
{
    json_key_ = json_key;
}

std::string JsonMapping::JSONValue() const
{
    return json_value_;
}

void JsonMapping::JSONValue(const std::string& json_value)
{
    json_value_ = json_value;
}

std::string JsonMapping::JSONContainerKey() const
{
    return json_container_key_;
}

void JsonMapping::JSONContainerKey(const std::string& key)
{
    json_container_key_ = key;
}
void JsonMapping::addMapping (JsonKey2DBOVariableMapping mapping)
{
    assert (mapping.variable().hasCurrentDBColumn());

    data_mappings_.push_back(mapping);
    list_.addProperty(mapping.variable().name(), mapping.variable().dataType());
    var_list_.add(mapping.variable());

    if (mapping.variable().isKey())
    {
        assert (mapping.variable().dataType() == PropertyDataType::INT);
        has_key_mapping_ = true;
    }
}

bool JsonMapping::hasFilledBuffer ()
{
    if (!buffer_)
        return false;
    return buffer_->size() > 0;
}

std::shared_ptr<Buffer> JsonMapping::buffer()
{
    return buffer_;
}

void JsonMapping::clearBuffer ()
{
    buffer_ = nullptr;
}

void JsonMapping::initializeKey ()
{
    if (!key_initialized_)
    {
        if (!has_key_mapping_ && db_object_.hasKeyVariable()) // first time only, add key variable
            has_key_variable_ = true;
        else
            has_key_variable_ = has_key_mapping_; // couldn't be added, can only have if mapped one exists

        if (has_key_variable_ && key_variable_ == nullptr)
        {
            key_variable_ = &db_object_.getKeyVariable();
            assert (key_variable_);
            assert (key_variable_->dataType() == PropertyDataType::INT);
            if (!list_.hasProperty(key_variable_->name()))
            {
                list_.addProperty(key_variable_->name(), key_variable_->dataType());
                var_list_.add(*key_variable_);
            }
        }

        if (override_key_variable_ && !has_key_variable_)
        {
            logwrn << "JsonMapping: parseJSON: override key set but no key variable exists, disabling override";
            override_key_variable_ = false;
        }

        not_parse_all_ = (json_key_ != "*") && (json_value_ != "*");

        key_initialized_ = true;
    }
}

unsigned int JsonMapping::parseJSON (nlohmann::json& j)
{
    if (!key_initialized_)
        initializeKey ();

    //assert (buffer_ == nullptr || buffer_->size() == 0);
    if (buffer_ == nullptr)
        buffer_ = std::shared_ptr<Buffer> (new Buffer (list_, db_object_.name()));

    unsigned int row_cnt = buffer_->size();
    unsigned int skipped_cnt = 0;
    unsigned int all_cnt = 0;

    bool skipped;

    if (json_container_key_.size())
    {
        if (j.find(json_container_key_) != j.end())
        {
            json& ac_list = j.at(json_container_key_);
            assert (ac_list.is_array());

            //loginf << "found target report array in '" << json_container_key_  << "', parsing";

            for (auto tr_it = ac_list.begin(); tr_it != ac_list.end(); ++tr_it)
            {
                //logdbg << "new target report";

                json& tr = tr_it.value();
                assert (tr.is_object());

                skipped = parseTargetReport (tr, row_cnt);

                if (!skipped)
                {
                    if (override_key_variable_)
                    {
                        assert (key_variable_);
                        assert (buffer_->has<int>(key_variable_->name()));
                        buffer_->get<int> (key_variable_->name()).set(row_cnt, key_count_);
                        //loginf << "override key row " << row_cnt << " val " << key_count_;
                    }

                    ++row_cnt;
                    ++key_count_;
                }
                else
                    ++skipped_cnt;

                ++all_cnt;
            }
        }
        else
            loginf << "found target report array '" << json_container_key_  << "' not found";
    }
    else
    {
        //loginf << "found single target report";
        assert (j.is_object());

        skipped = parseTargetReport (j, row_cnt);

        if (!skipped)
        {
            if (override_key_variable_)
            {
                assert (key_variable_);
                assert (buffer_->has<int>(key_variable_->name()));
                buffer_->get<int> (key_variable_->name()).set(row_cnt, key_count_);
                //loginf << "override key row " << row_cnt << " val " << key_count_;
            }

            ++row_cnt;
            ++key_count_;
        }

    }

    return row_cnt;
}

bool JsonMapping::parseTargetReport (const nlohmann::json& tr, size_t row_cnt)
{
    // check key match
    if (not_parse_all_)
    {
        if (tr.find (json_key_) != tr.end())
        {
            if (tr.at(json_key_) != json_value_)
            {
                logdbg << "JsonMapping: parseTargetReport: skipping because of wrong key value " << tr.at(json_key_);
                return true;
            }
            else
                logdbg << "JsonMapping: parseTargetReport: parsing with correct key and value";
        }
        else
        {
            logdbg << "JsonMapping: parseTargetReport: skipping because of missing key '" << json_key_ << "'";
            return true;
        }
    }

    // check if all required data exists and is not null
    for (auto& data_it : data_mappings_)
    {
        logdbg << "checking data mapping key " << data_it.jsonKey();

        if (data_it.mandatory() && (!data_it.hasKey(tr) || data_it.isNull(tr)))
        {
            logdbg << "skipping because of lack of data, " << data_it.jsonKey()
                   << " not found " << !data_it.hasKey(tr)
                   << " null " << data_it.isNull(tr);
            return true;
        }
    }

    //loginf << "not skipping";

    PropertyDataType data_type;
    std::string current_var_name;

    for (auto& data_it : data_mappings_)
    {
        //logdbg << "setting data mapping key " << data_it.jsonKey();

        const nlohmann::json& tr_val = data_it.getValue(tr);

        if (tr_val != nullptr)
        {
            data_type = data_it.variable().dataType();
            current_var_name = data_it.variable().name();

            switch (data_type)
            {
            case PropertyDataType::BOOL:
            {
                logdbg << "bool " << current_var_name << " json " << tr_val
                       << " format '" << data_it.jsonValueFormat() << "'";
                assert (buffer_->has<bool>(current_var_name));
                data_it.setValue (buffer_->get<bool> (current_var_name), row_cnt, tr_val);

                break;
            }
            case PropertyDataType::CHAR:
            {
                logdbg << "char " << current_var_name << " json " << tr_val
                       << " format '" << data_it.jsonValueFormat() << "'";
                assert (buffer_->has<char>(current_var_name));
                data_it.setValue (buffer_->get<char> (current_var_name), row_cnt, tr_val);

                break;
            }
            case PropertyDataType::UCHAR:
            {
                logdbg << "uchar " << current_var_name << " json " << tr_val
                       << " format '" << data_it.jsonValueFormat() << "'";
                assert (buffer_->has<unsigned char>(current_var_name));
                data_it.setValue (buffer_->get<unsigned char> (current_var_name), row_cnt, tr_val);

                break;
            }
            case PropertyDataType::INT:
            {
                logdbg << "int " << current_var_name << " json " << tr_val
                       << " format '" << data_it.jsonValueFormat() << "'";
                assert (buffer_->has<int>(current_var_name));
                data_it.setValue (buffer_->get<int> (current_var_name), row_cnt, tr_val);

                break;
            }
            case PropertyDataType::UINT:
            {
                logdbg << "uint " << current_var_name << " json " << tr_val
                       << " format '" << data_it.jsonValueFormat() << "'";
                assert (buffer_->has<unsigned int>(current_var_name));
                data_it.setValue (buffer_->get<unsigned int> (current_var_name), row_cnt, tr_val);

                break;
            }
            case PropertyDataType::LONGINT:
            {
                logdbg << "long " << current_var_name << " json " << tr_val
                       << " format '" << data_it.jsonValueFormat() << "'";
                assert (buffer_->has<long int>(current_var_name));
                data_it.setValue (buffer_->get<long int> (current_var_name), row_cnt, tr_val);

                break;
            }
            case PropertyDataType::ULONGINT:
            {
                logdbg << "ulong " << current_var_name << " json " << tr_val
                       << " format '" << data_it.jsonValueFormat() << "'";
                assert (buffer_->has<unsigned long>(current_var_name));
                data_it.setValue (buffer_->get<unsigned long> (current_var_name), row_cnt, tr_val);

                break;
            }
            case PropertyDataType::FLOAT:
            {
                logdbg << "float " << current_var_name << " json " << tr_val
                       << " format '" << data_it.jsonValueFormat() << "'";
                assert (buffer_->has<float>(current_var_name));
                data_it.setValue (buffer_->get<float> (current_var_name), row_cnt, tr_val);
                break;
            }
            case PropertyDataType::DOUBLE:
            {
                logdbg << "double " << current_var_name << " json " << tr_val
                       << " format '" << data_it.jsonValueFormat() << "'";
                assert (buffer_->has<double>(current_var_name));
                data_it.setValue (buffer_->get<double> (current_var_name), row_cnt, tr_val);

                break;
            }
            case PropertyDataType::STRING:
            {
                logdbg << "string " << current_var_name << " json " << tr_val
                       << " format '" << data_it.jsonValueFormat() << "'";
                assert (buffer_->has<std::string>(current_var_name));
                data_it.setValue (buffer_->get<std::string> (current_var_name), row_cnt, tr_val);

                break;
            }
            default:
                logerr  <<  "JsonMapping: parseTargetReport: impossible for property type "
                         << Property::asString(data_type);
                throw std::runtime_error ("JsonMapping: parseTargetReport: impossible property type "
                                          + Property::asString(data_type));
            }
        }
        else
        {
            logdbg  <<  "JsonMapping: parseTargetReport: key " << data_it.jsonKey()<< " not found, setting 0";

            data_type = data_it.variable().dataType();
            current_var_name = data_it.variable().name();

            switch (data_type)
            {
            case PropertyDataType::BOOL:
            {
                assert (buffer_->has<bool>(current_var_name));
                buffer_->get<bool> (current_var_name).setNone(row_cnt);
                break;
            }
            case PropertyDataType::CHAR:
            {
                assert (buffer_->has<char>(current_var_name));
                buffer_->get<char> (current_var_name).setNone(row_cnt);
                break;
            }
            case PropertyDataType::UCHAR:
            {
                assert (buffer_->has<unsigned char>(current_var_name));
                buffer_->get<unsigned char> (current_var_name).setNone(row_cnt);
                break;
            }
            case PropertyDataType::INT:
            {
                assert (buffer_->has<int>(current_var_name));
                buffer_->get<int> (current_var_name).setNone(row_cnt);
                break;
            }
            case PropertyDataType::UINT:
            {
                assert (buffer_->has<unsigned int>(current_var_name));
                buffer_->get<unsigned int> (current_var_name).setNone(row_cnt);
                break;
            }
            case PropertyDataType::LONGINT:
            {
                assert (buffer_->has<long int>(current_var_name));
                buffer_->get<long int> (current_var_name).setNone(row_cnt);
                break;
            }
            case PropertyDataType::ULONGINT:
            {
                assert (buffer_->has<unsigned long>(current_var_name));
                buffer_->get<unsigned long> (current_var_name).setNone(row_cnt);
                break;
            }
            case PropertyDataType::FLOAT:
            {
                assert (buffer_->has<float>(current_var_name));
                buffer_->get<float> (current_var_name).setNone(row_cnt);
                break;
            }
            case PropertyDataType::DOUBLE:
            {
                assert (buffer_->has<double>(current_var_name));
                buffer_->get<double> (current_var_name).setNone(row_cnt);
                break;
            }
            case PropertyDataType::STRING:
            {
                assert (buffer_->has<std::string>(current_var_name));
                buffer_->get<std::string> (current_var_name).setNone(row_cnt);
                break;
            }
            default:
                logerr  <<  "JsonMapping: parseTargetReport: set null impossible for property type "
                         << Property::asString(data_type);
                throw std::runtime_error ("JsonMapping: parseTargetReport: set null impossible property type "
                                          + Property::asString(data_type));
            }
        }
    }
    return false;
}

void JsonMapping::transformBuffer ()
{
    logdbg << "JsonMapping: transformBuffer: object " << db_object_.name();

    for (auto& data_it : data_mappings_)
    {
        if (data_it.dimension() != data_it.variable().dimension())
            logwrn << "JsonMapping: transformBuffer: variable " << data_it.variable().name()
                   << " has differing dimensions " << data_it.dimension() << " " << data_it.variable().dimension();
        else if (data_it.unit() != data_it.variable().unit()) // do unit conversion stuff
        {
            logdbg << "JsonMapping: transformBuffer: variable " << data_it.variable().name()
                   << " of same dimension has different units " << data_it.unit() << " " << data_it.variable().unit();

            const Dimension &dimension = UnitManager::instance().dimension (data_it.variable().dimension());
            double factor;

            factor = dimension.getFactor (data_it.unit(), data_it.variable().unit());
            std::string current_var_name = data_it.variable().name();
            PropertyDataType data_type = data_it.variable().dataType();

            logdbg  << "JsonMapping: transformBuffer: correct unit transformation with factor " << factor;

            switch (data_type)
            {
            case PropertyDataType::BOOL:
            {
                assert (buffer_->has<bool>(current_var_name));
                ArrayListTemplate<bool> &array_list = buffer_->get<bool>(current_var_name);
                logwrn << "JsonMapping: transformBuffer: double multiplication of boolean variable "
                       << current_var_name;
                array_list *= factor;
                break;
            }
            case PropertyDataType::CHAR:
            {
                assert (buffer_->has<char>(current_var_name));
                ArrayListTemplate<char> &array_list = buffer_->get<char> (current_var_name);
                logwrn << "JsonMapping: transformBuffer: double multiplication of char variable "
                       << current_var_name;
                array_list *= factor;
                break;
            }
            case PropertyDataType::UCHAR:
            {
                assert (buffer_->has<unsigned char>(current_var_name));
                ArrayListTemplate<unsigned char> &array_list = buffer_->get<unsigned char> (current_var_name);
                logwrn << "JsonMapping: transformBuffer: double multiplication of unsigned char variable "
                       << current_var_name;
                array_list *= factor;
                break;
            }
            case PropertyDataType::INT:
            {
                assert (buffer_->has<int>(current_var_name));
                ArrayListTemplate<int> &array_list = buffer_->get<int> (current_var_name);
                array_list *= factor;
                break;
            }
            case PropertyDataType::UINT:
            {
                assert (buffer_->has<unsigned int>(current_var_name));
                ArrayListTemplate<unsigned int> &array_list = buffer_->get<unsigned int> (current_var_name);
                array_list *= factor;
                break;
            }
            case PropertyDataType::LONGINT:
            {
                assert (buffer_->has<long int>(current_var_name));
                ArrayListTemplate<long int> &array_list = buffer_->get<long int>(current_var_name);
                array_list *= factor;
                break;
            }
            case PropertyDataType::ULONGINT:
            {
                assert (buffer_->has<unsigned long>(current_var_name));
                ArrayListTemplate<unsigned long> &array_list = buffer_->get<unsigned long>(current_var_name);
                array_list *= factor;
                break;
            }
            case PropertyDataType::FLOAT:
            {
                assert (buffer_->has<float>(current_var_name));
                ArrayListTemplate<float> &array_list = buffer_->get<float>(current_var_name);
                array_list *= factor;
                break;
            }
            case PropertyDataType::DOUBLE:
            {
                assert (buffer_->has<double>(current_var_name));
                ArrayListTemplate<double> &array_list = buffer_->get<double>(current_var_name);
                array_list *= factor;
                break;
            }
            case PropertyDataType::STRING:
                logerr << "JsonMapping: transformBuffer: unit transformation for string variable "
                       << data_it.variable().name() << " impossible";
                break;
            default:
                logerr  <<  "JsonMapping: transformBuffer: unknown property type "
                         << Property::asString(data_type);
                throw std::runtime_error ("JsonMapping: transformBuffer: unknown property type "
                                          + Property::asString(data_type));
            }
        }
    }

}


bool JsonMapping::overrideKeyVariable() const
{
    return override_key_variable_;
}

void JsonMapping::overrideKeyVariable(bool override)
{
    override_key_variable_ = override;
}

DBOVariableSet& JsonMapping::variableList()
{
    return var_list_;
}

bool JsonMapping::overrideDataSource() const
{
    return override_data_source_;
}

void JsonMapping::OverrideDataSource(bool override)
{
    override_data_source_ = override;
}

std::string JsonMapping::dataSourceVariableName() const
{
    return data_source_variable_name_;
}

void JsonMapping::dataSourceVariableName(const std::string& name)
{
    data_source_variable_name_ = name;
}

unsigned int JsonMapping::keyCount() const
{
    return key_count_;
}

void JsonMapping::keyCount(unsigned int key_count)
{
    key_count_ = key_count;
}


DBOVariable &JsonKey2DBOVariableMapping::variable() const
{
    return variable_;
}

//void JsonKey2DBOVariableMapping::variable(const DBOVariable &variable)
//{
//    variable_ = variable;
//}

bool JsonKey2DBOVariableMapping::mandatory() const
{
    return mandatory_;
}

void JsonKey2DBOVariableMapping::mandatory(bool mandatory)
{
    mandatory_ = mandatory;
}

Format JsonKey2DBOVariableMapping::jsonValueFormat() const
{
    return json_value_format_;
}

void JsonKey2DBOVariableMapping::jsonValueFormat(const Format &json_value_format)
{
    json_value_format_ = json_value_format;
}

std::string JsonKey2DBOVariableMapping::jsonKey() const
{
    return json_key_;
}

void JsonKey2DBOVariableMapping::jsonKey(const std::string &json_key)
{
    json_key_ = json_key;
}

