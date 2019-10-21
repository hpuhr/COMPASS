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
    JSONParsingSchema(const std::string& class_id, const std::string& instance_id, Configurable* parent);
    JSONParsingSchema() = default;
    JSONParsingSchema(JSONParsingSchema&& other) { *this = std::move(other); }

    /// @brief Move constructor
    JSONParsingSchema& operator=(JSONParsingSchema&& other);

    JSONObjectParserIterator begin() { return parsers_.begin(); }
    JSONObjectParserIterator end() { return parsers_.end(); }

    std::map<std::string, JSONObjectParser>& parsers () { return parsers_; }
    bool hasObjectParser (const std::string& name) { return parsers_.count(name) > 0; }
    JSONObjectParser& parser (const std::string& name);
    void removeParser (const std::string& name);

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    std::string name() const;
    void name(const std::string &name);

    void updateMappings ();

private:
    std::string name_;
    std::map <std::string, JSONObjectParser> parsers_;

protected:
    virtual void checkSubConfigurables () {}
};

#endif // JSONPARSINGSCHEMA_H
