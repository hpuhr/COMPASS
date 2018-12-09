#ifndef JSONPARSINGSCHEMA_H
#define JSONPARSINGSCHEMA_H

#include "configurable.h"
#include "jsonobjectparser.h"

#include <vector>

class JSONImporterTask;

class JSONParsingSchema : public Configurable
{
    using JSONObjectParserIterator = std::map<std::string, JSONObjectParser>::iterator;

public:
    JSONParsingSchema(const std::string& class_id, const std::string& instance_id, JSONImporterTask& task);
    JSONParsingSchema() = default;
    JSONParsingSchema(JSONParsingSchema&& other) { *this = std::move(other); }

    /// @brief Move constructor
    JSONParsingSchema& operator=(JSONParsingSchema&& other);

    JSONObjectParserIterator begin() { return parsers_.begin(); }
    JSONObjectParserIterator end() { return parsers_.end(); }

    const std::map<std::string, JSONObjectParser>& parsers () { return parsers_; }
    bool hasObjectParser (const std::string& name) { return parsers_.count(name) > 0; }
    JSONObjectParser& parser (const std::string& name);
    void removeParser (const std::string& name);

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    std::string name() const;
    void name(const std::string &name);

private:
    std::string name_;
    JSONImporterTask* task_ {nullptr};
    std::map <std::string, JSONObjectParser> parsers_;

protected:
    virtual void checkSubConfigurables () {}
};

#endif // JSONPARSINGSCHEMA_H
