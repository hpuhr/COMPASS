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

#ifndef MYSQLSERVER_H
#define MYSQLSERVER_H

#include <cassert>

#include "configurable.h"
#include "mysqlppconnection.h"
#include "mysqlserverwidget.h"

class MySQLServer : public Configurable
{
  public:
    MySQLServer(const std::string& instance_id, MySQLppConnection& parent)
        : Configurable("MySQLServer", instance_id, &parent), connection_(parent), widget_(nullptr)
    {
        registerParameter("host", &host_, "");
        registerParameter("user", &user_, "");
        registerParameter("password", &password_, "");
        registerParameter("port", &port_, 3306);
        registerParameter("database", &database_, "");
    }

    virtual ~MySQLServer()
    {
        if (widget_)
            widget_ = nullptr;
    }

    /// Returns the database server name or IP address
    const std::string& host() const { return host_; }
    void host(const std::string& host) { host_ = host; }

    /// Returns the username
    const std::string user() const { return user_; }
    void user(const std::string& user) { user_ = user; }

    /// Returns the password
    const std::string password() const { return password_; }
    void password(const std::string& password) { password_ = password; }

    /// Returns the port number
    unsigned int port() const { return port_; }
    void port(unsigned int port) { port_ = port; }

    const std::string database() const { return database_; }
    void database(const std::string& database) { database_ = database; }

    MySQLServerWidget* widget()
    {
        if (!widget_)
            widget_.reset(new MySQLServerWidget(connection_, *this));

        assert(widget_);
        return widget_.get();
    }

    void deleteWidget() { widget_ = nullptr; }

  protected:
    MySQLppConnection& connection_;
    /// Database server name or IP address
    std::string host_;
    /// Username
    std::string user_;
    /// Password
    std::string password_;
    /// Port number
    unsigned int port_;

    std::string database_;

    std::unique_ptr<MySQLServerWidget> widget_;
};

#endif  // MYSQLSERVER_H
