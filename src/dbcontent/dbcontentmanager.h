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

#ifndef DBCONTENT_DBCONTENTMANAGER_H_
#define DBCONTENT_DBCONTENTMANAGER_H_

#include "configurable.h"
#include "dbcontent/source/configurationdatasource.h"
#include "dbcontent/source/dbdatasource.h"
#include "global.h"
#include "singleton.h"
#include "buffer.h"

#include <qobject.h>

#include <vector>
#include <memory>

class COMPASS;
class DBContent;
class DBContentManagerWidget;
class DBContentManagerDataSourcesWidget;
class DBSchemaManager;

namespace dbContent {

class MetaVariableConfigurationDialog;
class Variable;
class MetaVariable;
class VariableSet;

}

class DBContentManager : public QObject, public Configurable
{
    Q_OBJECT

public slots:

    void databaseOpenedSlot();
    //void databaseContentChangedSlot();
    void databaseClosedSlot();

    void metaDialogOKSlot();

signals:
    void dbObjectsChangedSignal();

    void loadingStartedSignal(); // emitted when load has been started
    // all data contained, also new one. requires_reset true indicates that all shown info should be re-created,
    // e.g. when data in the beginning was removed, or order of previously emitted data was changed, etc.
    void loadedDataSignal (const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset);
    void loadingDoneSignal(); // emitted when all dbos have finished loading
    void insertDoneSignal(); // emitted when all dbos have finished loading

public:
    const static std::vector<std::string> data_source_types_;

    DBContentManager(const std::string& class_id, const std::string& instance_id, COMPASS* compass);
    virtual ~DBContentManager();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    bool existsDBContent(const std::string& dbcontent_name);
    DBContent& dbContent(const std::string& dbcontent_name);
    void deleteDBContent(const std::string& dbcontent_name);
    bool hasData();

    using DBObjectIterator = typename std::map<std::string, DBContent*>::iterator;
    DBObjectIterator begin() { return dbcontent_.begin(); }
    DBObjectIterator end() { return dbcontent_.end(); }
    size_t size() { return dbcontent_.size(); }

    bool existsMetaVariable(const std::string& var_name);
    dbContent::MetaVariable& metaVariable(const std::string& var_name);
    void renameMetaVariable(const std::string& old_var_name, const std::string& new_var_name);
    void deleteMetaVariable(const std::string& var_name);
    const std::vector<std::unique_ptr<dbContent::MetaVariable>>& metaVariables() { return meta_variables_; }

    bool usedInMetaVariable(const dbContent::Variable& variable);
    dbContent::MetaVariableConfigurationDialog* metaVariableConfigdialog();

    bool hasConfigDataSource(unsigned int ds_id);
    dbContent::ConfigurationDataSource& configDataSource(unsigned int ds_id);

    bool hasDataSource(unsigned int ds_id);
    bool hasDataSourcesOfDBContent(const std::string dbcontent_name);
    bool canAddNewDataSourceFromConfig (unsigned int ds_id);
    void addNewDataSource (unsigned int ds_id); // be sure not to call from different thread

    dbContent::DBDataSource& dataSource(unsigned int ds_id);
    const std::vector<std::unique_ptr<dbContent::DBDataSource>>& dataSources() const;

    std::map<unsigned int, std::vector <std::pair<std::string, unsigned int>>> getNetworkLines(); //ds_id -> (ip, port)

    void saveDBDataSources();

    bool loadingWanted (const std::string& dbo_name);
    bool hasDSFilter (const std::string& dbo_name);
    std::vector<unsigned int> unfilteredDS (const std::string& dbo_name); // DS IDs

    void load();
    void addLoadedData(std::map<std::string, std::shared_ptr<Buffer>> data);
    void loadingDone(DBContent& object); // to be called by dbo when it's loading is finished
    bool loadInProgress() const;
    void clearData();

    void insertData(std::map<std::string, std::shared_ptr<Buffer>> data);
    void insertDone(DBContent& object); // to be called by dbo when it's insert is finished
    bool insertInProgress() const;

    DBContentManagerWidget* widget();
    DBContentManagerDataSourcesWidget* loadWidget();

    bool useLimit() const;
    void useLimit(bool useLimit);

    unsigned int limitMin() const;
    void limitMin(unsigned int limitMin);

    unsigned int limitMax() const;
    void limitMax(unsigned int limitMax);

    bool useOrder() const;
    void useOrder(bool useOrder);

    bool useOrderAscending() const;
    void useOrderAscending(bool useOrderAscending);

    bool hasOrderVariable();
    dbContent::Variable& orderVariable();
    void orderVariable(dbContent::Variable& variable);
    bool hasOrderMetaVariable();
    dbContent::MetaVariable& orderMetaVariable();
    void orderMetaVariable(dbContent::MetaVariable& variable);
    void clearOrderVariable();

    void quitLoading();

    bool hasAssociations() const;
    void setAssociationsIdentifier(const std::string& assoc_id);
//    void setAssociationsByAll();
//    void removeAssociations();

//    bool hasAssociationsDataSource() const;
    std::string associationsID() const;
//    std::string associationsDataSourceName() const;

    bool isOtherDBObjectPostProcessing(DBContent& object);

    bool hasMaxRecordNumber() const { return has_max_rec_num_; }
    unsigned int maxRecordNumber() const;
    void maxRecordNumber(unsigned int value);

    const std::map<std::string, std::shared_ptr<Buffer>>& data() const;

    void dsTypeLoadingWanted (const std::string& ds_type, bool wanted);
    bool dsTypeLoadingWanted (const std::string& ds_type);

protected:
    COMPASS& compass_;

    std::map<std::string, bool> ds_type_loading_wanted_;

    bool use_order_{false};
    bool use_order_ascending_{false};
    std::string order_variable_dbcontent_name_;
    std::string order_variable_name_;

    bool use_limit_{false};
    unsigned int limit_min_{0};
    unsigned int limit_max_{100000};

    bool has_associations_{false};
    std::string associations_id_;

    bool has_max_rec_num_ {false};
    unsigned int max_rec_num_ {0};

    std::map<std::string, std::shared_ptr<Buffer>> data_;

    std::map<std::string, std::shared_ptr<Buffer>> insert_data_;

    bool load_in_progress_{false};
    bool insert_in_progress_{false};

    /// Container with all DBOs (DBO name -> DBO pointer)
    std::map<std::string, DBContent*> dbcontent_;
    std::vector<std::unique_ptr<dbContent::MetaVariable>> meta_variables_;

    std::vector<std::unique_ptr<dbContent::ConfigurationDataSource>> config_data_sources_;
    std::vector<std::unique_ptr<dbContent::DBDataSource>> db_data_sources_;

    std::unique_ptr<DBContentManagerWidget> widget_;
    std::unique_ptr<DBContentManagerDataSourcesWidget> load_widget_;

    std::unique_ptr<dbContent::MetaVariableConfigurationDialog> meta_cfg_dialog_;

    virtual void checkSubConfigurables();
    void finishLoading();
    void finishInserting();

    void addInsertedDataToChache();
    void filterDataSources();
    void cutCachedData();

    void loadDBDataSources();
    void sortDBDataSources();
    void loadMaxRecordNumber();
};

#endif /* DBCONTENT_DBCONTENTMANAGER_H_ */
