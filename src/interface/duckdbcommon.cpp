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

#include "duckdbcommon.h"
#include "buffer.h"
#include "property_templates.h"
#include "propertylist.h"

#include <cassert>

/**************************************************************************************************
 * DuckDBResult
 **************************************************************************************************/

/**
 */
PropertyDataType DuckDBResult::dataTypeFromDuckDB(duckdb_type type)
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

    logerr << "DuckDBResult: dataTypeFromDuckDB: data type not implemented";
    assert(false);

    return PropertyDataType::BOOL;
}

/**
 */
DuckDBResult::DuckDBResult()
{
}

/**
 */
DuckDBResult::~DuckDBResult()
{
    if (has_result_)
    {
        //@TODO: any extra freeing needed?
        duckdb_destroy_result(&result_);
        has_result_ = false;
        error_ = false;
        error_msg_ = "";
    }
}

/**
 * Generate a buffer from the current result.
 * In this version the scheme is specified by the result.
 */
std::shared_ptr<Buffer> DuckDBResult::toBuffer(const std::string& dbcontent_name)
{
    if (!usable())
        return std::shared_ptr<Buffer>();

    idx_t col_count = duckdb_column_count(&result_);

    //collect props
    PropertyList properties;
    for (idx_t c = 0; c < col_count; ++c)
    {
        std::string name(duckdb_column_name(&result_, c));
        auto dtype = DuckDBResult::dataTypeFromDuckDB(duckdb_column_type(&result_, c));

        properties.addProperty(name, dtype);
    }

    return toBuffer(properties, dbcontent_name);
}

/**
 * Generate a buffer from the current result.
 * In this version the scheme is specified by the given property list.
 */
std::shared_ptr<Buffer> DuckDBResult::toBuffer(const PropertyList& properties,
                                               const std::string& dbcontent_name)
{
    if (!usable() || properties.size() < 1)
        return std::shared_ptr<Buffer>();

    //create buffer
    std::shared_ptr<Buffer> buffer(new Buffer(properties, dbcontent_name));

    if (!toBuffer(*buffer))
        return std::shared_ptr<Buffer>();

    return buffer;
}

/**
 * Fill a given buffer with the current result.
 * In this version the scheme is specified by the buffer.
 */
bool DuckDBResult::toBuffer(Buffer& buffer)
{
    if (!usable())
        return false;

    const auto& properties = buffer.properties();

    idx_t col_count = duckdb_column_count(&result_);
    idx_t row_count = duckdb_row_count(&result_);
    assert(col_count == properties.size()); // result column count must match provided buffer

    #define UpdateFunc(PDType, DType)                             \
        bool is_null = duckdb_value_is_null(&result_, c, r);      \
        if (!is_null)                                             \
            buffer.get<DType>(p.name()).set(r, read<DType>(c, r));

    #define NotFoundFunc                                                                         \
        logerr << "DuckDBResult: toBuffer: unknown property type " << Property::asString(dtype); \
        assert(false);

    for (idx_t r = 0; r < row_count; ++r)
    {
        for (idx_t c = 0; c < col_count; ++c)
        {
            const auto& p = properties.at(c);
            auto dtype = p.dataType();

            SwitchPropertyDataType(dtype, UpdateFunc, NotFoundFunc)
        }
    }

    return true;
}
