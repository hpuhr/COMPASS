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
#include "viewpoint.h"

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

    setObjectName("ViewPointsImportTask");
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
        if (!vp_it.contains(VP_ID_KEY) || !vp_it.at(VP_ID_KEY).is_number())
            throw std::runtime_error("view point '"+vp_it.dump()+"' does not contain a valid id");

        if (!vp_it.contains(VP_TYPE_KEY) || !vp_it.at(VP_TYPE_KEY).is_string())
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
    if(db_interface.existsViewPointsTable() && db_interface.viewPoints().size() && show_done_summary_)
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
        assert (vp_it.contains(VP_ID_KEY));

        id = vp_it.at(VP_ID_KEY);

        if (!vp_it.contains(VP_STATUS_KEY))
            vp_it[VP_STATUS_KEY] = "open";

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

                // line
                if (ds_it.contains("line_id"))
                {
                    assert (ds_it.at("line_id").is_number_unsigned());
                    unsigned int line_id = ds_it.at("line_id");

                    loginf << "ViewPointsImportTask: import: line_id " << line_id;

                    asterix_importer_task.fileLineID(line_id);
                }
                else
                    asterix_importer_task.fileLineID(0); // import to L1 if not set


                if (ds_it.contains("time_offset"))
                {
                    assert (ds_it.at("time_offset").is_number());

                    float tod_offset = ds_it.at("time_offset");

                    asterix_importer_task.overrideTodActive(true);
                    asterix_importer_task.overrideTodOffset(tod_offset);
                }
                else
                {
                    loginf << "ViewPointsImportTask: import: override information not set";
                    asterix_importer_task.overrideTodActive(false);
                }

                if (ds_it.contains("date"))
                {
                    assert (ds_it.at("date").is_string());
                    string date_str = ds_it.at("date");

                    loginf << "ViewPointsImportTask: import: date " << date_str;

                    boost::posix_time::ptime date = Time::fromDateString(date_str);

                    asterix_importer_task.date(date);
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

            boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();
            while ((boost::posix_time::microsec_clock::local_time()-start_time).total_milliseconds() < 50)
            {
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                QThread::msleep(1);
            }
        }
    }

    done_ = true;

    emit doneSignal(name_);

    loginf << "ViewPointsImportTask: done";
}
void ViewPointsImportTask::stop()
{

}

const nlohmann::json& ViewPointsImportTask::currentData() const
{
    return current_data_;
}

