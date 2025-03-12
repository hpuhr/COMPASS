/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <string>

namespace db
{

enum class SQLPlaceholder
{
    AtVar = 0,   //e.g. sqlite
    QuestionMark //e.g. duckdb
};

/**
 */
struct SQLConfig
{
    bool           precise_types           = false;
    SQLPlaceholder placeholder             = SQLPlaceholder::AtVar;
    bool           use_conflict_resolution = false;
    bool           indexing                = true;
    bool           supports_mt             = false;

    bool           verbose                 = true;
};

/**
 */
struct PerformanceMetrics
{
    enum Flags
    {
        PM_Read   = 1 << 0,
        PM_Insert = 1 << 1,
        PM_Update = 1 << 2,
    };

    std::string asString(int flags = 255) const;

    double read_time         = 0.0;
    size_t read_num_chunks   = 0;
    size_t read_num_rows     = 0;

    double insert_time       = 0.0;
    size_t insert_num_chunks = 0;
    size_t insert_num_rows   = 0;

    double update_time       = 0.0;
    size_t update_num_chunks = 0;
    size_t update_num_rows   = 0;

    bool   valid             = false;
};

/**
 */
struct Index : public std::pair<std::string, std::string>
{
    Index() {}
    Index(const std::string& index_name, 
            const std::string& column_name) : std::pair<std::string, std::string>(index_name, column_name) {}
    const std::string& indexName() const { return this->first; }
    const std::string& columnName() const { return this->second; }
};

/**
 */
struct SQLPragma : public std::pair<std::string, std::string>
{
    SQLPragma() {}
    SQLPragma(const std::string& name, 
              const std::string& value) : std::pair<std::string, std::string>(name, value) {}
    const std::string& name() const { return this->first; }
    const std::string& value() const { return this->second; }
};

}
