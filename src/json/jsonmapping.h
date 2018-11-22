#ifndef JSONMAPPING_H
#define JSONMAPPING_H

#include <string>
#include <memory>

#include "propertylist.h"
#include "json.hpp"
#include "format.h"
#include "dbovariable.h"
#include "dbovariableset.h"
#include "arraylist.h"
#include "stringconv.h"

class DBObject;
class DBOVariable;
class Buffer;

class JsonKey2DBOVariableMapping
{
public:
    JsonKey2DBOVariableMapping (const std::string& json_key, DBOVariable& variable, bool mandatory)
        : JsonKey2DBOVariableMapping(json_key, variable, mandatory, {variable.dataType(), ""}, "", "")
    {}

    JsonKey2DBOVariableMapping (const std::string& json_key, DBOVariable& variable, bool mandatory,
                                Format json_value_format)
        : JsonKey2DBOVariableMapping(json_key, variable, mandatory, json_value_format, "", "")
    {}

    JsonKey2DBOVariableMapping (const std::string& json_key, DBOVariable& variable, bool mandatory,
                                const std::string& dimension, const std::string& unit)
        : JsonKey2DBOVariableMapping(json_key, variable, mandatory, {variable.dataType(), ""}, dimension, unit)
    {}

    JsonKey2DBOVariableMapping (const std::string& json_key, DBOVariable& variable, bool mandatory,
                                Format json_value_format, const std::string& dimension, const std::string& unit)
        : json_key_(json_key), variable_(variable), mandatory_(mandatory), json_value_format_(json_value_format),
          dimension_(dimension), unit_(unit)
    {
        sub_keys_ = Utils::String::split(json_key_, '.');
        has_sub_keys_ = sub_keys_.size() > 1;
        num_sub_keys_ = sub_keys_.size();

        logdbg << "JsonKey2DBOVariableMapping: ctor: key " << json_key_ << " num subkeys " << sub_keys_.size();

    }

    bool hasKey (const nlohmann::json& j)
    {
        if (has_sub_keys_)
        {
            const nlohmann::json* k = &j;
            std::string sub_key;

            for (unsigned int cnt=0; cnt < num_sub_keys_; ++cnt)
            {
                sub_key = sub_keys_.at(cnt);

                if (k->find (sub_key) != k->end())
                {
                    if (cnt == num_sub_keys_-1)
                        break;

                    if (k->at(sub_key).is_object())
                        k = &k->at(sub_key);
                    else
                        return false;
                }
                else
                    return false;
            }
            return true;
        }
        else
            return j.find (json_key_) != j.end();
    }

    bool isNull (const nlohmann::json& j)
    {
        if (has_sub_keys_)
        {
            const nlohmann::json* k = &j;
            std::string sub_key;

            for (unsigned int cnt=0; cnt < num_sub_keys_; ++cnt)
            {
                sub_key = sub_keys_.at(cnt);

                if (k->find (sub_key) != k->end())
                {
                    if (cnt == num_sub_keys_-1)
                        return k->at(sub_key).is_null();

                    if (k->at(sub_key).is_object())
                        k = &k->at(sub_key);
                    else
                        return true;
                }
                else
                    return true;
            }
            return true;
        }
        else
        {
            if (j.find (json_key_) != j.end())
                return j.at(json_key_) == nullptr;
            else
                return true;
        }
    }
    //const nlohmann::json& getValue (const nlohmann::json& j) { return j[json_key_]; }

    //    inline nlohmann::json& getValue (nlohmann::json j)
    //    {
    //        if (has_sub_keys_)
    //        {
    //            nlohmann::json* k = &j;
    //            std::string sub_key;

    //            for (unsigned int cnt=0; cnt < num_sub_keys_; ++cnt)
    //            {
    //                sub_key = sub_keys_.at(cnt);

    //                if (k->find (sub_key) != k->end())
    //                {
    //                    if (cnt == num_sub_keys_-1)
    //                        return k->at(sub_key);

    //                    if (k->at(sub_key).is_object())
    //                        k = &k->at(sub_key);
    //                    else
    //                        return nullptr_;
    //                }
    //                else
    //                    return nullptr_;
    //            }
    //            return *k;
    //        }
    //        else
    //        {
    //            if (j.find (json_key_) != j.end())
    //                return j.at(json_key_);
    //            else
    //                return nullptr_;
    //        }
    //    }

