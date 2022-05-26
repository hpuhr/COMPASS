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

#include "viewpointsimporttask.h"
#include "viewpointsimporttaskdialog.h"
#include "taskmanager.h"
#include "compass.h"
#include "dbinterface.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "files.h"
#include "logger.h"
#include "viewmanager.h"
#include "viewpointsimporttask.h"
#include "viewpointsimporttaskwidget.h"
#include "global.h"

#include "asteriximporttask.h"
#include "asteriximporttaskwidget.h"
#include "asterixoverridewidget.h"

//#include "managedatasourcestask.h"

#include <fstream>

#include <QCoreApplication>
#include <QApplication>
#include <QThread>
#include <QMessageBox>

#include "boost/date_time/posix_time/posix_time.hpp"

using namespace std;
using namespace nlohmann;
using namespace Utils;

//const std::string DONE_PROPERTY_NAME = "view_points_imported";

ViewPointsImportTask::ViewPointsImportTask(const std::string& class_id, const std::string& instance_id,
                                           TaskManager& task_manager)
    : Task("ViewPointsImportTask", "Import View Points", true, false, task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_import_view_points.json")
{
    tooltip_ =
            "Allows import of view points and associated datasets.";

    createSubConfigurables(); // no thing

    current_error_ = "No filename set";
}

ViewPointsImportTask::~ViewPointsImportTask()
{
}

ViewPointsImportTaskDialog* ViewPointsImportTask::dialog()
{
    if (!dialog_)
    {
        dialog_.reset(new ViewPointsImportTaskDialog(*this));

        connect(dialog_.get(), &ViewPointsImportTaskDialog::importSignal,
                this, &ViewPointsImportTask::dialogImportSlot);

        connect(dialog_.get(), &ViewPointsImportTaskDialog::cancelSignal,
                this, &ViewPointsImportTask::dialogCancelSlot);
    }

    assert(dialog_);
    return dialog_.get();

}

void ViewPointsImportTask::dialogImportSlot()
{
    assert (canRun());

    assert (dialog_);
    dialog_->hide();

    run();
}

void ViewPointsImportTask::dialogCancelSlot()
{
    assert (dialog_);
    dialog_->hide();
}

void ViewPointsImportTask::generateSubConfigurable(const std::string& class_id,
                                                   const std::string& instance_id)
{
    throw std::runtime_error("ViewPointsImportTask: generateSubConfigurable: unknown class_id " +
                             class_id);
}

void ViewPointsImportTask::importFilename(const std::string& filename)
{
    current_filename_ = filename;

    parseCurrentFile();

    if (dialog_)
        dialog_->updateText();
}

std::string ViewPointsImportTask::currentError() const
{
    return current_error_;
}

void ViewPointsImportTask::parseCurrentFile ()
{
    loginf << "ViewPointsImportTask: parseCurrentFile: file '" << current_filename_ << "'";

    current_error_ = "";

    current_data_.clear();

    if (!Files::fileExists(current_filename_))
    {
        current_error_ = "file '" + current_filename_ + "' does not exist";
        logerr << "ViewPointsImportTask: parseCurrentFile: " << current_error_;
        return;
    }

    std::ifstream ifs(current_filename_);

    try
    {
        current_data_ = json::parse(ifs);

        checkParsedData();
    }
    catch (exception& e)
    {
        current_error_ = "parsing file '" + current_filename_ + "' resulted in error '" + e.what() + "'";
        logerr << "ViewPointsImportTask: parseCurrentFile: " << current_error_;
    }

    loginf << "ViewPointsImportTask: parseCurrentFile: done";
}

void ViewPointsImportTask::checkParsedData ()
{
    loginf << "ViewPointsImportTask: checkParsedData";

    if (!current_data_.is_object())
        throw std::runtime_error("current data is not an object");

    if (!current_data_.contains("content_type")
            || !current_data_.at("content_type").is_string()
            || current_data_.at("content_type") != "view_points")
        throw std::runtime_error("current data is not view point content");

    if (!current_data_.contains("content_version")
            || !current_data_.at("content_version").is_string()
            || current_data_.at("content_version") != "0.2")
        throw std::runtime_error("current data content version is not supported");

    if (current_data_.contains("view_point_context"))
    {
        json& context = current_data_.at("view_point_context");

        if (context.contains("datasets"))
        {
            if (!context.at("datasets").is_array())
                throw std::runtime_error("datasets is not an array");

            for (json& ds_it : context.at("datasets").get<json::array_t>())
            {
                if (ds_it.contains("name") && !ds_it.at("name").is_string())
                    throw std::runtime_error("dataset '"+ds_it.dump()+"' does not contain a valid name");

                if (!ds_it.contains("filename") || !ds_it.at("filename").is_string())
                    throw std::runtime_error("dataset '"+ds_it.dump()+"' does not contain a valid filename");

                std::string filename = ds_it.at("filename");

                bool found = true;

                if (!Files::fileExists(filename))
                {
                    found = false;

                    std::string file = Files::getFilenameFromPath(filename);
                    std::string dir = Files::getDirectoryFromPath(current_filename_);

                    loginf << "ViewPointsImportTask: checkParsedData: filename '" << filename
                           << "' not found, checking for file '" << file << "' in dir '" << dir << "'";

                    filename = dir+"/"+file;

                    if (Files::fileExists(filename))
                    {
                        found = true;

                        loginf << "ViewPointsImportTask: checkParsedData: filename '" << filename
                               << "' found at different path";
                    }
                }

                if (!found)
                    throw std::runtime_error("dataset '"+ds_it.dump()+"' does not contain a usable filename");
            }
        }
    }

    if (!current_data_.contains("view_points"))
        throw std::runtime_error("current data does not contain view points");

    json& view_points = current_data_.at("view_points");

    if (!view_points.is_array())
        throw std::runtime_error("view_points is not an array");

    if (!view_points.size())
        throw std::runtime_error("view_points is an empty array");

    for (auto& vp_it : view_points.get<json::array_t>())
    {
        if (!vp_it.contains("id") || !vp_it.at("id").is_number())
            throw std::runtime_error("view point '"+vp_it.dump()+"' does not contain a valid id");

        if (!vp_it.contains("type") || !vp_it.at("type").is_string())
            throw std::runtime_error("view point '"+vp_it.dump()+"' does not contain a valid type");
    }

    loginf << "ViewPointsImportTask: checkParsedData: current data seems to be valid, contains " << view_points.size()
           << " view points";
}

bool ViewPointsImportTask::checkPrerequisites()
{
    return canImport();
}

bool ViewPointsImportTask::isRecommended()
{
    return false;
}

bool ViewPointsImportTask::isRequired()
{
    return false;
}

bool ViewPointsImportTask::canImport ()
{
    return current_error_.size() == 0;
}

bool ViewPointsImportTask::canRun()
{
    return canImport();
}

void ViewPointsImportTask::run()
{
    loginf << "ViewPointsImportTask: import";

    assert (canImport()); // checked file content, version etc
    done_ = false;
    stopped_ = false;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // view points

    assert (current_data_.contains("view_points"));

    json& view_points = current_data_.at("view_points");
    assert (view_points.size());

    DBInterface& db_interface = COMPASS::instance().interface();

    // check and clear existing ones
    if(db_interface.existsViewPointsTable() && db_interface.viewPoints().size())
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(
                    nullptr, "Clear Existing View Points",
                    "There are already view points defined in the database.\n\n"
                    "Do you agree to delete all view points?",
                    QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            loginf << "ViewPointsImportTask: import: deleting all view points";
            db_interface.deleteAllViewPoints();
        }
        else
        {
            loginf << "ViewPointsImportTask: import: aborted";

            done_ = true;
            return;
        }
    }

    unsigned int id;
    for (auto& vp_it : view_points.get<json::array_t>())
    {
        assert (vp_it.contains("id"));

        id = vp_it.at("id");

        if (!vp_it.contains("status"))
            vp_it["status"] = "open";

        db_interface.setViewPoint(id, vp_it.dump());
    }

    COMPASS::instance().viewManager().loadViewPoints();

    loginf << "ViewPointsImportTask: import: imported " << to_string(view_points.size()) << " view points";

    QApplication::restoreOverrideCursor();

    // datasets
    if (current_data_.contains("view_point_context"))
    {
        json& context = current_data_.at("view_point_context");

        if (context.contains("datasets"))
        {
            for (auto& ds_it : context.at("datasets").get<json::array_t>())
            {
                std::string name;

                if (ds_it.contains("name"))
                    name= ds_it.at("name");

                std::string filename = ds_it.at("filename");

                loginf << "ViewPointsImportTask: import: importing dataset name '" << name
                       << "' file '" << filename << "'";

                if (!Files::fileExists(filename))
                {
                    std::string file = Files::getFilenameFromPath(filename);
                    std::string dir = Files::getDirectoryFromPath(current_filename_);

                    filename = dir+"/"+file;

                    assert (Files::fileExists(filename));
                }

                ASTERIXImportTask& asterix_importer_task = task_manager_.asterixImporterTask();

                // set data source info
                if (ds_it.contains("ds_name") && ds_it.contains("ds_sac") && ds_it.contains("ds_sic"))
                {
                    TODO_ASSERT

                            //                    ManageDataSourcesTask& ds_task = COMPASS::instance().taskManager().manageDataSourcesTask();

                            //                    assert (ds_it.at("ds_name").is_string());
                            //                    assert (ds_it.at("ds_sac").is_number());
                            //                    assert (ds_it.at("ds_sic").is_number());

                            //                    std::string ds_name = ds_it.at("ds_name");
                            //                    int ds_sac = ds_it.at("ds_sac");
                            //                    assert (ds_sac >= 0);
                            //                    int ds_sic = ds_it.at("ds_sic");
                            //                    assert (ds_sic >= 0);

                            //                    if (!ds_task.hasDataSource("Tracker", ds_sac, ds_sic)) // add if not existing
                            //                    {
                            //                        loginf << "ViewPointsImportTask: import: adding data source '" << ds_name << "' "
                            //                                                       << ds_sac << "/" << ds_sic;
                            //                        StoredDBODataSource& new_ds = ds_task.addNewStoredDataSource("Tracker");
                            //                        new_ds.name(ds_name);
                            //                        new_ds.sac(ds_sac);
                            //                        new_ds.sic(ds_sic);
                            //                    }
                            //                    else // set name if existing
                            //                    {
                            //                        loginf << "ViewPointsImportTask: import: setting data source '" << ds_name << "' "
                            //                                                       << ds_sac << "/" << ds_sic;
                            //                        StoredDBODataSource& ds = ds_task.getDataSource("Tracker", ds_sac, ds_sic);
                            //                        ds.name(ds_name);
                            //                    }
                }


                if (ds_it.contains("ds_sac") && ds_it.contains("ds_sic")
                        && ds_it.contains("ds_sac_override") && ds_it.contains("ds_sic_override")
                        && ds_it.contains("time_offset"))
                {
                    TODO_ASSERT

                            //                    loginf << "ViewPointsImportTask: import: override information set";

                            //                    // set override information
                            //                    asterix_importer_task.overrideActive(true);

                            //                    assert (ds_it.at("ds_sac").is_number());
                            //                    asterix_importer_task.overrideSacOrg(ds_it.at("ds_sac"));

                            //                    assert (ds_it.at("ds_sic").is_number());
                            //                    asterix_importer_task.overrideSicOrg(ds_it.at("ds_sic"));

                            //                    assert (ds_it.at("ds_sac_override").is_number());
                            //                    asterix_importer_task.overrideSacNew(ds_it.at("ds_sac_override"));

                            //                    assert (ds_it.at("ds_sic_override").is_number());
                            //                    asterix_importer_task.overrideSicNew(ds_it.at("ds_sic_override"));

                            //                    assert (ds_it.at("time_offset").is_number());
                            //                    asterix_importer_task.overrideTodOffset(ds_it.at("time_offset"));

                            //                    // set new data source info
                            //                    assert (ds_it.at("ds_name").is_string());
                            //                    assert (ds_it.at("ds_sac_override").is_number());
                            //                    assert (ds_it.at("ds_sic_override").is_number());

                            //                    std::string ds_name = ds_it.at("ds_name");
                            //                    int ds_sac = ds_it.at("ds_sac_override");
                            //                    assert (ds_sac >= 0);
                            //                    int ds_sic = ds_it.at("ds_sic_override");
                            //                    assert (ds_sic >= 0);

                            //                    if (!ds_task.hasDataSource("Tracker", ds_sac, ds_sic)) // add if not existing
                            //                    {
                            //                        loginf << "ViewPointsImportTask: import: adding override data source '" << ds_name << "' "
                            //                                                       << ds_sac << "/" << ds_sic;
                            //                        StoredDBODataSource& new_ds = ds_task.addNewStoredDataSource("Tracker");
                            //                        new_ds.name(ds_name);
                            //                        new_ds.sac(ds_sac);
                            //                        new_ds.sic(ds_sic);
                            //                    }
                            //                    else // set name if existing
                            //                    {
                            //                        loginf << "ViewPointsImportTask: import: setting override data source '" << ds_name << "' "
                            //                                                       << ds_sac << "/" << ds_sic;
                            //                        StoredDBODataSource& ds = ds_task.getDataSource("Tracker", ds_sac, ds_sic);
                            //                        ds.name(ds_name);
                            //                    }
                }
                else
                {
                    loginf << "ViewPointsImportTask: import: override information not set";
                    asterix_importer_task.overrideTodActive(false);
                }
                asterix_importer_task.importFilename(filename);

                assert(asterix_importer_task.canRun());
                asterix_importer_task.showDoneSummary(false);

                //widget->runCurrentTaskSlot();
                loginf << "ViewPointsImportTask: import: running task";
                asterix_importer_task.run();

                while (!asterix_importer_task.done())
                {
                    QCoreApplication::processEvents();
                    QThread::msleep(1);
                }

                loginf << "ViewPointsImportTask: import: importing dataset '" << name << "' done";

            }

            //task_manager_.appendSuccess("ViewPointsImportTask: import of ASTERIX files done");

            done_ = true;

            emit doneSignal(name_);
        }
    }

    loginf << "ViewPointsImportTask: done";
}
void ViewPointsImportTask::stop()
{

}

const nlohmann::json& ViewPointsImportTask::currentData() const
{
    return current_data_;
}

