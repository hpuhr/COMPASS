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


#include "managedatasourcestask.h"
#include "managedatasourcestaskwidget.h"
#include "taskmanager.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "dboeditdatasourceswidget.h"
#include "dboeditdatasourceactionoptionswidget.h"
#include "dbobjectmanager.h"
#include "dbobject.h"

ManageDataSourcesTask::ManageDataSourcesTask(const std::string& class_id, const std::string& instance_id,
                                   TaskManager& task_manager)
    : Task("ManageDataSourcesTask", "Manage Data Sources", true, false, task_manager),
      Configurable (class_id, instance_id, &task_manager, "task_manage_datasources.json")
{
    createSubConfigurables();
}

TaskWidget* ManageDataSourcesTask::widget ()
{
    if (!widget_)
    {
        widget_.reset(new ManageDataSourcesTaskWidget(*this));

        connect (&task_manager_, &TaskManager::expertModeChangedSignal,
                 widget_.get(), &ManageDataSourcesTaskWidget::expertModeChangedSlot);
    }

    return widget_.get();
}

void ManageDataSourcesTask::deleteWidget ()
{
    widget_.reset(nullptr);
}

void ManageDataSourcesTask::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    if (class_id == "StoredDBODataSource")
    {
        unsigned int id = configuration().getSubConfiguration(
                    class_id, instance_id).getParameterConfigValueUint("id");
        std::string dbo_name = configuration().getSubConfiguration(
                    class_id, instance_id).getParameterConfigValueString("dbo_name");

        assert (stored_data_sources_[dbo_name].find (id) == stored_data_sources_[dbo_name].end());

        loginf << "ManageDataSourcesTask: generateSubConfigurable: generating stored DS " << instance_id
               << " with object " << dbo_name << " id " << id;

        stored_data_sources_[dbo_name].emplace(std::piecewise_construct,
                                               std::forward_as_tuple(id),  // args for key
                                               std::forward_as_tuple(class_id, instance_id, *this));
        // args for mapped value
    }
    else
        throw std::runtime_error ("ManageDataSourcesTask: generateSubConfigurable: unknown class_id "+class_id );
}

bool ManageDataSourcesTask::checkPrerequisites ()
{
    if (!ATSDB::instance().interface().ready())
        return false;

    return true;
}

bool ManageDataSourcesTask::isRecommended ()
{
    return false;
}

bool ManageDataSourcesTask::hasStoredDataSource (const std::string& dbo_name, unsigned int id)
{
    return stored_data_sources_[dbo_name].find (id) != stored_data_sources_[dbo_name].end();
}

StoredDBODataSource& ManageDataSourcesTask::storedDataSource (const std::string& dbo_name, unsigned int id)
{
    assert (hasStoredDataSource (dbo_name, id));
    return stored_data_sources_[dbo_name].at(id);
}

StoredDBODataSource& ManageDataSourcesTask::addNewStoredDataSource (const std::string& dbo_name)
{
    unsigned int id = stored_data_sources_[dbo_name].size() ? stored_data_sources_[dbo_name].rbegin()->first+1 : 0;

    loginf << "ManageDataSourcesTask: addNewStoredDataSource: new for object " << dbo_name << " id " << id;

    assert (!hasStoredDataSource (dbo_name, id));

    std::string instance_id = "StoredDBODataSource"+dbo_name+std::to_string(id);

    Configuration& config = configuration().addNewSubConfiguration("StoredDBODataSource", instance_id);
    config.addParameterUnsignedInt ("id", id);
    config.addParameterString ("dbo_name", dbo_name);

    generateSubConfigurable("StoredDBODataSource", instance_id);

    return storedDataSource(dbo_name, id);
}

//void ManageDataSourcesTask::renameStoredDataSource (const std::string& name, const std::string& new_name)
//{
//    loginf << "ManageDataSourcesTask: renameStoredDataSource: name " << name << " new_name " << new_name;

//    assert (hasStoredDataSource (name));
//    assert (!hasStoredDataSource (new_name));

//    stored_data_sources_[new_name] = std::move(stored_data_sources_.at(name));

//    stored_data_sources_.erase(name);

//    assert (hasStoredDataSource (new_name));
//    stored_data_sources_.at(new_name).name(new_name);
//}

void ManageDataSourcesTask::deleteStoredDataSource (const std::string& dbo_name, unsigned int id)
{
    assert (hasStoredDataSource (dbo_name, id));
    stored_data_sources_[dbo_name].erase(id);
    assert (!hasStoredDataSource (dbo_name, id));

    if (edit_ds_widgets_[dbo_name])
        edit_ds_widgets_[dbo_name]->update();
}

const std::map<unsigned int, StoredDBODataSource>& ManageDataSourcesTask::storedDataSources(
        const std::string& dbo_name) const
{
    assert (stored_data_sources_.count(dbo_name));
    return stored_data_sources_.at(dbo_name);
}

DBOEditDataSourceActionOptionsCollection ManageDataSourcesTask::getSyncOptionsFromDB (const std::string& dbo_name)
{
    DBOEditDataSourceActionOptionsCollection options_collection;

    assert (ATSDB::instance().objectManager().existsObject(dbo_name));

    const std::map<int, DBODataSource>& db_data_sources =
            ATSDB::instance().objectManager().object(dbo_name).dataSources();

    for (auto& ds_it : db_data_sources)
    {
        assert (ds_it.first >= 0); // todo refactor to uint?
        unsigned int id = ds_it.first;
        options_collection [id] = DBOEditDataSourceActionOptionsCreator::getSyncOptionsFromDB (
                    ATSDB::instance().objectManager().object(dbo_name), ds_it.second);
    }

    return options_collection;
}

DBOEditDataSourceActionOptionsCollection ManageDataSourcesTask::getSyncOptionsFromCfg (const std::string& dbo_name)
{
    DBOEditDataSourceActionOptionsCollection options_collection;

    assert (ATSDB::instance().objectManager().existsObject(dbo_name));

    for (auto& ds_it : stored_data_sources_[dbo_name])
    {
        assert (ds_it.first >= 0); // todo refactor to uint?
        unsigned int id = ds_it.first;
        options_collection [id] = DBOEditDataSourceActionOptionsCreator::getSyncOptionsFromCfg (
                    ATSDB::instance().objectManager().object(dbo_name), ds_it.second);
    }

    return options_collection;
}

DBOEditDataSourcesWidget* ManageDataSourcesTask::editDataSourcesWidget(const std::string& dbo_name)
{
    if (!edit_ds_widgets_[dbo_name])
    {
        assert (ATSDB::instance().objectManager().existsObject(dbo_name));

        edit_ds_widgets_[dbo_name].reset(
                    new DBOEditDataSourcesWidget (*this, ATSDB::instance().objectManager().object(dbo_name)));
    }

    return edit_ds_widgets_[dbo_name].get();
}
