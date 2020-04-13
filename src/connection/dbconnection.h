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

#ifndef DBCONNECTION_H_
#define DBCONNECTION_H_

#include <qobject.h>

#include <memory>

#include "configurable.h"

class DBCommand;
class DBCommandList;
class DBResult;
class DBConnectionInfo;
class Buffer;
class DBTableInfo;
class QWidget;

/**
 * @brief Interface for any database connection
 *
 * @details Defines what function are required for a database connection, which is a class which can
 * be used to encapsulate different SQL based database systems and client libraries. The DBInterface
 * only deals with a DBConnection, whatever the underlying system requires.
 */
class DBConnection : public QObject, public Configurable
{
    Q_OBJECT

  signals:
    void connectedSignal();

  public:
    /** @brief Constructor
     *
     * \param info defines what database system is used
     */
    DBConnection(const std::string& class_id, const std::string& instance_id, Configurable* parent)
        : Configurable(class_id, instance_id, parent), connection_ready_(false)
    {
    }
    /// @brief Destructor
    virtual ~DBConnection() {}

    /// @brief Initializes the connection to the database
    virtual void disconnect() = 0;
    /// @brief Executes a simple SQL command, returned data is not retrieved
    virtual void executeSQL(const std::string& sql) = 0;

    /// @brief Create a bound statement, to which variables can be bound
    virtual void prepareBindStatement(const std::string& statement) = 0;
    /// @brief Prepare a transaction (for fast insertion) for a bound statement
    virtual void beginBindTransaction() = 0;
    /// @brief Execute the transaction and clear bound variables
    virtual void stepAndClearBindings() = 0;
    /// @brief Commit the transaction.
    virtual void endBindTransaction() = 0;
    /// @brief Clear the bound statement for reuse
    virtual void finalizeBindStatement() = 0;

    /// @brief Bind a int variable at index to a value
    virtual void bindVariable(unsigned int index, int value) = 0;
    /// @brief Bind a double variable at index to a value
    virtual void bindVariable(unsigned int index, double value) = 0;
    /// @brief Bind a string variable at index to a value
    virtual void bindVariable(unsigned int index, const std::string& value) = 0;
    /// @brief Bind a variable to the NULL value
    virtual void bindVariableNull(unsigned int index) = 0;

    /// @brief Executes a database query where data can be returned
    virtual std::shared_ptr<DBResult> execute(const DBCommand& command) = 0;
    /// @brief Executes a number of database queries where data (of the same structure) can be
    /// returned
    virtual std::shared_ptr<DBResult> execute(const DBCommandList& command_list) = 0;

    /// @brief Prepare a database query for incremental data retrieval of the result
    virtual void prepareCommand(std::shared_ptr<DBCommand> command) = 0;
    /// @brief Step through a prepared query and return a number of results
    virtual std::shared_ptr<DBResult> stepPreparedCommand(unsigned int max_results = 0) = 0;
    /// @brief Finalize the prepared query
    virtual void finalizeCommand() = 0;
    /// @brief Returns if all data from the prepared command was read
    virtual bool getPreparedCommandDone() = 0;

    virtual std::map<std::string, DBTableInfo> getTableInfo() = 0;
    virtual std::vector<std::string> getDatabases() = 0;

    /// @brief Return the DBConnectionInfo defining the database system and parameters

    virtual QWidget* widget() = 0;
    // virtual void deleteWidget ()=0;
    virtual QWidget* infoWidget() = 0;

    virtual std::string status() const = 0;
    virtual std::string identifier() const = 0;
    virtual std::string type() const = 0;

    bool ready() { return connection_ready_; }

  protected:
    /// Defines the database system and parameters
    bool connection_ready_;

    /// @brief Creates a prepared query (internal)
    virtual void prepareStatement(const std::string& sql) = 0;
    /// @brief Finalizes a prepared query (internal)
    virtual void finalizeStatement() = 0;

    virtual void checkSubConfigurables() {}

    /// @brief Executes an SQL command which returns data (internal)
    // void execute (std::string command, Buffer *buffer);

    /// @brief Executes an SQL command which no returns data (internal)
    // void execute (std::string command);
};

#endif /* DBCONNECTION_H_ */
