#include "jsonmapping.h"

#include "dbovariable.h"
#include "buffer.h"
#include "dbobject.h"

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
    assert (mapping.variable_.hasCurrentDBColumn());

    data_mappings_.push_back(mapping);
    list_.addProperty(mapping.variable_.name(), mapping.variable_.dataType());

    if (mapping.variable_.isKey())
    {
        assert (mapping.variable_.dataType() == PropertyDataType::INT);
        has_key_mapping_ = true;
    }
}

std::shared_ptr<Buffer> JsonMapping::buffer() const
{
    return buffer_;
}

void JsonMapping::clearBuffer ()
{
    buffer_ = nullptr;
}

unsigned int JsonMapping::parseJSON (nlohmann::json& j, bool test)
{
    DBOVariable* key_var {nullptr};

    if (key_count_ == 0 && !has_key_mapping_ && db_object_.hasKeyVariable()) // first time only, add key variable
        has_key_variable_ = true;
    else
        has_key_variable_ = has_key_mapping_; // couldn't be added, can only have if mapped one exists

    if (has_key_variable_)
    {
        key_var = &db_object_.getKeyVariable();
        assert (key_var);
        assert (key_var->dataType() == PropertyDataType::INT);
        list_.addProperty(key_var->name(), key_var->dataType());
    }

    if (override_key_variable_ && !has_key_variable_)
    {
        logwrn << "JsonMapping: parseJSON: override key set but no key variable exists, disabling override";
        override_key_variable_ = false;
    }

    assert (buffer_ == nullptr);
    buffer_ = std::shared_ptr<Buffer> (new Buffer (list_, db_object_.name()));

    unsigned int row_cnt = 0;
    unsigned int skipped_cnt = 0;
    unsigned int all_cnt = 0;

    bool skip_this;

    for (json::iterator it = j.begin(); it != j.end(); ++it)
    {
        loginf << it.key(); // << ':' << it->asInt() << '\n';

        if (it.key() == json_container_key_)
        {
            json& ac_list = it.value();
            assert (ac_list.is_array());

            //Json::Value& ac_list = it.v;
            loginf << "parsing target reports";
            for (auto tr_it = ac_list.begin(); tr_it != ac_list.end(); ++tr_it)
            {
                loginf << "new target report";

                json& tr = tr_it.value();
                assert (tr.is_object());

                if (json_key_ != "*" && json_value_ != "*")
                {
                    loginf << "skipping because of key";
                    skip_this = true;

                    if (tr.find (json_key_) == tr.end() || tr[json_key_] != json_value_)
                        continue;
                }

                skip_this = false;

                // check if all required data exists and is not null
                for (auto& data_it : data_mappings_)
                {
                    if (data_it.mandatory_ &&
                            (tr.find (data_it.json_key_) == tr.end() || tr[data_it.json_key_] == nullptr))
                    {
                        loginf << "skipping because of lack of data, "
                               << data_it.json_key_ << " not found "
                               << (tr.find (data_it.json_key_) == tr.end()) << " null "
                               << (tr[data_it.json_key_] == nullptr);
                        skip_this = true;
                        break;
                    }
                }

                if (!skip_this)
                {
                    loginf << "not skipping";

                    PropertyDataType data_type;
                    std::string current_var_name;
                    bool null_value;

                    for (auto& data_it : data_mappings_)
                    {
                        if (tr.find (data_it.json_key_) != tr.end())
                        {
                            data_type = data_it.variable_.dataType();
                            current_var_name = data_it.variable_.name();
                            null_value = tr[data_it.json_key_] == nullptr;

                            switch (data_type)
                            {
                            case PropertyDataType::BOOL:
                            {
                                loginf << "bool " << current_var_name << " json " << tr[data_it.json_key_]
                                       << " format '" << data_it.json_value_format_ << "'";
                                assert (buffer_->has<bool>(current_var_name));
                                ArrayListTemplate<bool> &array_list = buffer_->get<bool> (current_var_name);

                                if (null_value)
                                    array_list.setNone(row_cnt);
                                else
                                {
                                    try
                                    {
                                        if (data_it.json_value_format_ == "")
                                            array_list.set(row_cnt, tr[data_it.json_key_]);
                                        else
                                            array_list.setFromFormat(row_cnt, data_it.json_value_format_,
                                                                     toString(tr[data_it.json_key_]));

                                        loginf << "bool " << current_var_name << " json " << tr[data_it.json_key_]
                                               << " buffer " << array_list.get(row_cnt);
                                    }
                                    catch (json::exception& e)
                                    {
                                        logerr  <<  "JsonMapping: parseJSON: json exception " << e.what();
                                        array_list.setNone(row_cnt);
                                    }
                                }

                                //array_list.set(row_cnt, static_cast<unsigned int> (tr[data_it.json_key_]));

                                break;
                            }
                            case PropertyDataType::CHAR:
                            {
                                loginf << "char " << current_var_name << " json " << tr[data_it.json_key_]
                                       << " format '" << data_it.json_value_format_ << "'";
                                assert (buffer_->has<char>(current_var_name));
                                ArrayListTemplate<char> &array_list = buffer_->get<char> (current_var_name);
                                //array_list.convertToStandardFormat(column.dataFormat());

                                if (null_value)
                                    array_list.setNone(row_cnt);
                                else
                                {
                                    try
                                    {
                                        if (data_it.json_value_format_ == "")
                                            array_list.set(row_cnt, static_cast<unsigned int> (tr[data_it.json_key_]));
                                        else
                                            array_list.setFromFormat(row_cnt, data_it.json_value_format_,
                                                                     toString(tr[data_it.json_key_]));

                                        loginf << "char " << current_var_name << " json " << tr[data_it.json_key_]
                                               << " buffer " << array_list.get(row_cnt);
                                    }
                                    catch (json::exception& e)
                                    {
                                        logerr  <<  "JsonMapping: parseJSON: json exception " << e.what();
                                        array_list.setNone(row_cnt);
                                    }
                                }

                                //array_list.set(row_cnt, static_cast<unsigned int> (tr[data_it.json_key_]));

                                break;
                            }
                            case PropertyDataType::UCHAR:
                            {
                                loginf << "uchar " << current_var_name << " json " << tr[data_it.json_key_]
                                       << " format '" << data_it.json_value_format_ << "'";
                                assert (buffer_->has<unsigned char>(current_var_name));
                                ArrayListTemplate<unsigned char> &array_list =
                                        buffer_->get<unsigned char> (current_var_name);

                                if (null_value)
                                    array_list.setNone(row_cnt);
                                else
                                {
                                    try
                                    {
                                        if (data_it.json_value_format_ == "")
                                            array_list.set(row_cnt, tr[data_it.json_key_]);
                                        else
                                            array_list.setFromFormat(row_cnt, data_it.json_value_format_,
                                                                     toString(tr[data_it.json_key_]));

                                        loginf << "uchar " << current_var_name << " json " << tr[data_it.json_key_]
                                               << " buffer " << array_list.get(row_cnt);
                                    }
                                    catch (json::exception& e)
                                    {
                                        logerr  <<  "JsonMapping: parseJSON: json exception " << e.what();
                                        array_list.setNone(row_cnt);
                                    }
                                }

                                break;
                            }
                            case PropertyDataType::INT:
                            {
                                loginf << "int " << current_var_name << " json " << tr[data_it.json_key_]
                                       << " format '" << data_it.json_value_format_ << "'";
                                assert (buffer_->has<int>(current_var_name));
                                ArrayListTemplate<int> &array_list =
                                        buffer_->get<int> (current_var_name);

                                if (null_value)
                                    array_list.setNone(row_cnt);
                                else
                                {
                                    try
                                    {
                                        if (data_it.json_value_format_ == "")
                                            array_list.set(row_cnt, tr[data_it.json_key_]);
                                        else
                                            array_list.setFromFormat(row_cnt, data_it.json_value_format_,
                                                                     toString(tr[data_it.json_key_]));

                                        loginf << "int " << current_var_name << " json " << tr[data_it.json_key_]
                                               << " buffer " << array_list.get(row_cnt);
                                    }
                                    catch (json::exception& e)
                                    {
                                        logerr  <<  "JsonMapping: parseJSON: json exception " << e.what();
                                        array_list.setNone(row_cnt);
                                    }
                                }

                                break;
                            }
                            case PropertyDataType::UINT:
                            {
                                loginf << "uint " << current_var_name << " json " << tr[data_it.json_key_]
                                       << " format '" << data_it.json_value_format_ << "'";
                                assert (buffer_->has<unsigned int>(current_var_name));
                                ArrayListTemplate<unsigned int> &array_list =
                                        buffer_->get<unsigned int> (current_var_name);

                                if (null_value)
                                    array_list.setNone(row_cnt);
                                else
                                {
                                    try
                                    {
                                        if (data_it.json_value_format_ == "")
                                            array_list.set(row_cnt, tr[data_it.json_key_]);
                                        else
                                            array_list.setFromFormat(row_cnt, data_it.json_value_format_,
                                                                     toString(tr[data_it.json_key_]));

                                        loginf << "uint " << current_var_name << " json " << tr[data_it.json_key_]
                                               << " buffer " << array_list.get(row_cnt);
                                    }
                                    catch (json::exception& e)
                                    {
                                        logerr  <<  "JsonMapping: parseJSON: json exception " << e.what();
                                        array_list.setNone(row_cnt);
                                    }
                                }

                                break;
                            }
                            case PropertyDataType::LONGINT:
                            {
                                loginf << "long " << current_var_name << " json " << tr[data_it.json_key_]
                                       << " format '" << data_it.json_value_format_ << "'";
                                assert (buffer_->has<long int>(current_var_name));
                                ArrayListTemplate<long int> &array_list =
                                        buffer_->get<long int>(current_var_name);

                                if (null_value)
                                    array_list.setNone(row_cnt);
                                else
                                {
                                    try
                                    {
                                        if (data_it.json_value_format_ == "")
                                            array_list.set(row_cnt, tr[data_it.json_key_]);
                                        else
                                            array_list.setFromFormat(row_cnt, data_it.json_value_format_,
                                                                     toString(tr[data_it.json_key_]));

                                        loginf << "long " << current_var_name << " json " << tr[data_it.json_key_]
                                               << " buffer " << array_list.get(row_cnt);
                                    }
                                    catch (json::exception& e)
                                    {
                                        logerr  <<  "JsonMapping: parseJSON: json exception " << e.what();
                                        array_list.setNone(row_cnt);
                                    }
                                }

                                break;
                            }
                            case PropertyDataType::ULONGINT:
                            {
                                loginf << "ulong " << current_var_name << " json " << tr[data_it.json_key_]
                                       << " format '" << data_it.json_value_format_ << "'";
                                assert (buffer_->has<unsigned long>(current_var_name));
                                ArrayListTemplate<unsigned long> &array_list =
                                        buffer_->get<unsigned long>(current_var_name);

                                if (null_value)
                                    array_list.setNone(row_cnt);
                                else
                                {
                                    try
                                    {
                                        if (data_it.json_value_format_ == "")
                                            array_list.set(row_cnt, tr[data_it.json_key_]);
                                        else
                                            array_list.setFromFormat(row_cnt, data_it.json_value_format_,
                                                                     toString(tr[data_it.json_key_]));

                                        loginf << "ulong " << current_var_name << " json " << tr[data_it.json_key_]
                                               << " buffer " << array_list.get(row_cnt);
                                    }
                                    catch (json::exception& e)
                                    {
                                        logerr  <<  "JsonMapping: parseJSON: json exception " << e.what();
                                        array_list.setNone(row_cnt);
                                    }
                                }
                                break;
                            }
                            case PropertyDataType::FLOAT:
                            {
                                loginf << "float " << current_var_name << " json " << tr[data_it.json_key_]
                                       << " format '" << data_it.json_value_format_ << "'";
                                assert (buffer_->has<float>(current_var_name));
                                ArrayListTemplate<float> &array_list =
                                        buffer_->get<float>(current_var_name);

                                if (null_value)
                                    array_list.setNone(row_cnt);
                                else
                                {
                                    try
                                    {
                                        if (data_it.json_value_format_ == "")
                                            array_list.set(row_cnt, tr[data_it.json_key_]);
                                        else
                                            array_list.setFromFormat(row_cnt, data_it.json_value_format_,
                                                                     toString(tr[data_it.json_key_]));

                                        loginf << "float " << current_var_name << " json " << tr[data_it.json_key_]
                                               << " buffer " << array_list.get(row_cnt);
                                    }
                                    catch (json::exception& e)
                                    {
                                        logerr  <<  "JsonMapping: parseJSON: json exception " << e.what();
                                        array_list.setNone(row_cnt);
                                    }
                                }
                                break;
                            }
                            case PropertyDataType::DOUBLE:
                            {
                                loginf << "double " << current_var_name << " json " << tr[data_it.json_key_]
                                       << " format '" << data_it.json_value_format_ << "'";
                                assert (buffer_->has<double>(current_var_name));
                                ArrayListTemplate<double> &array_list =
                                        buffer_->get<double>(current_var_name);

                                if (null_value)
                                    array_list.setNone(row_cnt);
                                else
                                {
                                    try
                                    {
                                        if (data_it.json_value_format_ == "")
                                            array_list.set(row_cnt, tr[data_it.json_key_]);
                                        else
                                            array_list.setFromFormat(row_cnt, data_it.json_value_format_,
                                                                     toString(tr[data_it.json_key_]));

                                        loginf << "double " << current_var_name << " json " << tr[data_it.json_key_]
                                               << " buffer " << array_list.get(row_cnt);
                                    }
                                    catch (json::exception& e)
                                    {
                                        logerr  <<  "JsonMapping: parseJSON: json exception " << e.what();
                                        array_list.setNone(row_cnt);
                                    }
                                }

                                break;
                            }
                            case PropertyDataType::STRING:
                            {
                                loginf << "string " << current_var_name << " json " << tr[data_it.json_key_]
                                       << " format '" << data_it.json_value_format_ << "'";
                                assert (buffer_->has<std::string>(current_var_name));
                                ArrayListTemplate<std::string> &array_list =
                                        buffer_->get<std::string>(current_var_name);

                                if (null_value)
                                    array_list.setNone(row_cnt);
                                else
                                {
                                    try
                                    {
                                        assert (data_it.json_value_format_ == "");
                                        array_list.set(row_cnt, tr[data_it.json_key_]);
                                        loginf << "string " << current_var_name << " json " << tr[data_it.json_key_]
                                               << " buffer " << array_list.get(row_cnt);
                                    }
                                    catch (json::exception& e)
                                    {
                                        logerr  <<  "JsonMapping: parseJSON: json exception " << e.what();
                                        array_list.setNone(row_cnt);
                                    }
                                }

                                break;
                            }
                            default:
                                logerr  <<  "JsonMapping: parseJSON: impossible for property type "
                                         << Property::asString(data_type);
                                throw std::runtime_error ("JsonMapping: parseJSON: impossible property type "
                                                          + Property::asString(data_type));
                            }
                        }
                    }

                    if (override_key_variable_)
                    {
                        assert (key_var);
                        assert (buffer_->has<int>(key_var->name()));
                        ArrayListTemplate<int> &array_list = buffer_->get<int> (key_var->name());
                        array_list.set(row_cnt, key_count_);
                        loginf << "override key " << array_list.get(row_cnt);
                    }

                    //                    logdbg << "\t rn " << rec_num_cnt_
                    //                           << " rc " << receiver
                    //                           << " ta " << target_address
                    //                           << " cs " << (callsign_valid ? callsign : "NULL")
                    //                           << " alt_baro " << (altitude_baro_valid ? std::to_string(altitude_baro_ft) : "NULL")
                    //                           << " alt_geo " << (altitude_geo_valid ? std::to_string(altitude_geo_ft) : "NULL")
                    //                           << " lat " << std::to_string(latitude_deg)
                    //                           << " lon " << std::to_string(longitude_deg)
                    //                           << " dt " << date_time.toString("yyyy.MM.dd hh:mm:ss.zzz").toStdString();

                    //                    key_al.set(row_cnt, rec_num_cnt_);
                    //                    dsid_al.set(row_cnt, receiver);
                    //                    target_addr_al.set(row_cnt, target_address);

                    //                    if (callsign_valid)
                    //                        callsign_al.set(row_cnt, callsign);
                    //                    else
                    //                        callsign_al.setNone(row_cnt);

                    //                    if (altitude_baro_valid)
                    //                        altitude_baro_al.set(row_cnt, altitude_baro_ft);
                    //                    else
                    //                        altitude_baro_al.setNone(row_cnt);

                    //                    if (altitude_geo_valid)
                    //                        altitude_geo_al.set(row_cnt, altitude_geo_ft);
                    //                    else
                    //                        altitude_geo_al.setNone(row_cnt);

                    //                    latitude_al.set(row_cnt, latitude_deg);
                    //                    longitude_al.set(row_cnt, longitude_deg);
                    //                    tod_al.set(row_cnt, (int) tod);

                    //                    row_cnt++;
                    //                    rec_num_cnt_++;
                    //                    inserted_cnt_++;

                    //                    if (datasources_existing_.count(receiver) == 0 && datasources_to_add_.count(receiver) == 0)
                    //                        datasources_to_add_[receiver] = receiver_name;

                    row_cnt++;
                    key_count_++;
                }
                else
                {
                    loginf << "skipping";
                    skipped_cnt++;
                }

                all_cnt++;
            }
        }
    }

    return row_cnt;
}

bool JsonMapping::overrideKeyVariable() const
{
    return override_key_variable_;
}

void JsonMapping::overrideKeyVariable(bool override)
{
    override_key_variable_ = override;
}

