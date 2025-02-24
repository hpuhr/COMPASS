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

#include "dbinstance.h"
#include "dbinterface.h"
#include "dbconnection.h"

#include "logger.h"
#include "compass.h"

/**
 */
DBInstance::DBInstance(DBInterface* interface)
:   interface_(*interface)
{
    loginf << "DBInstance: constructor";
}

/**
 */
db::SQLConfig DBInstance::sqlConfiguration(bool verbose) const
{
    auto config = sqlConfiguration_impl();

    config.verbose = verbose;

    return config;
}

/**
 */
DBInstance::~DBInstance()
{
    loginf << "DBInstance: destructor";

    close();
}

/**
 * Returns the current instance status.
 */
std::string DBInstance::status() const
{
    if (!db_open_)
        return "Not connected";
    else if (!db_connected_)
        return "Not connected";

    return "Ready";
}

/**
 */
bool DBInstance::dbOpen() const
{
    return db_open_;
}

/**
 */
bool DBInstance::dbConnected() const
{
    return db_connected_;
}

/**
 */
bool DBInstance::dbReady() const
{
    return dbOpen() && dbConnected();
}

/**
 */
size_t DBInstance::numCustomConnections() const
{
    return custom_connections_.size();
}

/**
 * Opens the given database file and creates a default connection. 
 * Will close the currently opened database and destroy all open connections.
 */
Result DBInstance::open(const std::string& file_name)
{
    loginf << "DBInstance: open: '" << file_name << "'";

    //close first
    if (dbOpen())
        close();

    auto open_result = open_impl(file_name);
    if (!open_result.ok())
        return open_result;

    db_open_     = true;
    db_filename_ = file_name;

    //open default connection
    auto conn_result = createConnection();
    if (!conn_result.ok())
        return conn_result;

    default_connection_.reset(conn_result.result());
    assert(default_connection_);

    db_connected_ = true;

    loginf << "DBInstance: open: opened!";

    return Result::succeeded();
}

/**
 * Closes the instance and destroys all open connections.
 */
void DBInstance::close()
{
    loginf << "DBInstance: close, db open? " << db_open_;

    if (!dbOpen())
        return;

    loginf << "DBInstance: closing...";

    //destroy connections
    default_connection_.reset();
    custom_connections_.clear();

    close_impl();

    db_open_      = false;
    db_connected_ = false;
    db_filename_  = "";
}

/**
 */
Result DBInstance::reconnect(bool cleanup_db, Result* cleanup_result)
{
    assert(dbReady());

    auto fn = db_filename_;

    //close db
    close();

    if (cleanup_db)
    {
        //cleanup closed db file (cleanup might need closing of db handles)
        auto res = cleanupDB(fn);
        
        if (cleanup_result)
            *cleanup_result = res;
    }

    //re-open db
    return open(fn);
}

/**
 */
Result DBInstance::cleanupDB(const std::string& db_fn)
{
    assert(!dbOpen());

    return cleanupDB_impl(db_fn);
}

/**
 */
Result DBInstance::cleanupDB_impl(const std::string& db_fn)
{
    //derive if some special action is needed
    return Result::succeeded();
}

/**
 */
DBConnection& DBInstance::defaultConnection()
{
    assert(dbReady());
    assert(default_connection_);

    return *default_connection_;
}

/**
 */
DBInstance::ConnectionWrapperPtr DBInstance::newCustomConnection()
{
    assert(dbReady());

    connection_mutex_.lock();

    auto r = createConnection();

    if (r.ok())
        custom_connections_.emplace_back(r.result());

    connection_mutex_.unlock();

    if (!r.ok())
        return ConnectionWrapperPtr(new ConnectionWrapper(r.error()));

    auto destroy_cb = [ this ] (DBConnection* conn)
    {
        assert(conn);
        this->destroyCustomConnection(conn);
    };

    return ConnectionWrapperPtr(new ConnectionWrapper(this, r.result(), destroy_cb));
}

/**
 */
void DBInstance::destroyCustomConnection(DBConnection* conn)
{
    assert(conn);

    connection_mutex_.lock();

    auto it = std::find_if(custom_connections_.begin(), custom_connections_.end(), 
        [ & ] (const ConnectionPtr& c) { return c.get() == conn; });
    
    bool found = (it != custom_connections_.end());
    if (found)
    {
        it->get()->disconnect();
        custom_connections_.erase(it);
    }

    connection_mutex_.unlock();

    //connection already destroyed? => doom
    assert(found);
}

/**
 */
ResultT<DBConnection*> DBInstance::createConnection()
{
    assert(dbOpen());

    auto r = createConnection_impl();
    assert(!r.ok() || r.result() != nullptr);

    return r;
}
