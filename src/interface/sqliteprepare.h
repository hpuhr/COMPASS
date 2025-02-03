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

#include <sqlite3.h>

#include "dbprepare.h"

#include "timeconv.h"

#include <string>
#include <memory>

#include <boost/optional.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

#include <json.hpp>

/**
 * Handles scoped preparation and execution of sql statements.
 */
class SQLitePrepare : public DBPrepare
{
public:
    SQLitePrepare(sqlite3* connection);
    virtual ~SQLitePrepare();

protected:
    bool init_impl(const std::string& sql_statement) override final;
    void cleanup_impl() override final;
    void cleanupBinds_impl() override final;

    bool beginTransaction_impl() override final;
    bool endTransaction_impl() override final;

    bool bind_null_impl(size_t idx) override final { return sqlite3_bind_null(statement_, idx) == SQLITE_OK; }
    bool bind_bool_impl(size_t idx, bool v) override final { return bind<int>(idx, static_cast<int>(v)); }
    bool bind_char_impl(size_t idx, char v) override final { return bind<int>(idx, static_cast<int>(v)); }
    bool bind_uchar_impl(size_t idx, unsigned char v) override final { return bind<int>(idx, static_cast<int>(v)); }
    bool bind_int_impl(size_t idx, int v) override final { return bind<int>(idx, v); }
    bool bind_uint_impl(size_t idx, unsigned int v) override final { return bind<int>(idx, static_cast<int>(v)); }
    bool bind_long_impl(size_t idx, long v) override final { return bind<long>(idx, v); }
    bool bind_ulong_impl(size_t idx, unsigned long v) override final { return bind<long>(idx, static_cast<long>(v)); }
    bool bind_float_impl(size_t idx, float v) override final { return bind<double>(idx, static_cast<double>(v)); }
    bool bind_double_impl(size_t idx, double v) override final { return bind<double>(idx, v); }
    bool bind_string_impl(size_t idx, const std::string& v) override final { return bind<std::string>(idx, v); }
    bool bind_json_impl(size_t idx, const nlohmann::json& v) override final { return bind<nlohmann::json>(idx, v); }
    bool bind_timestamp_impl(size_t idx, const boost::posix_time::ptime& v) override final { return bind<boost::posix_time::ptime>(idx, v); }

    virtual bool executeBinds_impl() = 0;
    virtual bool execute_impl(const ExecOptions* options, DBResult* result) = 0;

    template<typename T>
    bool bind(size_t idx, const T& value)
    {
        throw std::runtime_error("SQLitePrepare: bind: not implemented for type");
        return false;
    }

private:
    sqlite3*      connection_ = nullptr;
    sqlite3_stmt* statement_  = nullptr;
};

#define StandardBind(DType, SQLTypeName)                                    \
template<>                                                                  \
inline bool SQLitePrepare::bind(size_t idx, const DType& value)             \
{                                                                           \
    return sqlite3_bind_##SQLTypeName(statement_, idx, value) == SQLITE_OK; \
}

StandardBind(int, int)
StandardBind(long, int64)
StandardBind(double, double)

template<>
inline bool SQLitePrepare::bind(size_t idx, const std::string& value)
{
    return sqlite3_bind_text(statement_, idx, value.c_str(), -1, SQLITE_TRANSIENT) == SQLITE_OK;
}

template<>
inline bool SQLitePrepare::bind(size_t idx, const nlohmann::json& value)
{
    auto str = value.dump();
    return bind<std::string>(idx, str);
}

template<>
inline bool SQLitePrepare::bind(size_t idx, const boost::posix_time::ptime& value)
{
    long ts = Utils::Time::toLong(value);
    return bind<long>(idx, ts);
}
