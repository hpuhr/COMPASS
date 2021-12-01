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


#include <qobject.h>

#include <vector>

class COMPASS;
class DBObject;
class DBObjectManagerWidget;
class DBObjectManagerLoadWidget;
class MetaDBOVariableConfigurationDialog;
class DBOVariable;
class MetaDBOVariable;
class DBOVariableSet;
class DBSchemaManager;

class DBObjectManager : public QObject, public Configurable
{
    Q_OBJECT

public slots:
    // void schemaLockedSlot ();
    void loadSlot();
    void updateSchemaInformationSlot();

    void databaseOpenedSlot();
    void databaseContentChangedSlot();
    void loadingDoneSlot(DBObject& object);
    void metaDialogOKSlot();

signals:
    void dbObjectsChangedSignal();
    void schemaChangedSignal();

    void loadingStartedSignal();
    void allLoadingDoneSignal();

public:
    const static std::vector<std::string> data_source_types_;

    DBObjectManager(const std::string& class_id, const std::string& instance_id, COMPASS* compass);

    bool existsObject(const std::string& dbo_name);
    DBObject& object(const std::string& dbo_name);
    void deleteObject(const std::string& dbo_name);
    bool hasData();

    using DBObjectIterator = typename std::map<std::string, DBObject*>::iterator;
    DBObjectIterator begin() { return objects_.begin(); }
    DBObjectIterator end() { return objects_.end(); }
    size_t size() { return objects_.size(); }

    bool existsMetaVariable(const std::string& var_name);
    MetaDBOVariable& metaVariable(const std::string& var_name);
    void deleteMetaVariable(const std::string& var_name);
    const std::vector<std::unique_ptr<MetaDBOVariable>>& metaVariables() { return meta_variables_; }

    bool usedInMetaVariable(const DBOVariable& variable);

    bool hasDataSource(unsigned int ds_id);
    bool canAddNewDataSourceFromConfig (unsigned int ds_id);
    void addNewDataSource (unsigned int ds_id);
    DBContent::DBDataSource& dataSource(unsigned int ds_id);
    const std::vector<std::unique_ptr<DBContent::DBDataSource>>& dataSources() const;

    void saveDBDataSources();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    virtual ~DBObjectManager();

    DBObjectManagerWidget* widget();
    DBObjectManagerLoadWidget* loadWidget();

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
    DBOVariable& orderVariable();
    void orderVariable(DBOVariable& variable);
    bool hasOrderMetaVariable();
    MetaDBOVariable& orderMetaVariable();
    void orderMetaVariable(MetaDBOVariable& variable);
    void clearOrderVariable();

    //    void lock ();
    //    void unlock ();

    void quitLoading();

    //void removeDependenciesForSchema(const std::string& schema_name);

    bool hasAssociations() const;
    void setAssociationsDataSource(const std::string& dbo, const std::string& data_source_name);
    void setAssociationsByAll();
    void removeAssociations();

    bool hasAssociationsDataSource() const;
    std::string associationsDBObject() const;
    std::string associationsDataSourceName() const;

    bool isOtherDBObjectPostProcessing(DBObject& object);

    bool loadInProgress() const;

    MetaDBOVariableConfigurationDialog* metaVariableConfigdialog();

protected:
    COMPASS& compass_;

    bool use_order_{false};
    bool use_order_ascending_{false};
    std::string order_variable_dbo_name_;
    std::string order_variable_name_;

    bool use_limit_{false};
    unsigned int limit_min_{0};
    unsigned int limit_max_{100000};

    bool locked_{false};

    bool has_associations_{false};
    std::string associations_dbo_;
    std::string associations_ds_;

    bool load_in_progress_{false};

    /// Container with all DBOs (DBO name -> DBO pointer)
    std::map<std::string, DBObject*> objects_;
    std::vector<std::unique_ptr<MetaDBOVariable>> meta_variables_;

    std::vector<std::unique_ptr<DBContent::ConfigurationDataSource>> config_data_sources_;
    std::vector<std::unique_ptr<DBContent::DBDataSource>> db_data_sources_;

    std::unique_ptr<DBObjectManagerWidget> widget_;
    std::unique_ptr<DBObjectManagerLoadWidget> load_widget_;

    std::unique_ptr<MetaDBOVariableConfigurationDialog> meta_cfg_dialog_;

    virtual void checkSubConfigurables();
    void finishLoading();

    bool hasConfigDataSource (unsigned int sac, unsigned int sic);
    DBContent::ConfigurationDataSource& getConfigDataSource (unsigned int sac, unsigned int sic);

    void loadDBDataSources();
    void sortDBDataSources();
};

#endif /* DBOBJECTMANAGER_H_ */
