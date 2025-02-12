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

#include "duckdbexecresult.h"
#include "buffer.h"
#include "property_templates.h"
#include "propertylist.h"

#include <cassert>

/**
 */
PropertyDataType DuckDBExecResult::dataTypeFromDuckDB(duckdb_type type)
{
    if (type == duckdb_type::DUCKDB_TYPE_BOOLEAN)
        return PropertyDataType::BOOL;
    else if (type == duckdb_type::DUCKDB_TYPE_TINYINT)
        return PropertyDataType::CHAR;
    else if (type == duckdb_type::DUCKDB_TYPE_UTINYINT)
        return PropertyDataType::UCHAR;
    else if (type == duckdb_type::DUCKDB_TYPE_INTEGER)
        return PropertyDataType::INT;
    else if (type == duckdb_type::DUCKDB_TYPE_UINTEGER)
        return PropertyDataType::UINT;
    else if (type == duckdb_type::DUCKDB_TYPE_BIGINT)
        return PropertyDataType::LONGINT;
    else if (type == duckdb_type::DUCKDB_TYPE_UBIGINT)
        return PropertyDataType::ULONGINT;
    else if (type == duckdb_type::DUCKDB_TYPE_FLOAT)
        return PropertyDataType::FLOAT;
    else if (type == duckdb_type::DUCKDB_TYPE_DOUBLE)
        return PropertyDataType::DOUBLE;
    else if (type == duckdb_type::DUCKDB_TYPE_VARCHAR)
        return PropertyDataType::STRING;
    
    //@TODO: more types needed?

    logerr << "DuckDBExecResult: dataTypeFromDuckDB: data type not implemented";
    assert(false);

    return PropertyDataType::BOOL;
}

/**
 */
DuckDBExecResult::DuckDBExecResult() = default;

/**
 */
DuckDBExecResult::~DuckDBExecResult()
{
    if (result_valid_)
    {
        //@TODO: any extra freeing needed?
        duckdb_destroy_result(&result_);
        result_valid_ = false;
    }
}

/**
 */
bool DuckDBExecResult::hasError() const
{
    assert(result_valid_);
    return result_error_;
}

/**
 */
std::string DuckDBExecResult::errorString() const
{
    if (!hasError())
        return "";

    std::string errstr = (duckdb_result_error(&result_));
    if (errstr.empty())
        errstr = "unknown error";

    return errstr;
}

/**
 */
boost::optional<PropertyList> DuckDBExecResult::propertyList() const
{
    assert(result_valid_);

    if (hasError())
        return boost::optional<PropertyList>();

    auto num_cols = numColumns();
    if (!num_cols.has_value())
        return boost::optional<PropertyList>();

    size_t nc = num_cols.value();

    //collect props
    PropertyList properties;
    for (idx_t c = 0; c < nc; ++c)
    {
        std::string name(duckdb_column_name(&result_, c));
        auto dtype = DuckDBExecResult::dataTypeFromDuckDB(duckdb_column_type(&result_, c));

        properties.addProperty(name, dtype);
    }

    return properties;
}

/**
 */
boost::optional<size_t> DuckDBExecResult::numColumns() const
{
    assert(result_valid_);

    if (hasError())
        return boost::optional<size_t>();

    size_t n = duckdb_column_count(&result_);
    return n;
}

/**
 */
boost::optional<size_t> DuckDBExecResult::numRows() const
{
    assert(result_valid_);

    if (hasError())
        return boost::optional<size_t>();

    size_t n = duckdb_row_count(&result_);
    return n;
}

/**
 */
duckdb_result* DuckDBExecResult::result()
{
    return &result_;
}

/**
 * Fill a given buffer with the current result.
 * In this version the scheme is specified by the buffer.
 */
bool DuckDBExecResult::toBuffer(Buffer& buffer,
                                const boost::optional<size_t>& offset,
                                const boost::optional<size_t>& max_entries)
{
    assert(result_valid_);

    const auto& properties = buffer.properties();

    auto nc = numColumns();
    auto nr = numRows();
    assert(nc.has_value());
    assert(nr.has_value());

    idx_t col_count = nc.value();
    idx_t row_count = nr.value();
    assert(col_count == properties.size()); // result column count must match provided buffer

    #define UpdateFunc(PDType, DType, Suffix)                              \
        bool is_null = duckdb_value_is_null(&result_, c, r);               \
        if (!is_null)                                                      \
        {                                                                  \
            DType v = read<DType>(c, r);                                   \
            buffer.get<DType>(pname).set(buf_idx, v);                      \
        }

    #define NotFoundFunc                                                                             \
        logerr << "DuckDBExecResult: toBuffer: unknown property type " << Property::asString(dtype); \
        assert(false);

    size_t r0 = offset.has_value() ? offset.value() : 0;
    size_t r1 = std::min(row_count, max_entries.has_value() ? r0 + max_entries.value() : row_count);

    //nothing to read?
    if (r0 >= r1 || r0 >= row_count)
        return true;

    //read rows into buffer
    for (idx_t r = r0, buf_idx = 0; r < r1; ++r, ++buf_idx)
    {
        for (idx_t c = 0; c < col_count; ++c)
        {
            const auto& p = properties.at(c);
            auto dtype = p.dataType();
            const auto& pname = p.name();

            SwitchPropertyDataType(dtype, UpdateFunc, NotFoundFunc)
        }
    }

    return true;
}
