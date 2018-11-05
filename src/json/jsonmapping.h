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

class DBObject;
class DBOVariable;
class Buffer;

class JsonKey2DBOVariableMapping
{
public:
    JsonKey2DBOVariableMapping (std::string json_key, DBOVariable& variable, bool mandatory,
                                Format json_value_format)
        : json_key_(json_key), variable_(variable), mandatory_(mandatory), json_value_format_(json_value_format)
    {}

    JsonKey2DBOVariableMapping (std::string json_key, DBOVariable& variable, bool mandatory)
        : json_key_(json_key), variable_(variable), mandatory_(mandatory), json_value_format_(variable_.dataType(), "")
    {}

    bool hasKey (nlohmann::json& j) { return j.find (json_key_) != j.end(); }
    bool isNull (nlohmann::json& j) { return j[json_key_] == nullptr; }
    nlohmann::json& getValue (nlohmann::json& j) { return j[json_key_]; }

    template<typename T>
    void setValue(ArrayListTemplate<T>& array_list, unsigned int row_cnt, nlohmann::json& j)
    {
        if (isNull(j))
            array_list.setNone(row_cnt);
        else
        {
            try
            {
                if (json_value_format_ == "")
                    array_list.set(row_cnt, getValue(j));
                else
                    array_list.setFromFormat(row_cnt, json_value_format_,
                                             toString(getValue(j)));

                logdbg << "JsonKey2DBOVariableMapping: setValue: json " << getValue(j)
                       << " buffer " << array_list.get(row_cnt);
            }
            catch (nlohmann::json::exception& e)
            {
                logerr  <<  "JsonMapping: parseJSON: json exception " << e.what();
                array_list.setNone(row_cnt);
            }
        }
    }

    void setValue(ArrayListTemplate<char>& array_list, unsigned int row_cnt, nlohmann::json& j)
    {
        if (isNull(j))
            array_list.setNone(row_cnt);
        else
        {
            try
            {
                if (json_value_format_ == "")
                    array_list.set(row_cnt, static_cast<int> (getValue(j)));
                else
                    array_list.setFromFormat(row_cnt, json_value_format_,
                                             toString(getValue(j)));

                logdbg << "JsonKey2DBOVariableMapping: setValue: json " << getValue(j)
                       << " buffer " << array_list.get(row_cnt);
            }
            catch (nlohmann::json::exception& e)
            {
                logerr  <<  "JsonMapping: parseJSON: json exception " << e.what();
                array_list.setNone(row_cnt);
            }
        }
    }


private:
    std::string json_key_;
    DBOVariable& variable_;
    bool mandatory_;
    Format json_value_format_;

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

    std::shared_ptr<Buffer> buffer() const;
    void clearBuffer ();

    unsigned int parseJSON (nlohmann::json& j, bool test);

    bool overrideKeyVariable() const;
    void overrideKeyVariable(bool override);

    DBOVariableSet& variableList();

    bool overrideDataSource() const;
    void OverrideDataSource(bool override);

    std::string dataSourceVariableName() const;
    void dataSourceVariableName(const std::string& name);

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

    unsigned int key_count_ {0};

    PropertyList list_;
    std::shared_ptr<Buffer> buffer_;

    inline std::string toString(const nlohmann::json &j)
    {
        if (j.type() == nlohmann::json::value_t::string) {
            return j.get<std::string>();
        }

        return j.dump();
    }

};


#endif // JSONMAPPING_H
