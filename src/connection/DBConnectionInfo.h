/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * DBConnectionInfo.h
 *
 *  Created on: Jul 19, 2012
 *      Author: sk
 */

#ifndef DBCONNECTIONINFO_H_
#define DBCONNECTIONINFO_H_

/// Database system identifier
enum DB_CONNECTION_TYPE { DB_TYPE_SQLITE=0, DB_TYPE_MYSQLpp, DB_TYPE_MYSQLCon };

/**
 * @brief Base interface for definition of a used database system
 *
 * Uses a connection type enum (DB_CONNECTION_TYPE) to define what database system is used (SQLite3, MySQL), and if
 * the database is being generated or read from.
 */
class DBConnectionInfo
{
public:
  /// @brief Constructor
  DBConnectionInfo(DB_CONNECTION_TYPE type) : type_(type) {}
  /// @brief Destructor
  virtual ~DBConnectionInfo() {}

  /// @brief Returns database system type
  DB_CONNECTION_TYPE getType () { return type_; }

  /// @brief Returns string identifying database system and database
  virtual std::string getIdString ()=0;

protected:
  /// Database system type
  DB_CONNECTION_TYPE type_;
};

/**
 * @brief Definition of a SQLite3 based database system
 */
//class SQLite3ConnectionInfo : public DBConnectionInfo
//{
//public:
//  SQLite3ConnectionInfo (std::string filename) : DBConnectionInfo(DB_TYPE_SQLITE), filename_(filename) {}
//  virtual ~SQLite3ConnectionInfo () {}
//
//  /// Returns the path and filename of the SQLite3 database file
//  std::string getFilename () { return filename_; }
//
//  std::string getIdString () { return "SQLite3 File: '"+filename_+"'";}
//
//private:
//  /// Path and filename of the SQLite3 database file
//  std::string filename_;
//};

/**
 * @brief Definition of a MySQL Connector based database system
 *
 */
class MySQLConnectionInfo : public DBConnectionInfo
{
public:
    MySQLConnectionInfo (DB_CONNECTION_TYPE type, std::string db, std::string server, std::string user, std::string password, unsigned int port)
   : DBConnectionInfo(type), db_(db), server_(server), user_(user), password_(password), port_(port) {}
  virtual ~MySQLConnectionInfo () {}

  /// Returns the database name
  std::string getDB () { return db_; }
  /// Returns the database server name or IP address
  std::string getServer () { return server_; }
  /// Returns the username
  std::string getUser () { return user_; }
  /// Returns the password
  std::string getPassword () { return password_; }
  /// Returns the port number
  unsigned int getPort () { return port_; }

  std::string getIdString () { return "MySQL Server: '"+server_+"' Database: '"+db_+"'";}

private:
  /// Database name
  std::string db_;
  /// Database server name or IP address
  std::string server_;
  /// Username
  std::string user_;
  /// Password
  std::string password_;
  /// Port number
  unsigned int port_;
};

/**
 * @brief Definition of a MySQL++ based database system
 *
 */
//class MySQLppConnectionInfo : public DBConnectionInfo
//{
//public:
//  MySQLppConnectionInfo (std::string db, std::string server, std::string user, std::string password, unsigned int port)
//   : DBConnectionInfo(DB_TYPE_MYSQLpp), server_(server), user_(user), password_(password), port_(port) {}
//  virtual ~MySQLppConnectionInfo () {}
//
//  /// Returns the database server name or IP address
//  std::string getServer () { return server_; }
//  /// Returns the username
//  std::string getUser () { return user_; }
//  /// Returns the password
//  std::string getPassword () { return password_; }
//  /// Returns the port number
//  unsigned int getPort () { return port_; }
//
//  std::string getIdString () { return "MySQL Server: '"+server_+"' User: '"+user_+"' Password: '" << password_ << "'";}
//
//  //virtual std::string getDBName () { return db_;}
//
//private:
//  /// Database name
////  std::string db_;
//  /// Database server name or IP address
//  std::string server_;
//  /// Username
//  std::string user_;
//  /// Password
//  std::string password_;
//  /// Port number
//  unsigned int port_;
//};

/**
 * @brief Definition of a MySQL Connector based database system
 *
 */
//class MySQLConConnectionInfo : public DBConnectionInfo
//{
//public:
//    MySQLConConnectionInfo (std::string db, std::string server, std::string user, std::string password, unsigned int port)
//   : DBConnectionInfo(DB_TYPE_MYSQLCon), db_(db), server_(server), user_(user), password_(password), port_(port) {}
//  virtual ~MySQLConConnectionInfo () {}
//
//  /// Returns the database name
//  std::string getDB () { return db_; }
//  /// Returns the database server name or IP address
//  std::string getServer () { return server_; }
//  /// Returns the username
//  std::string getUser () { return user_; }
//  /// Returns the password
//  std::string getPassword () { return password_; }
//  /// Returns the port number
//  unsigned int getPort () { return port_; }
//
//  std::string getIdString () { return "MySQL Server: '"+server_+"' Database: '"+db_+"'";}
//
//  //virtual std::string getDBName () { return db_;}
//
//private:
//  /// Database name
//  std::string db_;
//  /// Database server name or IP address
//  std::string server_;
//  /// Username
//  std::string user_;
//  /// Password
//  std::string password_;
//  /// Port number
//  unsigned int port_;
//};


#endif /* DBCONNECTIONINFO_H_ */
