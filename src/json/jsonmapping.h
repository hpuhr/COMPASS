#ifndef JSONMAPPING_H
#define JSONMAPPING_H

#include <string>
#include <memory>

#include "propertylist.h"
#include "json.hpp"
#include "format.h"
#include "dbovariable.h"

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

    std::string json_key_;
    DBOVariable& variable_;
    bool mandatory_;
    Format json_value_format_;
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

private:
    DBObject& db_object_;

    std::string json_container_key_;  // location of container with target report data
    std::string json_key_; // * for all
    std::string json_value_;
    std::vector <JsonKey2DBOVariableMapping> data_mappings_;

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
