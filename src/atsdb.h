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
 * ATSDB.h
 *
 *  Created on: Jul 12, 2011
 *      Author: sk
 */

#ifndef ATSDB_H_
#define ATSDB_H_

#include "propertylist.h"
#include "singleton.h"
#include "joborderer.h"
#include "dbovariableset.h"
#include "configurable.h"

#include <boost/thread/mutex.hpp>
#include <boost/function.hpp>
#include <set>
#include <map>
#include <vector>

class Buffer;
class DataSource;
class DBInterface;
class DBTableInfo;
class DBObject;
class DBObjectManager;
class DBSchema;
class DBSchemaManager;
//class StructureReader;
class DBOVariable;
class Job;
//class DBOReadDBJob;
//class FinalizeDBOReadJob;
class BufferReceiver;

/**
 * @mainpage  ATSDB Main Page
 *
 * This library encapsulates a database system and allows reading and writing of flight surveillance data.
 *
 * The main access point is the class ATSDB. Using this singleton, surveillance data can be written using the RDL
 * tool and SQLite3 file containers or a MySQL database. Such databases can be opened using the tool Palantir.
 *
 * Most of the classes were written to be persistent, meaning that local parameters can saved/restored using a sophisticated
 * Configuration framework. Also, there exists a mechanism for dynamic creation of instances based on such a Configuration,
 * which is embedded in the class Configurable. The main access point for this framework is the class ConfigurationManager.
 *
 * Based on the this configuration, an abstract representation of objects stored in the database was created, which should
 * largely be independent of the database schema. Such an object is called DBObject, all of which are managed by the DBObjectManager.
 *
 * Also based on the configuration is the database filtering system. The FilterManager is used to manage filters using the DBFilter
 * class, which itself holds filter conditions based on the DBFilterCondition class. For each DBObject, the current conditions in the
 * SQL query can be retrieved from the active filters.
 *
 * The most common data storage container is a Buffer, a dynamic and fast mechanism for data storage and retrieval. It is based on
 * memory pages based on the ArrayTemplate in interplay with the ArrayTemplateManager, and the MemoryManager which is the main
 * access point for memory allocation and management.
 * <p/> <br/>
 */

/**
 * @brief Main access point for all library function.
 *
 * @details Singleton, uses separate thread. Is started using the init function (with a connection type specifying the database
 * system and parameters). Has functionality which generates and allows access to sensor information of DBObjects. Allows reading/writing
 * data from/to the database and various access function. Can be stopped using the shutdown function.
 *
 * Note the following example code:
 * @code
 *  DBConnectionInfo *info = ...;
 *  ATSDB::getInstance().init (info);
 *  // do stuff
 *  ATSDB::getInstance().shutdown();
 * @endcode
 *
 * \todo Change sorting if export is active
 * \todo Maybe extend some classes to observer pattern
 * \todo Removed writing of new databases. Re-integrated if necessary
 */
class ATSDB : public Configurable, public Singleton
{
public:
    ///@brief Destructor.
    virtual ~ATSDB();

    void initialize ();

    ///@brief Shuts down the DB access.
    void shutdown ();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    DBInterface &interface ();
    DBSchemaManager &schemaManager ();
    DBObjectManager &objectManager ();

    bool ready ();

    ///@brief Adds data to a DBO from a C struct data pointer.
    //void insert (const std::string &dbo_type, void *data);
    //void insert (Buffer *buffer, std::string table_name);
    ///@brief Updates data records for a DBObject, delete buffer after execution
    //void update (Buffer *data);

    ///@brief Clears previous result and start reading of data for all existing DBOs.
    //void startReading (BufferReceiver *receiver, DBOVariableSet read_list);
    //void startReading (BufferReceiver *receiver, const std::string &dbo_type, DBOVariableSet read_list, std::string custom_filter_clause,
    //        DBOVariable *order);
    ///@brief Returns flag indicating if reading of data is done.
    //bool isReadingDone ();

    ///@brief Returns container with all defined data sources, DBO type -> data sources
//    std::map <std::string, std::map<int, std::string> > &getAllDataSources () { return data_sources_; }
//    std::map <std::pair<unsigned char, unsigned char>, DataSource* > &getDataSourceInstances (const std::string &dbo_type)
//            { assert (data_sources_instances_.find(dbo_type) != data_sources_instances_.end()); return data_sources_instances_[dbo_type]; }

    ///@brief Returns flag if data sources are defined for DBO type.
    //bool hasDataSources (const std::string &dbo_type);
    ///@brief Returns container with all defined data source for DBO type.
    //const std::map<int, std::string> &getDataSources (const std::string &dbo_type);
    ///@brief Returns data source name for a DBO type and data source number.
    //std::string getNameOfSensor (const std::string &dbo_type, unsigned int num);
    /// @brief Return if a DBObject has information about its active data sources
    //bool hasActiveDataSourcesInfo (const std::string &dbo_type);
    /// @brief Creates information about a DBObject's active data sources
    //void buildActiveDataSourcesInfo (const std::string &dbo_type);

    ///@brief Returns flag indicating if error state was set.
    //bool error();

