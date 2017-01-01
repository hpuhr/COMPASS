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
 * DBInterface.h
 *
 *  Created on: Feb 1, 2012
 *      Author: sk
 */

#ifndef DBINTERFACE_H_
#define DBINTERFACE_H_

#include <boost/thread/mutex.hpp>
#include <set>
#include "Configurable.h"
#include "PropertyList.h"
#include "DBOVariableSet.h"

class Buffer;
class BufferWriter;
class DBConnection;
class DBConnectionInfo;
class DBOVariable;
class QProgressDialog;
class DBResult;
class DBTableColumn;

class SQLGenerator;

/**
 * @brief Encapsulates all dedicated database functionality
 *
 * After instantiation, initConnection has to be called to create a database connection. After this step, a number of functions
 * provide read/write/other methods. Simply delete to close database connection.
 *
 * \todo Context reference point gets lost
 */
class DBInterface : public Configurable
{
public:
    /// @brief Constructor
    DBInterface();
    /// @brief Destructor
    virtual ~DBInterface();

    /// @brief Initializes a database connection based on the supplied type
    void initConnection (DBConnectionInfo *info);
    void openDatabase (std::string database_name);

    /// @brief Returns a buffer with all data sources for a DBO type
//    Buffer *getDataSourceDescription (const std::string &dbo_type);
//    bool hasActiveDataSourcesInfo (const std::string &dbo_type);
//    /// @brief Returns a set with all active data source ids for a DBO type
//    std::set<int> getActiveSensorNumbers (const std::string &dbo_type);

//    /// @brief Writes a buffer to the database, into a table defined by write_table_names_ and DBO type
//    void writeBuffer (Buffer *data);
//    void writeBuffer (Buffer *data, std::string table_name);
//    void updateBuffer (Buffer *data);

//    /// @brief Prepares incremental read of DBO type
//    void prepareRead (const std::string &dbo_type, DBOVariableSet read_list, std::string custom_filter_clause="",
//            DBOVariable *order=0);
//    /// @brief Returns data chunk of DBO type
//    Buffer *readDataChunk (const std::string &dbo_type, bool activate_key_search);
//    /// @brief Cleans up incremental read of DBO type
//    void finalizeReadStatement (const std::string &dbo_type);
//    /// @brief Sets reading_done_ flags
//    void clearResult ();

//    /// @brief Returns if incremental read for DBO type was prepared
//    bool isPrepared (const std::string &dbo_type);
//    /// @brief Returns if incremental read for DBO type was finished
//    bool getReadingDone (const std::string &dbo_type);
//    /// @brief Returns if all incremental reads were finished
//    bool isReadingDone ();
//    /// @brief Returns if DBO exists and has data in the database
//    bool exists (const std::string &dbo_type);
//    /// @brief Returns number of elements for DBO type
//    unsigned int count (const std::string &dbo_type);
//    DBResult *count (const std::string &dbo_type, unsigned int sensor_number);

//    /// @brief Creates the properties table
//    void createPropertiesTable ();
//    /// @brief Inserts a property
//    void insertProperty (std::string id, std::string value);
//    /// @brief Returns a property
//    std::string getProperty (std::string id);
//    bool hasProperty (std::string id);

//    /// @brief Returns the minimum/maximum table
//    void createMinMaxTable ();
//    /// @brief Returns buffer with the minimum/maximum of a DBO variable
//    Buffer *getMinMaxString (DBOVariable *var);
//    /// (dbo type, id) -> (min, max)
//    std::map <std::pair<std::string, std::string>, std::pair<std::string, std::string> > getMinMaxInfo ();
//    /// @brief Inserts a minimum/maximum value pair
//    void insertMinMax (std::string id, const std::string &dbo_type, std::string min, std::string max);

//    /// @brief Returns if database was post processed
//    bool isPostProcessed ();
//    /// @brief Post-processes the database
//    //void postProcess ();

//    /// @brief Returns variable values for a number of DBO type elements
//    Buffer *getInfo (const std::string &dbo_type, std::vector<unsigned int> ids, DBOVariableSet read_list, bool use_filters,
//            std::string order_by_variable, bool ascending, unsigned int limit_min=0, unsigned int limit_max=0,
//            bool finalize=0);

//    /// @brief Sets the context reference point as property
//    void setContextReferencePoint (bool defined, float latitude, float longitude);
//    /// @brief Returns if context reference point is defined
//    bool getContextReferencePointDefined ();
//    /// @brief Returns the context reference point
//    std::pair<float, float> getContextReferencePoint ();

//    /// @brief Returns the used DBConnectionInfo
//    DBConnectionInfo *getDBInfo () { assert (info_); return info_;}

//    /// @brief Returns a buffer containing all existing table names
//    Buffer *getTableList();
//    /// @brief Returns a buffer containing all column names for a given table name
//    Buffer *getColumnList(std::string table);

//    /// @brief Prints database schema information (for debugging purposes)
//    void printDBSchema ();

//    /// @brief Returns if minimum/maximum table exists
//    bool existsMinMaxTable ();
//    /// @brief Returns if properties table exists
//    bool existsPropertiesTable ();

//    /// @brief Deletes table content for given table name
//    void clearTableContent (std::string table_name);

//    /// @brief Updates the exists_ container
//    void updateExists ();
//    /// @brief Update the count_ container
//    void updateCount ();

//    /// @brief Returns the SQLGenerator
//    SQLGenerator *getSQLGenerator () { assert (sql_generator_); return sql_generator_; }

//    /// @brief Returns minimum/maximum information for all columns in a table
//    DBResult *queryMinMaxNormalForTable (std::string table);
//    /// @brief Returns minimum/maximum information for a given column in a table
//    DBResult *queryMinMaxForColumn (DBTableColumn *column, std::string table);

//    DBResult *getDistinctStatistics (const std::string &dbo_type, DBOVariable *variable, unsigned int sensor_number);

//    /// @brief Executes query and returns numbers for all active sensors
//    std::set<int> queryActiveSensorNumbers (const std::string &dbo_type);

//    void deleteAllRowsWithVariableValue (DBOVariable *variable, std::string value, std::string filter);
//    void updateAllRowsWithVariableValue (DBOVariable *variable, std::string value, std::string new_value, std::string filter);

//    void getMinMaxOfVariable (DBOVariable *variable, std::string filter_condition, std::string &min, std::string &max);
////    void getDistinctValues (DBOVariable *variable, std::string filter_condition, std::vector<std::string> &values);

//    Buffer *getTrackMatches (bool has_mode_a, unsigned int mode_a, bool has_ta, unsigned int ta, bool has_ti, std::string ti,
//            bool has_tod, double tod_min, double tod_max);

//    std::vector <std::string> getDatabases ();
private:
    /// Last used database name
    //std::string database_name_;
    /// Connection exists
    bool connected_;
    /// Database opened
    bool database_opened_;
    /// Container with all prepared flags (for incremental reading)
//    std::map <std::string, bool> prepared_;
//    /// Container with all reading done flags (for incremental reading)
//    std::map <std::string, bool> reading_done_;
//    /// Container with all exists flags, indicating if DBO has data in the database
//    std::map <std::string, bool> exists_;
//    /// Container with all DBO element counts
//    std::map <std::string, unsigned int> count_;

    /// Protects the database
    boost::mutex mutex_;

    /// Container with all table names, based on DBO type
    // TODO solve this
    //std::map <std::string, std::string> write_table_names_;
    /// Size of a read chunk in incremental reading process
    unsigned int read_chunk_size_;

    /// Definition of used database system and parameters
    DBConnectionInfo *info_;
    /// Generates SQL statements
    //SQLGenerator *sql_generator_;
    /// Connection to database, created based on DBConnectionInfo
    DBConnection *connection_;
    /// Writes buffer to the database
    //BufferWriter *buffer_writer_;

    /// @brief Returns if a defined table name exists
//    bool existsTable (std::string table_name);

//    /// @brief Queries if DBO has data in database
//    bool queryContains (const std::string &dbo_type);
//    /// @brief Returns element count for DBO
//    unsigned int queryCount (const std::string &dbo_type);

//    /// @brief Returns buffer with min/max data from another Buffer with the string contents. Delete returned buffer yourself.
//    Buffer *createFromMinMaxStringBuffer (Buffer *string_buffer, PropertyDataType data_type);
};

#endif /* SQLITE3CONNECTION_H_ */
