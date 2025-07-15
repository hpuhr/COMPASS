#pragma once

#include "configurable.h"
#include "asterixjsonparser.h"

#include <jasterix/iteminfo.h>

#include <vector>
#include <memory>

class ASTERIXImportTask;

class ASTERIXJSONParsingSchema : public Configurable
{
    using ASTERIXJSONParserIterator = std::map<unsigned int, std::unique_ptr<ASTERIXJSONParser>>::iterator;

public:
    ASTERIXJSONParsingSchema(const std::string& class_id, const std::string& instance_id,
                             ASTERIXImportTask& task);
    /// @brief Move constructor
//    ASTERIXJSONParsingSchema& operator=(ASTERIXJSONParsingSchema&& other);

    ASTERIXJSONParserIterator begin() { return parsers_.begin(); }
    ASTERIXJSONParserIterator end() { return parsers_.end(); }

    std::map<unsigned int, std::unique_ptr<ASTERIXJSONParser>>& parsers() { return parsers_; }
    bool hasObjectParser(unsigned int category) { return parsers_.count(category) > 0; }
    ASTERIXJSONParser& parser(unsigned int);
    void removeParser(unsigned int);

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    std::string name() const;
    void name(const std::string& name);

    //void updateMappings();

private:
    ASTERIXImportTask& task_;

    std::string name_;
    std::map<unsigned int, std::unique_ptr<ASTERIXJSONParser>> parsers_;

  protected:
    virtual void checkSubConfigurables() {}

};

