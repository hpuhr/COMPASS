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
#include "dbcontent/variable/variableset.h"
#include "propertylist.h"
#include "sqlgenerator.h"

#include <QObject>

#include <boost/thread/mutex.hpp>

#include <memory>
#include <set>

static const std::string TABLE_NAME_PROPERTIES = "properties";
static const std::string TABLE_NAME_SECTORS = "sectors";
static const std::string TABLE_NAME_VIEWPOINTS = "viewpoints";
static const std::string TABLE_NAME_TARGETS = "targets";

class COMPASS;
class Buffer;
class BufferWriter;
class SQLiteConnection;
class QProgressDialog;
class DBContent;
class DBResult;
class DBTableInfo;
class Job;
class BufferWriter;
class Sector;
class SectorLayer;

class SQLGenerator;
class QWidget;

namespace dbContent
{
class DBDataSource;
class Variable;
class Target;
}

class DBInterface : public QObject, public Configurable
{
    Q_OBJECT

signals:
    //void databaseOpenedSignal();
    void databaseContentChangedSignal();
    //void databaseClosedSignal();

public:
    DBInterface(std::string class_id, std::string instance_id, COMPASS* compass);
    virtual ~DBInterface();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    void openDBFile(const std::string& filename, bool overwrite);
    void closeDBFile();
    bool dbOpen();

    const std::map<std::string, DBTableInfo>& tableInfo() { return table_info_; }

    bool ready();

    SQLiteConnection& connection();

    bool existsDataSourcesTable();
    void createDataSourcesTable();
    std::vector<std::unique_ptr<dbContent::DBDataSource>> getDataSources();
    void saveDataSources(const std::vector<std::unique_ptr<dbContent::DBDataSource>>& data_sources);
    // clears previous and saves new ones

    // insert data and create associated data sources
    void insertBuffer(DBContent& dbcontent, std::shared_ptr<Buffer> buffer);
    void insertBuffer(const std::string& table_name, std::shared_ptr<Buffer> buffer);

    void updateBuffer(const std::string& table_name, const std::string& key_col, std::shared_ptr<Buffer> buffer,
                      int from_index = -1, int to_index = -1);  // no indexes means full buffer

    void prepareRead(const DBContent& dbobject, dbContent::VariableSet read_list,
                     const std::vector<std::string>& extra_from_parts,
                     std::string custom_filter_clause, std::vector<dbContent::Variable*> filtered_variables,
                     bool use_order = false, dbContent::Variable* order_variable = nullptr,
                     bool use_order_ascending = false, const std::string& limit = "");

    std::shared_ptr<Buffer> readDataChunk(const DBContent& dbobject);
    void finalizeReadStatement(const DBContent& dbobject);

    size_t count(const std::string& table);

    bool existsPropertiesTable();
    void createPropertiesTable();
    void setProperty(const std::string& id, const std::string& value);
    std::string getProperty(const std::string& id);
    bool hasProperty(const std::string& id);

    bool existsTable(const std::string& table_name);
    void createTable(const DBContent& object);

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

    bool existsTargetsTable();
    void createTargetsTable();
    void clearTargetsTable();
    std::map<unsigned int, std::shared_ptr<dbContent::Target>> loadTargets();
    void saveTargets(std::map<unsigned int, std::shared_ptr<dbContent::Target>> targets);

    void clearTableContent(const std::string& table_name);

    unsigned int getMaxRecordNumber(DBContent& object);
    unsigned int getMaxRefTrackTrackNum();

    //std::map<unsigned int, std::tuple<std::set<unsigned int>, std::tuple<bool, unsigned int, unsigned int>,
    //std::tuple<bool, unsigned int, unsigned int>>> queryADSBInfo();
    // ta -> mops versions, nucp_nics, nac_ps

protected:
    std::unique_ptr<SQLiteConnection> db_connection_;

    bool properties_loaded_ {false};

    boost::mutex connection_mutex_;
    boost::mutex table_info_mutex_;

    unsigned int read_chunk_size_;

    SQLGenerator sql_generator_;

    std::map<std::string, DBTableInfo> table_info_;

    std::map<std::string, std::string> properties_;

    virtual void checkSubConfigurables();

    void insertBindStatementUpdateForCurrentIndex(std::shared_ptr<Buffer> buffer, unsigned int buffer_index);

    void loadProperties();
    void saveProperties();

    void updateTableInfo();
};

#endif /* DBINTERFACE_H_ */
