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

#include <QFileDialog>
#include <fstream>

#include "atsdb.h"
#include "dbinterface.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dboeditdatasourceactionoptionswidget.h"
#include "dboeditdatasourceswidget.h"
#include "json.hpp"
#include "managedatasourcestaskwidget.h"
#include "taskmanager.h"

using namespace nlohmann;

ManageDataSourcesTask::ManageDataSourcesTask(const std::string& class_id,
                                             const std::string& instance_id,
                                             TaskManager& task_manager)
    : Task("ManageDataSourcesTask", "Manage Data Sources", true, false, task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_manage_datasources.json")
{
    tooltip_ =
        "Allows management of data sources, as stored in the configuration as well as the "
        "database.";

    createSubConfigurables();
}

TaskWidget* ManageDataSourcesTask::widget()
{
    if (!widget_)
    {
        widget_.reset(new ManageDataSourcesTaskWidget(*this));

        connect(&task_manager_, &TaskManager::expertModeChangedSignal, widget_.get(),
                &ManageDataSourcesTaskWidget::expertModeChangedSlot);
    }

    return widget_.get();
}

void ManageDataSourcesTask::deleteWidget() { widget_.reset(nullptr); }

void ManageDataSourcesTask::generateSubConfigurable(const std::string& class_id,
                                                    const std::string& instance_id)
{
    if (class_id == "StoredDBODataSource")
    {
        unsigned int id = configuration()
                              .getSubConfiguration(class_id, instance_id)
                              .getParameterConfigValueUint("id");
        std::string dbo_name = configuration()
                                   .getSubConfiguration(class_id, instance_id)
                                   .getParameterConfigValueString("dbo_name");

        assert(stored_data_sources_[dbo_name].find(id) == stored_data_sources_[dbo_name].end());

        logdbg << "ManageDataSourcesTask: generateSubConfigurable: generating stored DS "
               << instance_id << " with object " << dbo_name << " id " << id;

        stored_data_sources_[dbo_name].emplace(std::piecewise_construct,
                                               std::forward_as_tuple(id),  // args for key
                                               std::forward_as_tuple(class_id, instance_id, *this));
        // args for mapped value
    }
    else
        throw std::runtime_error(
            "ManageDataSourcesTask: generateSubConfigurable: unknown class_id " + class_id);
}

bool ManageDataSourcesTask::checkPrerequisites()
{
    for (auto& widget_it : edit_ds_widgets_)
        widget_it.second->update();

    if (!ATSDB::instance().interface().ready())
        return false;

    return true;
}

bool ManageDataSourcesTask::isRecommended()
{
    for (auto& dbo_it : ATSDB::instance().objectManager())
    {
        for (auto& ds_it : dbo_it.second->dataSources())
        {
            if (ds_it.second.name() ==
                std::to_string(ds_it.second.id()))  // check if name is not set
                return true;

            if (dbo_it.first == "Radar")
            {
                if (!ds_it.second.hasLatitude() || !ds_it.second.hasLongitude() ||
                    !ds_it.second.hasAltitude())
                    return true;
            }
        }
    }

    return false;
}

bool ManageDataSourcesTask::hasStoredDataSource(const std::string& dbo_name, unsigned int id)
{
    return stored_data_sources_[dbo_name].find(id) != stored_data_sources_[dbo_name].end();
}

StoredDBODataSource& ManageDataSourcesTask::storedDataSource(const std::string& dbo_name,
                                                             unsigned int id)
{
    assert(hasStoredDataSource(dbo_name, id));
    return stored_data_sources_[dbo_name].at(id);
}

StoredDBODataSource& ManageDataSourcesTask::addNewStoredDataSource(const std::string& dbo_name)
{
    unsigned int id = stored_data_sources_[dbo_name].size()
                          ? stored_data_sources_[dbo_name].rbegin()->first + 1
                          : 0;

    loginf << "ManageDataSourcesTask: addNewStoredDataSource: new for object " << dbo_name << " id "
           << id;

    assert(!hasStoredDataSource(dbo_name, id));

    std::string instance_id = "StoredDBODataSource" + dbo_name + std::to_string(id);

    Configuration& config =
        configuration().addNewSubConfiguration("StoredDBODataSource", instance_id);
    config.addParameterUnsignedInt("id", id);
    config.addParameterString("dbo_name", dbo_name);

    generateSubConfigurable("StoredDBODataSource", instance_id);

    return storedDataSource(dbo_name, id);
}

// void ManageDataSourcesTask::renameStoredDataSource (const std::string& name, const std::string&
// new_name)
//{
//    loginf << "ManageDataSourcesTask: renameStoredDataSource: name " << name << " new_name " <<
//    new_name;

//    assert (hasStoredDataSource (name));
//    assert (!hasStoredDataSource (new_name));

//    stored_data_sources_[new_name] = std::move(stored_data_sources_.at(name));

//    stored_data_sources_.erase(name);

//    assert (hasStoredDataSource (new_name));
//    stored_data_sources_.at(new_name).name(new_name);
//}

void ManageDataSourcesTask::deleteStoredDataSource(const std::string& dbo_name, unsigned int id)
{
    assert(hasStoredDataSource(dbo_name, id));
    stored_data_sources_[dbo_name].erase(id);
    assert(!hasStoredDataSource(dbo_name, id));

    if (edit_ds_widgets_[dbo_name])
        edit_ds_widgets_[dbo_name]->update();
}

const std::map<unsigned int, StoredDBODataSource>& ManageDataSourcesTask::storedDataSources(
    const std::string& dbo_name)
{
    return stored_data_sources_[dbo_name];  // might not have ones, so use []
}

DBOEditDataSourceActionOptionsCollection ManageDataSourcesTask::getSyncOptionsFromDB(
    const std::string& dbo_name)
{
    DBOEditDataSourceActionOptionsCollection options_collection;

    assert(ATSDB::instance().objectManager().existsObject(dbo_name));

    const std::map<int, DBODataSource>& db_data_sources =
        ATSDB::instance().objectManager().object(dbo_name).dataSources();

    for (auto& ds_it : db_data_sources)
    {
        assert(ds_it.first >= 0);  // todo refactor to uint?
        unsigned int id = ds_it.first;
        options_collection[id] = DBOEditDataSourceActionOptionsCreator::getSyncOptionsFromDB(
            ATSDB::instance().objectManager().object(dbo_name), ds_it.second);
    }

    return options_collection;
}

DBOEditDataSourceActionOptionsCollection ManageDataSourcesTask::getSyncOptionsFromCfg(
    const std::string& dbo_name)
{
    DBOEditDataSourceActionOptionsCollection options_collection;

    assert(ATSDB::instance().objectManager().existsObject(dbo_name));

    for (auto& ds_it : stored_data_sources_[dbo_name])
    {
        assert(ds_it.first >= 0);  // todo refactor to uint?
        unsigned int id = ds_it.first;
        options_collection[id] = DBOEditDataSourceActionOptionsCreator::getSyncOptionsFromCfg(
            ATSDB::instance().objectManager().object(dbo_name), ds_it.second);
    }

    return options_collection;
}

DBOEditDataSourcesWidget* ManageDataSourcesTask::editDataSourcesWidget(const std::string& dbo_name)
{
    if (!edit_ds_widgets_[dbo_name])
    {
        assert(ATSDB::instance().objectManager().existsObject(dbo_name));

        edit_ds_widgets_[dbo_name].reset(new DBOEditDataSourcesWidget(
            *this, ATSDB::instance().objectManager().object(dbo_name)));
    }

    return edit_ds_widgets_[dbo_name].get();
}

void ManageDataSourcesTask::exportConfigDataSources()
{
    loginf << "ManageDataSourcesTask: exportConfigDataSources";

    json j;

    for (auto& dbo_ds_it : stored_data_sources_)
    {
        unsigned int cnt = 0;

        for (auto& ds_it : dbo_ds_it.second)
        {
            j[dbo_ds_it.first][cnt] = ds_it.second.getAsJSON();
            ++cnt;
        }
    }

    logdbg << "ManageDataSourcesTask: exportConfigDataSources: json '" << j.dump(4) << "'";

    QFileDialog dialog(nullptr);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter("JSON Files (*.json)");
    dialog.setDefaultSuffix("json");
    dialog.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);

    QStringList file_names;
    if (dialog.exec())
        file_names = dialog.selectedFiles();

    QString filename;

    if (file_names.size() == 1)
        filename = file_names.at(0);

    if (filename.size() > 0)
    {
        loginf << "ManageDataSourcesTask: exportConfigDataSources: saving in filename '"
               << filename.toStdString() << "'";

        std::ofstream output_file;
        output_file.open(filename.toStdString(), std::ios_base::out);

        output_file << j.dump(4);

        task_manager_.appendInfo(
            "ManageDataSourcesTask: exported configuration data sources to file '" +
            filename.toStdString() + "'");
    }
    else
        loginf << "ManageDataSourcesTask: exportConfigDataSources: cancelled";
}

