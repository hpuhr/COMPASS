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

#ifndef DBINTERFACE_H_
#define DBINTERFACE_H_

#include "configurable.h"
#include "dboassociationcollection.h"
#include "dbovariableset.h"
#include "propertylist.h"
#include "sqlgenerator.h"

#include <qobject.h>

#include <QMutex>
#include <memory>
#include <set>

//static const std::string ACTIVE_DATA_SOURCES_PROPERTY_PREFIX = "activeDataSources_"; // TODO remove
static const std::string TABLE_NAME_PROPERTIES = "properties";
//static const std::string TABLE_NAME_MINMAX = "atsdb_minmax";
static const std::string TABLE_NAME_SECTORS = "sectors";
static const std::string TABLE_NAME_VIEWPOINTS = "viewpoints";

class COMPASS;
class Buffer;
class BufferWriter;
class SQLiteConnection;
class DBOVariable;
class QProgressDialog;
class DBObject;
class DBODataSource;
class DBResult;
class DBTableInfo;
//class DBInterfaceInfoWidget;
class Job;
class BufferWriter;
class Sector;
class SectorLayer;

class SQLGenerator;
class QWidget;

namespace DBContent
{
class DBDataSource;
}

class DBInterface : public QObject, public Configurable
{
    Q_OBJECT

signals:
    void databaseOpenedSignal();
    void databaseContentChangedSignal();
    void databaseClosedSignal();

public:
    /// @brief Constructor
    DBInterface(std::string class_id, std::string instance_id, COMPASS* compass);
    /// @brief Destructor
    virtual ~DBInterface();

    //void databaseOpenend();
    //void databaseContentChanged();


    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    std::vector<std::string> getDatabases();

    //DBInterfaceInfoWidget* infoWidget();

    //QWidget* connectionWidget();

    void openDBFile(const std::string& filename);
    void closeDBFile();
    bool dbOpen();

    const std::map<std::string, DBTableInfo>& tableInfo() { return table_info_; }

    bool ready();

    SQLiteConnection& connection();

    bool hasDataSources();
    std::vector<std::unique_ptr<DBContent::DBDataSource>> getDataSources();
    void saveDataSources(const std::vector<std::unique_ptr<DBContent::DBDataSource>>& data_sources);
    // clears previous and saves new ones

    void insertBuffer(const DBObject& dbobject, std::shared_ptr<Buffer> buffer);
    void insertBuffer(const std::string& table_name, std::shared_ptr<Buffer> buffer);

    //    bool checkUpdateBuffer(DBObject& object, DBOVariable& key_var, DBOVariableSet& list,
    //                           std::shared_ptr<Buffer> buffer);
    //    void updateBuffer(MetaDBTable& meta_table, const DBTableColumn& key_col,
    //                      std::shared_ptr<Buffer> buffer, int from_index = -1,
    //                      int to_index = -1);  // no indexes means full buffer
    void updateBuffer(const std::string& table_name, const std::string& key_col, std::shared_ptr<Buffer> buffer,
                      int from_index = -1, int to_index = -1);  // no indexes means full buffer

    void prepareRead(const DBObject& dbobject, DBOVariableSet read_list,
                     std::string custom_filter_clause, std::vector<DBOVariable*> filtered_variables,
                     bool use_order = false, DBOVariable* order_variable = nullptr,
                     bool use_order_ascending = false, const std::string& limit = "");

    std::shared_ptr<Buffer> readDataChunk(const DBObject& dbobject);
    void finalizeReadStatement(const DBObject& dbobject);

    size_t count(const std::string& table);

    bool existsPropertiesTable();
    void createPropertiesTable();
    void setProperty(const std::string& id, const std::string& value);
    std::string getProperty(const std::string& id);
    bool hasProperty(const std::string& id);

    bool existsTable(const std::string& table_name);
    void createTable(const DBObject& object);
    //    bool existsMinMaxTable();
    //    void createMinMaxTable();
    //    std::pair<std::string, std::string> getMinMaxString(const DBOVariable& var);
    //    /// (dbo type, id) -> (min, max)
    //    std::map<std::pair<std::string, std::string>, std::pair<std::string, std::string>> getMinMaxInfo();
    //    void insertMinMax(const std::string& id, const std::string& object_name, const std::string& min,
    //                      const std::string& max);

    bool areColumnsNull (const std::string& table_name, const std::vector<std::string> columns);

    bool existsViewPointsTable();
    void createViewPointsTable();
    void setViewPoint(const unsigned int id, const std::string& value);
    std::map<unsigned int, std::string> viewPoints();
    void deleteViewPoint(const unsigned int id);
    void deleteAllViewPoints();

    bool existsSectorsTable();
    void createSectorsTable();
    std::vector<std::shared_ptr<SectorLayer>> loadSectors ();
    void clearSectorsTable();
    void saveSector(std::shared_ptr<Sector> sector); // write to db and add
    void deleteSector(std::shared_ptr<Sector> sector);
    void deleteAllSectors();

    void clearTableContent(const std::string& table_name);

    std::shared_ptr<DBResult> queryMinMaxNormalForTable(const std::string& table_name);

    std::set<int> queryActiveSensorNumbers(DBObject& object);

    std::map<unsigned int, std::tuple<std::set<unsigned int>, std::tuple<bool, unsigned int, unsigned int>,
    std::tuple<bool, unsigned int, unsigned int>>> queryADSBInfo();
    // ta -> mops versions, nucp_nics, nac_ps

    //    void deleteAllRowsWithVariableValue (DBOVariable *variable, std::string value, std::string
    //    filter); void updateAllRowsWithVariableValue (DBOVariable *variable, std::string value,
    //    std::string new_value, std::string filter);

    void createAssociationsTable(const std::string& table_name);
    DBOAssociationCollection getAssociations(const std::string& table_name);

protected:
    std::unique_ptr<SQLiteConnection> db_connection_;

    bool properties_loaded_ {false};

    QMutex connection_mutex_;

    unsigned int read_chunk_size_;

    SQLGenerator sql_generator_;

    //DBInterfaceInfoWidget* info_widget_{nullptr};

    std::map<std::string, DBTableInfo> table_info_;

    std::map<std::string, std::string> properties_;

    virtual void checkSubConfigurables();

    void insertBindStatementUpdateForCurrentIndex(std::shared_ptr<Buffer> buffer, unsigned int row);

    //    /// @brief Returns buffer with min/max data from another Buffer with the string contents.
    //    Delete returned buffer yourself. Buffer *createFromMinMaxStringBuffer (Buffer
    //    *string_buffer, PropertyDataType data_type);

    void loadProperties();
    void saveProperties();

    void updateTableInfo();
    //void closeConnection();

};

#endif /* DBINTERFACE_H_ */
