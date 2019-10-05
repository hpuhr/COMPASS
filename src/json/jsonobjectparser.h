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

#ifndef JSONOBJECTPARSER_H
#define JSONOBJECTPARSER_H

#include <string>
#include <memory>

#include "propertylist.h"
#include "configurable.h"
#include "format.h"
#include "dbovariable.h"
#include "dbovariableset.h"
#include "stringconv.h"
#include "jsondatamapping.h"
#include "jsonobjectparserwidget.h"

class DBObject;
class DBOVariable;
class Buffer;

class JSONObjectParser : public Configurable
{
    using MappingIterator = std::vector<JSONDataMapping>::iterator;
public:
    JSONObjectParser (const std::string& class_id, const std::string& instance_id, Configurable* parent);
    JSONObjectParser() = default;
    JSONObjectParser(JSONObjectParser&& other) { *this = std::move(other); }

    /// @brief Move constructor
    JSONObjectParser& operator=(JSONObjectParser&& other);

    DBObject &dbObject() const;

    std::string JSONKey() const;
    void JSONKey(const std::string& json_key);

    std::string JSONValue() const;
    void JSONValue(const std::string& json_value);

    std::string JSONContainerKey() const;
    void JSONContainerKey(const std::string& key);

    MappingIterator begin() { return data_mappings_.begin(); }
    MappingIterator end() { return data_mappings_.end(); }
    bool hasMapping (unsigned int index) const;
    void removeMapping (unsigned int index);

    void transformBuffer (std::shared_ptr<Buffer> buffer, long key_begin=-1) const;

    // returs true on successful parse
    bool parseJSON (nlohmann::json& j, std::shared_ptr<Buffer> buffer) const;
    void createMappingStubs (nlohmann::json& j);

    const DBOVariableSet& variableList() const;

    bool overrideDataSource() const;
    void overrideDataSource(bool override);

    std::string dataSourceVariableName() const;
    void dataSourceVariableName(const std::string& name);

    bool initialized() const { return initialized_; }
    void initialize ();

    std::shared_ptr<Buffer> getNewBuffer () const;

    virtual void generateSubConfigurable (const std::string& class_id, const std::string& instance_id);

    JSONObjectParserWidget* widget ();

    std::string dbObjectName() const;

    void setMappingActive (JSONDataMapping& mapping, bool active);

private:
    std::string db_object_name_;
    DBObject* db_object_ {nullptr};

    std::string json_container_key_;  // location of container with target report data
    std::string json_key_; // * for all
    std::string json_value_;

    std::vector<std::string> json_values_vector_;

    DBOVariableSet var_list_;

    bool override_data_source_ {false};
    std::string data_source_variable_name_;

    bool initialized_ {false};

    bool not_parse_all_ {false};

    PropertyList list_;

    std::unique_ptr<JSONObjectParserWidget> widget_;

    std::vector <JSONDataMapping> data_mappings_;

    // returns true on successful parse
    bool parseTargetReport (const nlohmann::json& tr, std::shared_ptr<Buffer> buffer, size_t row_cnt) const;
    void createMappingsFromTargetReport (const nlohmann::json& tr);

protected:
    virtual void checkSubConfigurables () {}
};

#endif // JSONOBJECTPARSER_H
