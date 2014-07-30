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
 * DBConnection.h
 *
 *  Created on: Jul 19, 2012
 *      Author: sk
 */

#ifndef DBCONNECTION_H_
#define DBCONNECTION_H_

class DBCommand;
class DBCommandList;
class DBResult;
class DBConnectionInfo;
class Buffer;

/**
 * @brief Interface for any database connection
 *
 * @details Defines what function are required for a database connection, which is a class which can be used to encapsulate
 * different SQL based database systems and client libraries. The DBInterface only deals with a DBConnection, whatever the
 * underlying system requires.
 */
class DBConnection
{
public:
  /** @brief Constructor
   *
   * \param info defines what database system is used
   */
  DBConnection(DBConnectionInfo *info) {info_=info;}
  /// @brief Destructor
  virtual ~DBConnection() {};

  /// @brief Initializes the connection to the database
  virtual void init ()=0;
  /// @brief Executes a simple SQL command, returned data is not retrieved
  virtual void executeSQL(std::string sql)=0;

  /// @brief Create a bound statement, to which variables can be bound
  virtual void prepareBindStatement (std::string statement)=0;
  /// @brief Prepare a transaction (for fast insertion) for a bound statement
  virtual void beginBindTransaction ()=0;
  /// @brief Execute the transaction and clear bound variables
  virtual void stepAndClearBindings ()=0;
  /// @brief Commit the transaction.
  virtual void endBindTransaction ()=0;
  /// @brief Clear the bound statement for reuse
  virtual void finalizeBindStatement ()=0;

  /// @brief Bind a int variable at index to a value
  virtual void bindVariable (unsigned int index, int value)=0;
  /// @brief Bind a double variable at index to a value
  virtual void bindVariable (unsigned int index, double value)=0;
  /// @brief Bind a string variable at index to a value
  virtual void bindVariable (unsigned int index, const char *value)=0;
  /// @brief Bind a variable to the NULL value
  virtual void bindVariableNull (unsigned int index)=0;

  /// @brief Executes a database query where data can be returned
  virtual DBResult *execute (DBCommand *command)=0;
  /// @brief Executes a number of database queries where data (of the same structure) can be returned
  virtual DBResult *execute (DBCommandList *command_list)=0;

  /// @brief Prepare a database query for incremental data retrieval of the result
  virtual void prepareCommand (DBCommand *command)=0;
  /// @brief Step through a prepared query and return a number of results
  virtual DBResult *stepPreparedCommand (unsigned int max_results=0)=0;
  /// @brief Finalize the prepared query
  virtual void finalizeCommand ()=0;
  /// @brief Returns if all data from the prepared command was read
  virtual bool getPreparedCommandDone ()=0;

  /// @brief Return a Buffer with all table names (as strings) in the database
  virtual Buffer *getTableList()=0;
  /// @brief Return a Buffer with all columns and data types for a table
  virtual Buffer *getColumnList(std::string table)=0;

  /// @brief Return the DBConnectionInfo defining the database system and parameters
  DBConnectionInfo *getDBInfo () { return info_; }

protected:
  /// Defines the database system and parameters
  DBConnectionInfo *info_;

  /// @brief Creates a prepared query (internal)
  virtual void prepareStatement (const char *sql)=0;
  /// @brief Finalizes a prepared query (internal)
  virtual void finalizeStatement ()=0;

  /// @brief Executes an SQL command which returns data (internal)
  void execute (std::string command, Buffer *buffer);
};

#endif /* DBCONNECTION_H_ */