void ManageDataSourcesTask::clearConfigDataSources()
{
    loginf << "ManageDataSourcesTask: clearConfigDataSources";

    for (auto& dbo_ds_it : stored_data_sources_)
        dbo_ds_it.second.clear();

    for (auto& edit_it : edit_ds_widgets_)
        edit_it.second->update();

    task_manager_.appendInfo("ManageDataSourcesTask: cleared all configuration data sources");

    emit statusChangedSignal(name_);
}

void ManageDataSourcesTask::importConfigDataSources()
{
    loginf << "ManageDataSourcesTask: importConfigDataSources";

    QString filename =
        QFileDialog::getOpenFileName(nullptr, "Add Data Sources as JSON", "", "*.json");

    if (filename.size() > 0)
        importConfigDataSources(filename.toStdString());
    else
        loginf << "ManageDataSourcesTask: importConfigDataSources: cancelled";
}

void ManageDataSourcesTask::importConfigDataSources(const std::string& filename)
{
    loginf << "ManageDataSourcesTask: importConfigDataSources: filename '" << filename << "'";

    std::ifstream input_file(filename, std::ifstream::in);

    try
    {
        json j = json::parse(input_file);

        for (auto& j_dbo_it : j.items())
        {
            std::string dbo_name = j_dbo_it.key();

            for (auto& j_ds_it : j_dbo_it.value().get<json::array_t>())
            {
                loginf << "ManageDataSourcesTask: importConfigDataSources: found dbo " << dbo_name
                       << " ds '" << j_ds_it.dump(4) << "'";

                assert(j_ds_it.contains("dbo_name"));
                assert(j_ds_it.contains("name"));

                if (j_ds_it.contains("sac") && j_ds_it.contains("sic"))
                {
                    unsigned int sac = j_ds_it.at("sac");
                    unsigned int sic = j_ds_it.at("sic");

                    if (hasDataSource(dbo_name, sac, sic))
                    {
                        loginf << "ManageDataSourcesTask: importConfigDataSources: setting "
                                  "existing by sac/sic "
                               << sac << "/" << sic;
                        getDataSource(dbo_name, sac, sic).setFromJSON(j_ds_it);
                        continue;
                    }
                }

                std::string name = j_ds_it.at("name");

                if (hasDataSource(dbo_name, name))
                {
                    loginf << "ManageDataSourcesTask: importConfigDataSources: setting existing by "
                              "name "
                           << name;
                    getDataSource(dbo_name, name).setFromJSON(j_ds_it);
                    continue;
                }

                loginf << "ManageDataSourcesTask: importConfigDataSources: no equivalent found, "
                          "creating new";

                StoredDBODataSource& new_ds = addNewStoredDataSource(dbo_name);
                new_ds.setFromJSON(j_ds_it);
            }
        }

        for (auto& edit_it : edit_ds_widgets_)
            edit_it.second->update();

        task_manager_.appendInfo(
            "ManageDataSourcesTask: imported configuration data sources from  file '" + filename +
            "'");
    }
    catch (json::exception& e)
    {
        logerr << "ManageDataSourcesTask: importConfigDataSources: could not load file '"
               << filename << "'";
        throw e;
    }
}

