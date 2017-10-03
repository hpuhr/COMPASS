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
#include <memory>
#include <qobject.h>

#include "configurable.h"
#include "propertylist.h"
#include "dbovariableset.h"
#include "sqlgenerator.h"

static const std::string ACTIVE_DATA_SOURCES_PROPERTY_PREFIX="activeDataSources_";
static const std::string TABLE_NAME_PROPERTIES = "atsdb_properties";
static const std::string TABLE_NAME_MINMAX = "atsdb_minmax";

class ATSDB;
class Buffer;
class BufferWriter;
class DBConnection;
class DBOVariable;
class DBTable;
class QProgressDialog;
class DBObject;
class DBODataSource;
class DBResult;
class DBTableColumn;
class DBTableInfo;
class DBInterfaceWidget;
class DBInterfaceInfoWidget;
class Job;
class BufferWriter;

class SQLGenerator;
class QWidget;
class QProgressDialog;

/**
 * @brief Encapsulates all dedicated database functionality
 *
 * After instantiation, initConnection has to be called to create a database connection. After this step, a number of functions
 * provide read/write/other methods. Simply delete to close database connection.
 *
 * \todo Context reference point gets lost
 */
class DBInterface : public QObject, public Configurable
{
    Q_OBJECT
signals:
    void databaseOpenedSignal ();
    void postProcessingDoneSignal ();

public slots:
    void postProcessingJobDoneSlot();
    //void updateDBObjectInformationSlot ();

public:
    /// @brief Constructor
    DBInterface(std::string class_id, std::string instance_id, ATSDB *atsdb);
    /// @brief Destructor
    virtual ~DBInterface();

    const std::map<std::string, DBConnection*> &connections () { return connections_; }

    const std::string &usedConnection () { return used_connection_; }
    /// @brief Initializes a database connection based on the supplied type
    void useConnection (const std::string &connection_type);
    void databaseOpened ();
    void closeConnection ();

    void updateTableInfo ();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    std::vector <std::string> getDatabases ();

    DBInterfaceWidget *widget();
    DBInterfaceInfoWidget *infoWidget();

    QWidget *connectionWidget ();

    const std::map <std::string, DBTableInfo> &tableInfo () { return table_info_; }

    bool ready ();

    DBConnection &connection ();

    /// @brief Returns a container with all data sources for a DBO
    std::map <int, DBODataSource> getDataSources (const DBObject &object);
    bool hasActiveDataSources (const DBObject &object);
    /// @brief Returns a set with all active data source ids for a DBO type
    std::set<int> getActiveDataSources (const DBObject &object);

    //    /// @brief Writes a buffer to the database, into a table defined by write_table_names_ and DBO type
    //    void writeBuffer (Buffer *data);
    //    void writeBuffer (Buffer *data, std::string table_name);
    void updateBuffer (DBObject &object, DBOVariable &key_var, std::shared_ptr<Buffer> buffer);

    //    /// @brief Prepares incremental read of DBO type
    void prepareRead (const DBObject &dbobject, DBOVariableSet read_list, std::string custom_filter_clause, std::vector <DBOVariable *> filtered_variables,
                      bool use_order=false, DBOVariable *order_variable=nullptr, bool use_order_ascending=false, const std::string &limit="");
    /// @brief Returns data chunk of DBO type
    std::shared_ptr <Buffer> readDataChunk (const DBObject &dbobject, bool activate_key_search);
    /// @brief Cleans up incremental read of DBO type
    void finalizeReadStatement (const DBObject &dbobject);
    /// @brief Sets reading_done_ flags
    //void clearResult ();

    /// @brief Returns number of rows for a database table
    size_t count (const std::string &table);
    //    DBResult *count (const std::string &dbo_type, unsigned int sensor_number);

    /// @brief Returns if properties table exists
    bool existsPropertiesTable ();
    /// @brief Creates the properties table
    void createPropertiesTable ();
    /// @brief Inserts a property
    void setProperty (const std::string& id, const std::string& value);
    /// @brief Returns a property
    std::string getProperty (const std::string& id);
    bool hasProperty (const std::string& id);

    /// @brief Returns if minimum/maximum table exists
    bool existsMinMaxTable ();
    /// @brief Returns the minimum/maximum table
    void createMinMaxTable ();
    /// @brief Returns buffer with the minimum/maximum of a DBO variable
    std::pair<std::string, std::string> getMinMaxString (const DBOVariable& var);
    /// (dbo type, id) -> (min, max)
    std::map <std::pair<std::string, std::string>, std::pair<std::string, std::string> > getMinMaxInfo ();
    /// @brief Inserts a minimum/maximum value pair
    void insertMinMax (const std::string& id, const std::string& object_name, const std::string& min, const std::string& max);

    /// @brief Returns if database was post processed
    bool isPostProcessed ();
    void postProcess ();

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

    /// @brief Deletes table content for given table name
    void clearTableContent (const std::string& table_name);

    /// @brief Returns minimum/maximum information for all columns in a table
    std::shared_ptr<DBResult> queryMinMaxNormalForTable (const DBTable& table);
    //    /// @brief Returns minimum/maximum information for a given column in a table
    //    DBResult *queryMinMaxForColumn (DBTableColumn *column, std::string table);

    //    DBResult *getDistinctStatistics (const std::string &dbo_type, DBOVariable *variable, unsigned int sensor_number);

    /// @brief Executes query and returns numbers for all active sensors
    std::set<int> queryActiveSensorNumbers (const DBObject &object);

    //    void deleteAllRowsWithVariableValue (DBOVariable *variable, std::string value, std::string filter);
    //    void updateAllRowsWithVariableValue (DBOVariable *variable, std::string value, std::string new_value, std::string filter);

    //    void getMinMaxOfVariable (DBOVariable *variable, std::string filter_condition, std::string &min, std::string &max);
    ////    void getDistinctValues (DBOVariable *variable, std::string filter_condition, std::vector<std::string> &values);

    //    Buffer *getTrackMatches (bool has_mode_a, unsigned int mode_a, bool has_ta, unsigned int ta, bool has_ti, std::string ti,
    //            bool has_tod, double tod_min, double tod_max);

protected:
    std::map <std::string, DBConnection*> connections_;

    std::string used_connection_;
    /// Connection to database, created based on DBConnectionInfo
    DBConnection *current_connection_;

    /// Interface initialized (after opening database)
    bool initialized_;

    /// Protects the database
    boost::mutex connection_mutex_;

    /// Container with all table names, based on DBO type
    // TODO solve this
    //std::map <std::string, std::string> write_table_names_;
    /// Size of a read chunk in incremental reading process
    unsigned int read_chunk_size_;

    /// Generates SQL statements
    SQLGenerator sql_generator_;

    /// Writes buffer to the database
    BufferWriter *buffer_writer_{nullptr};

    DBInterfaceWidget *widget_;
    DBInterfaceInfoWidget *info_widget_;

    std::map <std::string, DBTableInfo> table_info_;

    std::vector <std::shared_ptr<Job>> postprocess_jobs_;
    QProgressDialog* postprocess_dialog_ {nullptr};
    unsigned int postprocess_job_num_{0};

    virtual void checkSubConfigurables ();

    void setPostProcessed (bool value);
    //    /// @brief Returns buffer with min/max data from another Buffer with the string contents. Delete returned buffer yourself.
    //    Buffer *createFromMinMaxStringBuffer (Buffer *string_buffer, PropertyDataType data_type);
};

#endif /* SQLITE3CONNECTION_H_ */
