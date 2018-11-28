#ifndef JSONOBJECTPARSER_H
#define JSONOBJECTPARSER_H

#include <string>
#include <memory>

#include "propertylist.h"

#include "format.h"
#include "dbovariable.h"
#include "dbovariableset.h"
#include "stringconv.h"
#include "jsondatamapping.h"

class DBObject;
class DBOVariable;
class Buffer;

class JSONObjectParser
{
    using MappingIterator = std::vector<JSONDataMapping>::iterator;
public:
    JSONObjectParser (DBObject& dbObject);

    DBObject &dbObject() const;

    std::string JSONKey() const;
    void JSONKey(const std::string& json_key);

    std::string JSONValue() const;
    void JSONValue(const std::string& json_value);

    std::string JSONContainerKey() const;
    void JSONContainerKey(const std::string& key);

    void addMapping (JSONDataMapping mapping);
    MappingIterator begin() { return data_mappings_.begin(); }
    MappingIterator end() { return data_mappings_.end(); }

    void transformBuffer (std::shared_ptr<Buffer> buffer, long key_begin=-1) const;

    unsigned int parseJSON (nlohmann::json& j, std::shared_ptr<Buffer> buffer) const;

    bool overrideKeyVariable() const;
    void overrideKeyVariable(bool override);

    const DBOVariableSet& variableList() const;

    bool overrideDataSource() const;
    void OverrideDataSource(bool override);

    std::string dataSourceVariableName() const;
    void dataSourceVariableName(const std::string& name);

    bool initialized() const { return initialized_; }
    void initialize ();

    std::shared_ptr<Buffer> getNewBuffer () const;

private:
    DBObject& db_object_;

    std::string json_container_key_;  // location of container with target report data
    std::string json_key_; // * for all
    std::string json_value_;
    std::vector <JSONDataMapping> data_mappings_;

    DBOVariableSet var_list_;

    bool override_key_variable_ {false};
    bool has_key_mapping_ {false};
    bool has_key_variable_ {false};

    bool override_data_source_ {false};
    std::string data_source_variable_name_;

    bool initialized_ {false};
    DBOVariable* key_variable_ {nullptr};

    bool not_parse_all_ {false};

    PropertyList list_;

    bool parseTargetReport (const nlohmann::json& tr, std::shared_ptr<Buffer> buffer, size_t row_cnt) const;
};

#endif // JSONOBJECTPARSER_H
