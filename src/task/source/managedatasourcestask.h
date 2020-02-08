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


#ifndef MANAGEDATASOURCESTASK_H
#define MANAGEDATASOURCESTASK_H


#include "configurable.h"
#include "task.h"
#include "storeddbodatasource.h"
#include "dboeditdatasourceactionoptions.h"

#include <QObject>

#include <memory>

class TaskManager;
class ManageDataSourcesTaskWidget;
class DBOEditDataSourcesWidget;

class ManageDataSourcesTask: public Task, public Configurable
{
public:
    ManageDataSourcesTask(const std::string& class_id, const std::string& instance_id,
                     TaskManager& task_manager);

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    virtual TaskWidget* widget ();
    virtual void deleteWidget ();

    virtual bool checkPrerequisites ();
    virtual bool isRecommended ();
    virtual bool isRequired () { return false; }

    /// @brief Returns flag indication if a StoredDBODataSource identified by name exists
    bool hasStoredDataSource (const std::string& dbo_name, unsigned int id);
    /// @brief Returns variable identified by id
    StoredDBODataSource& storedDataSource (const std::string& dbo_name, unsigned int id);
    StoredDBODataSource& addNewStoredDataSource (const std::string& dbo_name);
    void deleteStoredDataSource (const std::string& dbo_name, unsigned int id);
    const std::map<unsigned int, StoredDBODataSource>& storedDataSources(const std::string& dbo_name);

    DBOEditDataSourceActionOptionsCollection getSyncOptionsFromDB (const std::string& dbo_name);
    DBOEditDataSourceActionOptionsCollection getSyncOptionsFromCfg (const std::string& dbo_name);

    DBOEditDataSourcesWidget* editDataSourcesWidget(const std::string& dbo_name);

    void exportConfigDataSources ();
    void clearConfigDataSources ();
    void importConfigDataSources ();
    void importConfigDataSources (const std::string& filename);
    void autoSyncAllConfigDataSourcesToDB ();

    bool hasDataSource (const std::string& dbo_name, unsigned int sac, unsigned int sic);
    StoredDBODataSource& getDataSource (const std::string& dbo_name, unsigned int sac, unsigned int sic);
    bool hasDataSource (const std::string& dbo_name, const std::string& name);
    StoredDBODataSource& getDataSource (const std::string& dbo_name, const std::string& name);

protected:
    std::unique_ptr<ManageDataSourcesTaskWidget> widget_;

    virtual void checkSubConfigurables () {}

    std::map<std::string, std::unique_ptr<DBOEditDataSourcesWidget>> edit_ds_widgets_;

    std::map<std::string, std::map<unsigned int, StoredDBODataSource>> stored_data_sources_;
    // dbo -> id -> ds

};

#endif // MANAGEDATASOURCESTASK_H
