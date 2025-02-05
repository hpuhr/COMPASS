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

#include "dbexecresult.h"

#include "property.h"
#include "timeconv.h"

#include <memory>
#include <string>
#include <cassert>
#include <stdexcept>

#include <json.hpp>

#include <boost/date_time/posix_time/ptime.hpp>

class Buffer;
class PropertyList;

/**
 */
class DuckDBExecResult : public DBExecResult
{
public:
    DuckDBExecResult();
    virtual ~DuckDBExecResult();

    std::shared_ptr<Buffer> toBuffer(const std::string& dbcontent_name = "") override final;
    std::shared_ptr<Buffer> toBuffer(const PropertyList& properties,
                                     const std::string& dbcontent_name = "") override final;
    bool toBuffer(Buffer& buffer) override final;

    std::string errorString() const override final;

    void setResultValid();
    duckdb_result* result();

    static PropertyDataType dataTypeFromDuckDB(duckdb_type type);

private:
    template <typename T>
    T read(idx_t col, idx_t row)
    {
        assert(result_valid_);
        throw std::runtime_error("DuckDBResult: read: not implemented for type");
    }

    mutable duckdb_result result_;
    bool                  result_valid_;
};

#define StandardRead(DType, DuckDBDTypeName)                         \
template<>                                                           \
inline DType DuckDBExecResult::read(idx_t col, idx_t row)            \
{                                                                    \
    assert(result_valid_);                                           \
    return duckdb_value_ ## DuckDBDTypeName(&result_, col, row);     \
}

StandardRead(bool, boolean)
StandardRead(char, int8)
StandardRead(unsigned char, uint8)
StandardRead(int, int32)
StandardRead(unsigned int, uint32)
StandardRead(long, int64)
StandardRead(unsigned long, uint64)
StandardRead(float, float)
StandardRead(double, double)

template<>
inline std::string DuckDBExecResult::read(idx_t col, idx_t row)
{
    assert(result_valid_);
    return std::string(duckdb_value_varchar(&result_, col, row));
}

template<>
inline nlohmann::json DuckDBExecResult::read(idx_t col, idx_t row)
{
    assert(result_valid_);
    auto str = read<std::string>(col, row);
    return nlohmann::json::parse(str);
}

template<>
inline boost::posix_time::ptime DuckDBExecResult::read(idx_t col, idx_t row)
{
    assert(result_valid_);
    long ts = read<long>(col, row);
    return Utils::Time::fromLong(ts);
}
