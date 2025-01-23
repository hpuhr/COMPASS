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
#include <memory>

#include <boost/optional.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

#include <json.hpp>

class DuckDBResult;

/**
 * Handles scoped preparation and execution of sql statements.
 */
class DuckDBScopedPrepare
{
public:
    DuckDBScopedPrepare(duckdb_connection connection, 
                        const std::string& sql_statement);
    virtual ~DuckDBScopedPrepare();

    bool valid() const;

    template<typename T>
    void bind(size_t idx, const T& value)
    {
        assert(ok_);
        throw std::runtime_error("DuckDBScopedPrepare: bind: not implemented for type");
    }

    void bindNull(size_t idx)
    {
        assert(ok_);
        duckdb_bind_null(statement_, idx);
    }

    std::shared_ptr<DuckDBResult> execute();

private:
    duckdb_prepared_statement statement_;
    bool ok_ = false;
};

#define StandardBind(DType, DuckDBDTypeName)                            \
template<>                                                              \
inline void DuckDBScopedPrepare::bind(size_t idx, const DType& value)   \
{                                                                       \
    assert(ok_);                                                        \
    duckdb_bind_ ## DuckDBDTypeName(statement_, idx, value);            \
}

StandardBind(bool, boolean)
StandardBind(char, int8)
StandardBind(unsigned char, uint8)
StandardBind(int, int32)
StandardBind(unsigned int, uint32)
StandardBind(long, int64)
StandardBind(unsigned long, uint64)
StandardBind(float, float)
StandardBind(double, double)

template<>
inline void DuckDBScopedPrepare::bind(size_t idx, const std::string& value)
{
    assert(ok_);
    duckdb_bind_varchar_length(statement_, idx, value.data(), value.size());
}

template<>
inline void DuckDBScopedPrepare::bind(size_t idx, const nlohmann::json& value)
{
    assert(ok_);
    auto str = value.dump();
    bind<std::string>(idx, str);
}

template<>
inline void DuckDBScopedPrepare::bind(size_t idx, const boost::posix_time::ptime& value)
{
    assert(ok_);
    long ts = Utils::Time::toLong(value);
    bind<long>(idx, ts);
}