    inline const nlohmann::json& getValue (const nlohmann::json& j, unsigned int index=0)
    {
        if (has_sub_keys_)
        {
            std::string& sub_key = sub_keys_.at(index);

            if (j.find (sub_key) != j.end())
            {
                if (index == num_sub_keys_-1)
                    return j.at(sub_key);

                if (j.at(sub_key).is_object())
                    return getValue(j.at(sub_key), index+1);
                else
                    return nullptr_;
            }
        }
        else
        {
            if (j.find (json_key_) != j.end())
                return j.at(json_key_);
            else
                return nullptr_;
        }
        return nullptr_;
    }

    template<typename T>
    void setValue(ArrayListTemplate<T>& array_list, unsigned int row_cnt, const nlohmann::json& j)
    {
        if (j == nullptr)
            array_list.setNone(row_cnt);
        else
        {
            try
            {
                if (json_value_format_ == "")
                    array_list.set(row_cnt, j);
                else
                    array_list.setFromFormat(row_cnt, json_value_format_,
                                             toString(j));

                logdbg << "JsonKey2DBOVariableMapping: setValue: json " << j
                       << " buffer " << array_list.get(row_cnt);
            }
            catch (nlohmann::json::exception& e)
            {
                logerr  <<  "JsonKey2DBOVariableMapping: setValue: key " << json_key_ << " json exception " << e.what();
                array_list.setNone(row_cnt);
            }
        }
    }

    void setValue(ArrayListTemplate<char>& array_list, unsigned int row_cnt, const nlohmann::json& j)
    {
        if (j == nullptr)
            array_list.setNone(row_cnt);
        else
        {
            try
            {
                if (json_value_format_ == "")
                    array_list.set(row_cnt, static_cast<int> (j));
                else
                    array_list.setFromFormat(row_cnt, json_value_format_,
                                             toString(j));

                logdbg << "JsonKey2DBOVariableMapping: setValue: json " << j
                       << " buffer " << array_list.get(row_cnt);
            }
            catch (nlohmann::json::exception& e)
            {
                logerr  <<  "JsonKey2DBOVariableMapping: setValue: key " << json_key_ << " json exception " << e.what();
                array_list.setNone(row_cnt);
            }
        }
    }

