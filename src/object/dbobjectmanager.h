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

#ifndef DBOBJECTMANAGER_H_
#define DBOBJECTMANAGER_H_

#include "configurable.h"
#include "dbcontentconfigurationdatasource.h"
#include "dbcontentdbdatasource.h"
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
class MetaDBOVariableConfigurationDialog;
class DBContentVariable;
class MetaDBOVariable;
class DBContentVariableSet;
class DBSchemaManager;

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

    bool existsObject(const std::string& dbo_name);
    DBContent& object(const std::string& dbo_name);
    void deleteObject(const std::string& dbo_name);
    bool hasData();

    using DBObjectIterator = typename std::map<std::string, DBContent*>::iterator;
    DBObjectIterator begin() { return objects_.begin(); }
    DBObjectIterator end() { return objects_.end(); }
    size_t size() { return objects_.size(); }

    bool existsMetaVariable(const std::string& var_name);
    MetaDBOVariable& metaVariable(const std::string& var_name);
    void renameMetaVariable(const std::string& old_var_name, const std::string& new_var_name);
    void deleteMetaVariable(const std::string& var_name);
    const std::vector<std::unique_ptr<MetaDBOVariable>>& metaVariables() { return meta_variables_; }

    bool usedInMetaVariable(const DBContentVariable& variable);
    MetaDBOVariableConfigurationDialog* metaVariableConfigdialog();


    bool hasConfigDataSource(unsigned int ds_id);
    ConfigurationDataSource& configDataSource(unsigned int ds_id);

    bool hasDataSource(unsigned int ds_id);
    bool canAddNewDataSourceFromConfig (unsigned int ds_id);
    void addNewDataSource (unsigned int ds_id); // be sure not to call from different thread
    DBDataSource& dataSource(unsigned int ds_id);
    const std::vector<std::unique_ptr<DBDataSource>>& dataSources() const;

    std::map<unsigned int, std::vector <std::pair<std::string, unsigned int>>> getNetworkLines(); //ds_id -> (ip, port)

    void saveDBDataSources();

    bool loadingWanted (const std::string& dbo_name);
    bool hasDSFilter (const std::string& dbo_name);
    std::vector<unsigned int> unfilteredDS (const std::string& dbo_name); // DS IDs

    void load();
    void addLoadedData(std::map<std::string, std::shared_ptr<Buffer>> data);
    void loadingDone(DBContent& object); // to be called by dbo when it's loading is finished
    bool loadInProgress() const;

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
    DBContentVariable& orderVariable();
    void orderVariable(DBContentVariable& variable);
    bool hasOrderMetaVariable();
    MetaDBOVariable& orderMetaVariable();
    void orderMetaVariable(MetaDBOVariable& variable);
    void clearOrderVariable();

    void quitLoading();

    bool hasAssociations() const;
    void setAssociationsDataSource(const std::string& dbo, const std::string& data_source_name);
    void setAssociationsByAll();
    void removeAssociations();

    bool hasAssociationsDataSource() const;
    std::string associationsDBObject() const;
    std::string associationsDataSourceName() const;

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
    std::string order_variable_dbo_name_;
    std::string order_variable_name_;

    bool use_limit_{false};
    unsigned int limit_min_{0};
    unsigned int limit_max_{100000};

    bool has_associations_{false};
    std::string associations_dbo_;
    std::string associations_ds_;

    bool has_max_rec_num_ {false};
    unsigned int max_rec_num_ {0};

    std::map<std::string, std::shared_ptr<Buffer>> data_;

    std::map<std::string, std::shared_ptr<Buffer>> insert_data_;

    bool load_in_progress_{false};
    bool insert_in_progress_{false};

    /// Container with all DBOs (DBO name -> DBO pointer)
    std::map<std::string, DBContent*> objects_;
    std::vector<std::unique_ptr<MetaDBOVariable>> meta_variables_;

    std::vector<std::unique_ptr<ConfigurationDataSource>> config_data_sources_;
    std::vector<std::unique_ptr<DBDataSource>> db_data_sources_;

    std::unique_ptr<DBContentManagerWidget> widget_;
    std::unique_ptr<DBContentManagerDataSourcesWidget> load_widget_;

    std::unique_ptr<MetaDBOVariableConfigurationDialog> meta_cfg_dialog_;

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

#endif /* DBOBJECTMANAGER_H_ */