    //void buildMinMaxInfo (DBOVariable *var);
    ///@brief Returns string minimum for DBO variable.
    //std::string getMinAsString (DBOVariable *var);
    ///@brief Returns string maximum for DBO variable.
    //std::string getMaxAsString (DBOVariable *var);

    ///@brief Returns buffer with data from a DBO type, from a specific id and variables.
//    void getInfo (JobOrderer *orderer, boost::function<void (Job*)> done_function,
//            boost::function<void (Job*)> obsolete_function, const std::string &dbo_type, unsigned int id, DBOVariableSet read_list);
//    ///@brief Returns buffer with data from a DBO type, from specific ids and other parameters.
//    void getInfo (JobOrderer *orderer, boost::function<void (Job*)> done_function,
//            boost::function<void (Job*)> obsolete_function, const std::string &dbo_type,
//            std::vector<unsigned int> ids, DBOVariableSet read_list, bool use_filters, std::string order_by_variable,
//            bool ascending, unsigned int limit_min=0, unsigned int limit_max=0, bool finalize=true);

//    void getCount (JobOrderer *orderer, boost::function<void (Job*)> done_function,
//            boost::function<void (Job*)> obsolete_function, const std::string &dbo_type, unsigned int sensor_number);

//    void getDistinctStatistics (JobOrderer *orderer, boost::function<void (Job*)> done_function,
//            boost::function<void (Job*)> obsolete_function, const std::string &dbo_type, DBOVariable *variable,
//            unsigned int sensor_number);

//    ///@brief Starts the post-processing.
    //void postProcess (JobOrderer *orderer, boost::function<void(Job*)> done, boost::function<void(Job*)> obsolete);

    ///@brief Sets if data gets exported
    //void setExportActive (bool active);
    ///@brief Returns if data gets exported
    //bool getExportActive ();

    /// @brief Callback function for post-processing job done
//    void postProcessingDone( Job *job );
//    void activeDataSourcesDone( Job *job );
//    void minMaxDone( Job *job );
//    /// @brief Callback function for main reading job done
//    void readDBODone( Job *job );
//    /// @brief Callback function for main reading job done
//    void readDBOIntermediate( Job *job, Buffer *buffer );
//    /// @brief Callback function for finalizing of main reading job done
//    void finalizeReadDBODone( Job *job );

//    /// @brief Callback function for update job done
//    void updateDBODone ( Job *job );
//    void updateDBOAborted ( Job *job );

//    void abortFinalizeReadDBODone( Job *job );
//    /// @brief Callback function for aborted jobs
//    void jobAborted( Job *job );

//    void deleteAllRowsWithVariableValue (DBOVariable *variable, std::string value, std::string filter);
//    void updateAllRowsWithVariableValue (DBOVariable *variable, std::string value, std::string new_value, std::string filter);
//    void getMinMaxOfVariable (DBOVariable *variable, std::string filter_condition, std::string &min, std::string &max);
//    //void getDistinctValues (DBOVariable *variable, std::string filter_condition, std::vector<std::string> &values);

//    // Parameters in decimal, return buffer with track_num, min(tod), max(tod)
//    Buffer *getTrackMatches (bool has_mode_a, unsigned int mode_a, bool has_ta, unsigned int ta, bool has_ti, std::string ti,
//            bool has_tod, double tod_min, double tod_max);


protected:
    bool initialized_;

    /// Flag indicating if data gets exported
    //bool export_active_;

    /// DB interface, encapsulating all database functionality.
    DBInterface *db_interface_;
    DBObjectManager *dbo_manager_;
    DBSchemaManager *db_schema_manager_;
    /// Structure reader, can read data from defined C structs.
    //StructureReader *struct_reader_;
    /// Map containg all existing data sources (DBO type -> data source number, data source name).
//    std::map <std::string, std::map<int, std::string> > data_sources_;
//    std::map <std::string, std::map <std::pair<unsigned char, unsigned char>, DataSource* > > data_sources_instances_;

    /// Container with all current read jobs (one per loaded DBObject)
    //std::vector <DBOReadDBJob*> read_jobs_;
//    boost::mutex read_jobs_mutex_;
//    std::map <DBOReadDBJob*, BufferReceiver*> dbo_read_jobs_;
//    boost::mutex finalize_jobs_mutex_;
//    std::map <FinalizeDBOReadJob*, BufferReceiver*> dbo_finalize_jobs_;

    //std::vector <std::string> databases_;

    virtual void checkSubConfigurables ();

    ///@brief Quits all reading processes.
    //void quitReading ();

    //void buildDatabases ();

    ///@brief Generates minimum and maximum information from previous post-processing.
    //void loadMinMaxValues ();
    ///@brief Generates data sources information from previous post-processing.
    //void buildDataSources();
    ///@brief Generates active data sources information from previous post-processing.
    //void loadActiveDataSources ();

//    void testUpdate ();
//    void setJobsObsoleteForBufferReceiver (BufferReceiver *receiver);

    ///@brief Constructor.
    ATSDB();

public:
    ///@brief Instance access function for Singleton.
    static ATSDB& instance()
    {
        static ATSDB instance;
        return instance;
    }
    //unsigned int getNumJobsForBufferReceiver (BufferReceiver *receiver);
};

#endif /* ATSDB_H */
