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
 * BufferWriter.h
 *
 *  Created on: Jan 26, 2012
 *      Author: sk
 */

#ifndef BUFFERWRITER_H_
#define BUFFERWRITER_H_

#include <map>
#include <memory>

class DBObject;
class DBOVariable;
class Buffer;
class DBConnection;
class SQLGenerator;

/**
 * @brief Writes buffers to a SQL database table
 *
 * @details Uses a DBConnection and a SQLGenerator to create insertion commands from buffers.
 *
 * Was created for the conditions that a single writing process fills a new database.
 *
 * \todo Slightly outdated mechanism, should be generalized
 */
class BufferWriter
{
public:
    /// @brief Constructor
    BufferWriter(DBConnection *db_connection, SQLGenerator *sql_generator);
    /// @brief Destructor
    virtual ~BufferWriter();

    /// @brief Write a buffer containing data into a table
    //void write (Buffer *data, std::string tablename);
    void update (std::shared_ptr<Buffer> buffer, DBObject &object, DBOVariable &key_var, std::string tablename);

private:
    /// Database connection to write the data to
    DBConnection *db_connection_;
    /// Used to generate the SQL statements
    SQLGenerator *sql_generator_;

    /// List of already created tables
    std::vector <std::string> created_tables_;
    /// SQL statements for insertion process
    std::map <std::string, std::string> created_binds_;

    /// @brief Creates a database table with a Buffers definition and a specified name
//    void createTableForBuffer (Buffer *data, std::string tablename);
//    /// @brief Returns flag indicating if a table was already created
//    bool existsTableForBuffer (std::string tablename);

//    /// @brief Writes the data at the current index of a Buffer into the database
//    void insertBindStatementForCurrentIndex (Buffer *buffer);
    void insertBindStatementUpdateForCurrentIndex (std::shared_ptr<Buffer> buffer, unsigned int row);
};

#endif /* BUFFERWRITER_H_ */
