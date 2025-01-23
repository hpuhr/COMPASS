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

#include <duckdb.h>

#include "timeconv.h"

#include <string>

#include <boost/optional.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

#include <json.hpp>

/**
 * Handles scoped appending of data to a specific table.
 * Will flush and destroy the appender on destruction.
 */
class DuckDBScopedAppender
{
public:
    DuckDBScopedAppender(duckdb_connection connection, 
                         const std::string& table_name);
    virtual ~DuckDBScopedAppender();

    bool valid() const;

    template<typename T>
    void append(const T& value)
    {
        assert(ok_);
        throw std::runtime_error("DuckDBScopedAppender: append: not implemented for type");
    }

    void appendNull()
    {
        assert(ok_);
        duckdb_append_null(appender_);
        ++appended_;
    }

    size_t endRow();
    void flush();
    size_t columnCount() const;
    size_t currentColumnCount() const { return appended_; }
    
private:
    duckdb_appender appender_;
    bool ok_ = false;
    size_t appended_ = 0;
};

#define StandardApppender(DType, DuckDBDTypeName)                                 \
template<>                                                                        \
inline void DuckDBScopedAppender::append(const DType& value)                      \
{                                                                                 \
    assert(ok_);                                                                  \
    duckdb_append_ ## DuckDBDTypeName(appender_, value);                          \
    ++appended_;                                                                  \
}

StandardApppender(bool, bool)
StandardApppender(char, int8)
StandardApppender(unsigned char, uint8)
StandardApppender(int, int32)
StandardApppender(unsigned int, uint32)
StandardApppender(long, int64)
StandardApppender(unsigned long, uint64)
StandardApppender(float, float)
StandardApppender(double, double)

template<>
inline void DuckDBScopedAppender::append(const std::string& value)
{
    assert(ok_);
    duckdb_append_varchar(appender_, value.c_str());
    ++appended_;
}

template<>
inline void DuckDBScopedAppender::append(const nlohmann::json& value)
{
    assert(ok_);
    auto str = value.dump();
    append<std::string>(str);
}

template<>
inline void DuckDBScopedAppender::append(const boost::posix_time::ptime& value)
{
    assert(ok_);
    long ts = Utils::Time::toLong(value);
    append<long>(ts);
}