void ManageDataSourcesTask::autoSyncAllConfigDataSourcesToDB()
{
    loginf << "ManageDataSourcesTask: autoSyncAllConfigDataSourcesToDB";
    assert(widget_);

    for (auto& edit_ds_it : edit_ds_widgets_)
    {
        DBOEditDataSourcesWidget* current_widget = edit_ds_it.second.get();
        widget_->setCurrentWidget(current_widget);
        current_widget->syncOptionsFromCfgSlot();
        current_widget->performActionsSlot();
    }

    task_manager_.appendInfo(
        "ManageDataSourcesTask: synced all configuration data sources to database");
}

bool ManageDataSourcesTask::hasDataSource(const std::string& dbo_name, unsigned int sac,
                                          unsigned int sic)
{
    if (!stored_data_sources_.count(dbo_name))
        return false;

    for (auto& ds_it : stored_data_sources_.at(dbo_name))
    {
        if (ds_it.second.hasSac() && ds_it.second.sac() == sac && ds_it.second.hasSic() &&
            ds_it.second.sic() == sic)
            return true;
    }

    return false;
}

StoredDBODataSource& ManageDataSourcesTask::getDataSource(const std::string& dbo_name,
                                                          unsigned int sac, unsigned int sic)
{
    assert(hasDataSource(dbo_name, sac, sic));

    for (auto& ds_it : stored_data_sources_.at(dbo_name))
    {
        if (ds_it.second.hasSac() && ds_it.second.sac() == sac && ds_it.second.hasSic() &&
            ds_it.second.sic() == sic)
            return ds_it.second;
    }

    throw std::runtime_error("ManageDataSourcesTask: getDataSource: sac/sic not found");
}

bool ManageDataSourcesTask::hasDataSource(const std::string& dbo_name, const std::string& name)
{
    if (!stored_data_sources_.count(dbo_name))
        return false;

    for (auto& ds_it : stored_data_sources_.at(dbo_name))
    {
        if (ds_it.second.name() == name)
            return true;
    }

    return false;
}

StoredDBODataSource& ManageDataSourcesTask::getDataSource(const std::string& dbo_name,
                                                          const std::string& name)
{
    assert(hasDataSource(dbo_name, name));

    for (auto& ds_it : stored_data_sources_.at(dbo_name))
    {
        if (ds_it.second.name() == name)
            return ds_it.second;
    }

    throw std::runtime_error("ManageDataSourcesTask: getDataSource: name not found");
}
