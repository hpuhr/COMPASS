#ifndef JSONPARSINGSCHEMA_H
#define JSONPARSINGSCHEMA_H

#include "configurable.h"
#include "jsonobjectparser.h"

#include <vector>

class JSONImporterTask;

class JSONParsingSchema : public Configurable
{
    using JSONObjectParserIterator = std::vector<JSONObjectParser>::iterator;

public:
    JSONParsingSchema(const std::string& class_id, const std::string& instance_id, JSONImporterTask& task);
    JSONParsingSchema() = default;
    JSONParsingSchema(JSONParsingSchema&& other) { *this = std::move(other); }

    /// @brief Move constructor
    JSONParsingSchema& operator=(JSONParsingSchema&& other);

    JSONObjectParserIterator begin() { return mappings_.begin(); }
    JSONObjectParserIterator end() { return mappings_.end(); }

    const std::vector<JSONObjectParser>& mappings () { return mappings_; }

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    std::string name() const;
    void name(const std::string &name);

private:
    std::string name_;
    JSONImporterTask* task_ {nullptr};
    std::vector <JSONObjectParser> mappings_;

protected:
    virtual void checkSubConfigurables () {}
};

#endif // JSONPARSINGSCHEMA_H
