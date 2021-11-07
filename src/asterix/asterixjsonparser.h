#ifndef ASTERIXJSONPARSER_H
#define ASTERIXJSONPARSER_H

#include "configurable.h"
#include "dbovariable.h"
#include "dbovariableset.h"
#include "format.h"
#include "jsondatamapping.h"
#include "asterixjsonparserwidget.h"
#include "propertylist.h"
#include "stringconv.h"

#include <memory>
#include <string>

class DBObject;
class DBOVariable;
class Buffer;

class ASTERIXJSONParser : public Configurable
{
    using MappingIterator = std::vector<JSONDataMapping>::iterator;

public:
    ASTERIXJSONParser(const std::string& class_id, const std::string& instance_id,
                      Configurable* parent);
//     ASTERIXJSONParser() = default;
     //ASTERIXJSONParser(JSONObjectParser&& other) { *this = std::move(other); }

     /// @brief Move constructor
     //ASTERIXJSONParser& operator=(ASTERIXJSONParser&& other);

     DBObject& dbObject() const;

     MappingIterator begin() { return data_mappings_.begin(); }
     MappingIterator end() { return data_mappings_.end(); }
     bool hasMapping(unsigned int index) const;
     void removeMapping(unsigned int index);

     // returs true on successful parse
     bool parseJSON(nlohmann::json& j, Buffer& buffer) const;
     void createMappingStubs(nlohmann::json& j);

     const DBOVariableSet& variableList() const;

     bool initialized() const { return initialized_; }
     void initialize();

     std::shared_ptr<Buffer> getNewBuffer() const;
     void appendVariablesToBuffer(Buffer& buffer) const;

     virtual void generateSubConfigurable(const std::string& class_id,
                                          const std::string& instance_id);

     ASTERIXJSONParserWidget* widget();

     std::string dbObjectName() const;

     void setMappingActive(JSONDataMapping& mapping, bool active);

     void updateMappings();

     std::string name() const;
     void name(const std::string& name);

     unsigned int category() const;

private:
     std::string name_;
     unsigned int category_;

     std::string db_object_name_;
     DBObject* db_object_{nullptr};

     DBOVariableSet var_list_;

     bool initialized_{false};

     PropertyList list_;

     std::unique_ptr<ASTERIXJSONParserWidget> widget_;

     std::vector<JSONDataMapping> data_mappings_;

     // returns true on successful parse
     bool parseTargetReport(const nlohmann::json& tr, Buffer& buffer, size_t row_cnt) const;
     void createMappingsFromTargetReport(const nlohmann::json& tr);

     void checkIfKeysExistsInMappings(const std::string& location, const nlohmann::json& tr,
                                      bool is_in_array = false);

   protected:
     virtual void checkSubConfigurables() {}
};

#endif // ASTERIXJSONPARSER_H
