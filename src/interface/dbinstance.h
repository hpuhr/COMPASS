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

#include "dbdefs.h"

#include "result.h"

#include <memory>
#include <string>
#include <vector>

#include <boost/thread/mutex.hpp>
#include <boost/optional.hpp>

class DBInterface;
class DBConnection;

/**
 * A DBInstance is governing a db file/project. 
 * It is separated from a DBConnection, as in many db frameworks
 * there can be mutliple connections operating on a single db instance.
 * The instance can spawn new connections on demand.
 */
class DBInstance
{
public:
    /**
     */
    class ConnectionWrapper
    {
    public:
        typedef std::function<void(DBConnection*)> Destroyer;

        ConnectionWrapper(DBInstance* instance,
                          DBConnection* connection,
                          const Destroyer& destroyer)
        :   instance_  (instance  )
        ,   connection_(connection)
        ,   destroyer_ (destroyer )
        {
            assert(instance_  );
            assert(connection_);
            assert(destroyer_ );
        }

        ConnectionWrapper(const std::string& error)
        :   error_(error) {}

        virtual ~ConnectionWrapper()
        {
            if (connection_ && !detached_)
                destroyer_(connection_);
        }

        bool hasError() const { return error_.has_value(); }
        const std::string& error() { return error_.value(); }

        DBConnection& connection()
        { 
            assert(!hasError());

            return *connection_; 
        }

        void detach()
        {
            detached_ = true;
        }

    private:
        DBInstance*   instance_   = nullptr;
        DBConnection* connection_ = nullptr;
        Destroyer     destroyer_;
        bool          detached_   = false;

        boost::optional<std::string> error_;
    };

    typedef std::unique_ptr<ConnectionWrapper> ConnectionWrapperPtr;
    typedef std::unique_ptr<DBConnection>      ConnectionPtr;

    DBInstance(DBInterface* interface); 
    virtual ~DBInstance();

    std::string status() const;

    bool dbOpen() const;
    bool dbConnected() const;
    bool dbReady() const;

    Result open(const std::string& file_name);
    void close();
    Result reconnect(bool cleanup_db = false, Result* cleanup_result = nullptr);

    const std::string& dbFilename() const { return db_filename_; }

    DBConnection& defaultConnection();
    ConnectionWrapperPtr newCustomConnection();
    void destroyCustomConnections();

    size_t numCustomConnections() const;

    db::SQLConfig sqlConfiguration(bool verbose = false) const;
    
protected:
    DBInterface& interface() { return interface_; }
    const DBInterface& interface() const { return interface_; }

    /// implements opening of a db file
    virtual Result open_impl(const std::string& file_name) = 0;

    /// implements closing of a db file
    virtual void close_impl() = 0;

    /// implements connection creation
    virtual ResultT<DBConnection*> createConnection_impl(bool verbose) = 0;

    /// implements db file cleanup (if supported)
    virtual Result cleanupDB_impl(const std::string& db_fn);

    /// db backend specific sql settings
    virtual db::SQLConfig sqlConfiguration_impl() const = 0;

private:
    Result cleanupDB(const std::string& db_fn);

    ResultT<DBConnection*> createConnection(bool verbose);
    void destroyCustomConnection(DBConnection* conn);

    DBInterface& interface_;
    std::string  db_filename_;
    bool         db_open_      = false; 
    bool         db_connected_ = false;

    std::vector<std::unique_ptr<DBConnection>> custom_connections_;
    std::unique_ptr<DBConnection>              default_connection_;
    boost::mutex                               connection_mutex_;
    std::function<void(DBConnection*)>         destroyer_;
};
