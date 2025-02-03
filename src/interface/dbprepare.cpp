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

#include "dbprepare.h"
#include "dbresult.h"
#include "buffer.h"
#include "property_templates.h"

#include "logger.h"

/***************************************************************************************
 * DBPrepare
 ***************************************************************************************/

/**
 */
DBPrepare::DBPrepare()
{
}

/**
 */
DBPrepare::~DBPrepare()
{
    if (active_transaction_)
        logerr << "DBPrepare: ~DBPrepare: transaction still active";
    assert(!active_transaction_);

    if (active_binds_)
        logerr << "DBPrepare: ~DBPrepare: binds still active";
    assert(!active_binds_);

    if (prepared_stmnt_ok_)
        logerr << "DBPrepare: ~DBPrepare: cleanup not called";
    assert(!prepared_stmnt_ok_);
}

/**
 */
bool DBPrepare::init(const std::string& sql_statement)
{
    prepared_stmnt_ok_ = init_impl(sql_statement);
}

/**
 */
void DBPrepare::cleanup()
{
    if (active_transaction_)
        logerr << "DBPrepare: ~DBPrepare: transaction still active";
    assert(!active_transaction_);

    if (active_binds_)
        logerr << "DBPrepare: ~DBPrepare: binds still active";
    assert(!active_binds_);

    if (prepared_stmnt_ok_)
    {
        cleanup_impl();
        prepared_stmnt_ok_ = false;
    }
}

/**
 */
std::shared_ptr<DBResult> DBPrepare::execute(const ExecOptions& options)
{
    //create result
    std::shared_ptr<DBResult> result(new DBResult);

    //execute
    execute(&options, result.get());

    return result;
}

/**
 */
bool DBPrepare::execute(const ExecOptions* options, 
                        DBResult* result)
{
    if (!prepared_stmnt_ok_)
        logerr << "DBPrepare: execute: prepared statement invalid";
    assert(prepared_stmnt_ok_);

    bool ok = false;

    if (active_binds_)
    {
        //binds are active => execute bind statement as efficiently as possible
        ok = executeBinds_impl();
        cleanupBinds_impl();
    }
    else
    {
        //any data expected?
        if (result && options && options->dataExpected())
        {
            //create needed buffer (@TODO: could be inferred from result in duckdb...)
            std::shared_ptr<Buffer> b;
            b.reset(new Buffer(options->buffer_properties.value()));

            result->buffer(b);
        }

        //execute statement and optionally fetch data
        ok = execute_impl(options, result);
    }

    return ok;
}

/**
 */
bool DBPrepare::executeBuffer(const std::shared_ptr<Buffer>& buffer,
                              const boost::optional<size_t>& idx_from, 
                              const boost::optional<size_t>& idx_to)
{
    if (!prepared_stmnt_ok_)
        logerr << "DBPrepare: executeBuffer: prepared statement invalid";
    assert(prepared_stmnt_ok_);

    assert(buffer);

    if (buffer->size() == 0)
        return false;

    std::shared_ptr<Buffer> b = buffer;

    size_t idx0 = idx_from.has_value() ? idx_from.value() : 0;
    size_t idx1 = idx_to.has_value()   ? idx_to.value()   : b->size() - 1;
    assert(idx0 <= idx1);
    assert(idx1 < b->size());

    const auto& properties = b->properties().properties();
    size_t np = properties.size();

    #define UpdateFunc(PDType, DType, Suffix)                                             \
        bool is_null = b->isNull(p, r);                                                   \
        bool ok = is_null ? bind_null(c) : bind_##Suffix(c, b->get<DType>(pname).get(r)); \
        if (!ok)                                                                          \
            logerr << "DBPrepare: executeBuffer: updating '" << pname << "' failed";      \
        assert(ok);

    #define NotFoundFunc                                                                           \
        logerr << "DBPrepare: executeBuffer: unknown property type " << Property::asString(dtype); \
        assert(false);

    for (size_t r = idx0; r <= idx1; ++r)
    {
        for (size_t c = 0; c < np; ++c)
        {
            const auto& p     = properties[ c ];
            auto        dtype = p.dataType();
            const auto& pname = p.name();

            SwitchPropertyDataType(dtype, UpdateFunc, NotFoundFunc);
        }

        //use minimal execution version
        bool ok = execute(nullptr, nullptr);
        
        if (!ok)
            logerr << "DBPrepare: executeBuffer: updating buffer row " << r << " failed";
        assert(ok);
    }

    return true;
}

/**
 */
bool DBPrepare::beginTransaction()
{
    if (!prepared_stmnt_ok_)
        logerr << "DBPrepare: beginTransaction: prepared statement invalid";
    assert(prepared_stmnt_ok_);

    if (active_transaction_)
        logerr << "DBPrepare: beginTransaction: transaction already active";
    assert(!active_transaction_);

    bool ok = beginTransaction_impl();
    if (!ok)
        return false;

    active_transaction_ = true;

    return true;
}