    // return bool mandatory missing
    template<typename T>
    bool findAndSetValue(const nlohmann::json& j, ArrayListTemplate<T>& array_list, unsigned int row_cnt)
    {
        const nlohmann::json* val_ptr = &j;

        if (has_sub_keys_)
        {
            for (std::string& sub_key : sub_keys_)
            {
                if (val_ptr->find (sub_key) != val_ptr->end())
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
            if (val_ptr->find (json_key_) != val_ptr->end())
                val_ptr = &val_ptr->at(json_key_);
            else
                val_ptr = nullptr;
        }

        if (val_ptr == nullptr || *val_ptr == nullptr)
        {
            if (mandatory_)
                return true;

            array_list.setNone(row_cnt);
            return false;
        }
        else
        {
            try
            {
                T tmp = *val_ptr;
                if (json_value_format_ == "")
                    array_list.set(row_cnt, tmp);
                else
                    array_list.setFromFormat(row_cnt, json_value_format_,
                                             toString(tmp));

                logdbg << "JsonKey2DBOVariableMapping: setValue: json " << *val_ptr
                       << " buffer " << array_list.get(row_cnt);
            }
            catch (nlohmann::json::exception& e)
            {
                logerr  <<  "JsonKey2DBOVariableMapping: setValue: key " << json_key_ << " json exception " << e.what();
                array_list.setNone(row_cnt);
            }
        }

        return false; // everything ok
    }

    bool findAndSetValue(const nlohmann::json& j, ArrayListTemplate<char>& array_list, unsigned int row_cnt)
    {
        const nlohmann::json* val_ptr = &j;

        if (has_sub_keys_)
        {
            for (std::string& sub_key : sub_keys_)
            {
                if (val_ptr->find (sub_key) != val_ptr->end())
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
            if (val_ptr->find (json_key_) != val_ptr->end())
                val_ptr = &val_ptr->at(json_key_);
            else
                val_ptr = nullptr;
        }

        if (val_ptr == nullptr || *val_ptr == nullptr)
        {
            if (mandatory_)
                return true;

            array_list.setNone(row_cnt);
            return false;
        }
        else
        {
            try
            {
                char tmp = static_cast<int> (*val_ptr);
                if (json_value_format_ == "")
                    array_list.set(row_cnt, tmp);
                else
                    array_list.setFromFormat(row_cnt, json_value_format_,
                                             toString(tmp));

                logdbg << "JsonKey2DBOVariableMapping: setValue: json " << *val_ptr
                       << " buffer " << array_list.get(row_cnt);
            }
            catch (nlohmann::json::exception& e)
            {
                logerr  <<  "JsonKey2DBOVariableMapping: setValue: key " << json_key_ << " json exception " << e.what();
                array_list.setNone(row_cnt);
            }
        }

        return false; // everything ok
    }

    bool hasDimension () const { return dimension_.size() > 0; }
    /// @brief Returns dimension contained in the column
    const std::string &dimension () const { return dimension_; }
    /// @brief Returns unit
    const std::string &unit () const { return unit_; }

private:
    std::string json_key_;
    DBOVariable& variable_;
    bool mandatory_;
    Format json_value_format_;

    /// Unit dimension
    std::string dimension_;
    /// Unit
    std::string unit_;

    bool has_sub_keys_ {false};
    std::vector<std::string> sub_keys_;
    unsigned int num_sub_keys_;

    nlohmann::json nullptr_ = nullptr;

    inline std::string toString(const nlohmann::json& j)
    {
        if (j.type() == nlohmann::json::value_t::string) {
            return j.get<std::string>();
        }

        return j.dump();
    }

public:
    std::string jsonKey() const;
    void jsonKey(const std::string &json_key);

    DBOVariable& variable() const;
    //void variable(const DBOVariable &variable);

    bool mandatory() const;
    void mandatory(bool mandatory);

    Format jsonValueFormat() const;
    void jsonValueFormat(const Format &json_value_format);
};

class JsonMapping
{
    using MappingIterator = std::vector<JsonKey2DBOVariableMapping>::iterator;
public:
    JsonMapping (DBObject& dbObject);

    DBObject &dbObject() const;

    std::string JSONKey() const;
    void JSONKey(const std::string& json_key);

    std::string JSONValue() const;
    void JSONValue(const std::string& json_value);

    std::string JSONContainerKey() const;
    void JSONContainerKey(const std::string& key);

    void addMapping (JsonKey2DBOVariableMapping mapping);
    MappingIterator begin() { return data_mappings_.begin(); }
    MappingIterator end() { return data_mappings_.end(); }

    bool hasFilledBuffer ();
    std::shared_ptr<Buffer> buffer();
    void clearBuffer ();

    void transformBuffer ();

    unsigned int parseJSON (nlohmann::json& j);

    bool overrideKeyVariable() const;
    void overrideKeyVariable(bool override);

    DBOVariableSet& variableList();

    bool overrideDataSource() const;
    void OverrideDataSource(bool override);

    std::string dataSourceVariableName() const;
    void dataSourceVariableName(const std::string& name);

    unsigned int keyCount() const;
    void keyCount(unsigned int keyCount);

    bool keyInitialized() { return key_initialized_; }
    void initializeKey ();

private:
    DBObject& db_object_;

    std::string json_container_key_;  // location of container with target report data
    std::string json_key_; // * for all
    std::string json_value_;
    std::vector <JsonKey2DBOVariableMapping> data_mappings_;

    DBOVariableSet var_list_;

    bool override_key_variable_ {false};
    bool has_key_mapping_ {false};
    bool has_key_variable_ {false};

    bool override_data_source_ {false};
    std::string data_source_variable_name_;

    bool key_initialized_ {false};
    unsigned int key_count_ {0};
    DBOVariable* key_variable_ {nullptr};

    bool not_parse_all_ {false};

    PropertyList list_;
    std::shared_ptr<Buffer> buffer_;

    bool parseTargetReport (const nlohmann::json& tr, size_t row_cnt);

    inline std::string toString(const nlohmann::json& j)
    {
        if (j.type() == nlohmann::json::value_t::string) {
            return j.get<std::string>();
        }

        return j.dump();
    }

};


#endif // JSONMAPPING_H