/**
 */
bool DBPrepare::endTransaction()
{
    if (!prepared_stmnt_ok_)
        logerr << "DBPrepare: endTransaction: prepared statement invalid";
    assert(prepared_stmnt_ok_);

    if (!active_transaction_)
        logerr << "DBPrepare: endTransaction: no transaction active";
    assert(active_transaction_);

    bool ok = endTransaction_impl();
    if (!ok)
        return false;

    active_transaction_ = false;

    return true;
}

/**
 */
bool DBPrepare::bind_null(size_t idx) 
{ 
    assert(prepared_stmnt_ok_);
    active_binds_ = true; 
    return bind_null_impl(idx); 
}

/**
 */
bool DBPrepare::bind_bool(size_t idx, bool v) 
{ 
    assert(prepared_stmnt_ok_);
    active_binds_ = true; 
    return bind_bool_impl(idx, v); 
}

/**
 */
bool DBPrepare::bind_char(size_t idx, char v) 
{ 
    assert(prepared_stmnt_ok_);
    active_binds_ = true; 
    return bind_char_impl(idx, v); 
}

/**
 */
bool DBPrepare::bind_uchar(size_t idx, unsigned char v) 
{ 
    assert(prepared_stmnt_ok_);
    active_binds_ = true; 
    return bind_uchar_impl(idx, v); 
}

/**
 */
bool DBPrepare::bind_int(size_t idx, int v) 
{ 
    assert(prepared_stmnt_ok_);
    active_binds_ = true; 
    return bind_int_impl(idx, v); 
}

/**
 */
bool DBPrepare::bind_uint(size_t idx, unsigned int v) 
{ 
    assert(prepared_stmnt_ok_);
    active_binds_ = true; 
    return bind_uint_impl(idx, v); 
}

/**
 */
bool DBPrepare::bind_long(size_t idx, long v) 
{ 
    assert(prepared_stmnt_ok_);
    active_binds_ = true; 
    return bind_long_impl(idx, v); 
}

/**
 */
bool DBPrepare::bind_ulong(size_t idx, unsigned long v) 
{ 
    assert(prepared_stmnt_ok_);
    active_binds_ = true; 
    return bind_ulong_impl(idx, v); 
}

/**
 */
bool DBPrepare::bind_float(size_t idx, float v) 
{ 
    assert(prepared_stmnt_ok_);
    active_binds_ = true; 
    return bind_float_impl(idx, v); 
}

/**
 */
bool DBPrepare::bind_double(size_t idx, double v) 
{ 
    assert(prepared_stmnt_ok_);
    active_binds_ = true; 
    return bind_double_impl(idx, v); 
}

/**
 */
bool DBPrepare::bind_string(size_t idx, const std::string& v) 
{ 
    assert(prepared_stmnt_ok_);
    active_binds_ = true; 
    return bind_string_impl(idx, v); 
}

/**
 */
bool DBPrepare::bind_json(size_t idx, const nlohmann::json& v) 
{ 
    assert(prepared_stmnt_ok_);
    active_binds_ = true; 
    return bind_json_impl(idx, v); 
}

/**
 */
bool DBPrepare::bind_timestamp(size_t idx, const boost::posix_time::ptime& v) 
{ 
    assert(prepared_stmnt_ok_);
    active_binds_ = true; 
    return bind_timestamp_impl(idx, v); 
}

/***************************************************************************************
 * DBScopedPrepare
 ***************************************************************************************/

/**
 */
DBScopedPrepare::DBScopedPrepare(const std::shared_ptr<DBPrepare>& db_prepare, 
                                 const std::string& sql_statement,
                                 bool begin_transaction) 
:   db_prepare_(db_prepare) 
{
    assert(db_prepare_);
    db_prepare_->init(sql_statement);

    if (begin_transaction)
        db_prepare_->beginTransaction();
};

/**
 */
DBScopedPrepare::~DBScopedPrepare()
{
    if (db_prepare_->hasActiveTransaction())
        db_prepare_->endTransaction();

    db_prepare_->cleanup();
}

/**
 */
std::shared_ptr<DBResult> DBScopedPrepare::execute(const DBPrepare::ExecOptions& options) 
{ 
    return db_prepare_->execute(options); 
}

/**
 */
bool DBScopedPrepare::execute(const DBPrepare::ExecOptions* options, 
                              DBResult* result)
{
    return db_prepare_->execute(options, result);
}

/**
 */
bool DBScopedPrepare::executeBuffer(const std::shared_ptr<Buffer>& buffer,
                                    const boost::optional<size_t>& idx_from, 
                                    const boost::optional<size_t>& idx_to)
{
    return db_prepare_->executeBuffer(buffer, idx_from, idx_to);
}
