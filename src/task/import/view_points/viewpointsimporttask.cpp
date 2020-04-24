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

#include "viewpointsimporttask.h"
#include "viewpointsimporttaskwidget.h"
#include "taskmanager.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "files.h"
#include "logger.h"

#include <fstream>

using namespace std;
using namespace nlohmann;
using namespace Utils;

ViewPointsImportTask::ViewPointsImportTask(const std::string& class_id, const std::string& instance_id,
                                           TaskManager& task_manager)
    : Task("ViewPointsImportTask", "Import View Points", true, false, task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_import_view_points.json")
{
    tooltip_ =
            "Allows import of view points and associated datasets. This task can not "
            "be run, but is performed using the 'Import' button.";

    registerParameter("current_filename", &current_filename_, "");

    createSubConfigurables(); // no thing
}

TaskWidget* ViewPointsImportTask::widget()
{
    if (!widget_)
    {
        widget_.reset(new ViewPointsImportTaskWidget(*this));

        //        connect(widget_.get(), &ViewPointsImportTaskWidget::databaseOpenedSignal, this,
        //                &ViewPointsImportTask::databaseOpenedSlot);
        connect(&task_manager_, &TaskManager::expertModeChangedSignal, widget_.get(),
                &ViewPointsImportTaskWidget::expertModeChangedSlot);
    }

    return widget_.get();
}

void ViewPointsImportTask::deleteWidget() { widget_.reset(nullptr); }

void ViewPointsImportTask::generateSubConfigurable(const std::string& class_id,
                                                   const std::string& instance_id)
{
    throw std::runtime_error("ViewPointsImportTask: generateSubConfigurable: unknown class_id " +
                             class_id);
}

bool ViewPointsImportTask::checkPrerequisites()
{
    return ATSDB::instance().interface().ready();  // must be connected
}

bool ViewPointsImportTask::isRecommended()
{
    if (!checkPrerequisites())
        return false;

    if (ATSDB::instance().objectManager().hasData())
        return false; // not recommended if already has data

    return canImport();
}

bool ViewPointsImportTask::isRequired()
{
    return false;
}

std::string ViewPointsImportTask::currentFilename() const
{
    return current_filename_;
}

void ViewPointsImportTask::currentFilename(const std::string& value)
{
    loginf << "ViewPointsImportTask: currentFilename: value '" << value << "'";
    current_filename_ = value;

    parseCurrentFile();
}

std::string ViewPointsImportTask::currentError() const
{
    return current_error_;
}

void ViewPointsImportTask::parseCurrentFile ()
{
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
        return;
    }
}

void ViewPointsImportTask::checkParsedData ()
{
    if (!current_data_.is_object())
    {
        current_error_ = "current data is not an object";
        logerr << "ViewPointsImportTask: checkParsedData: " << current_error_;
        return;
    }

    if (current_data_.contains("view_point_context"))
    {
        json& context = current_data_.at("view_point_context");

        if (context.contains("datasets"))
        {
            if (!context.at("datasets").is_array())
            {
                current_error_ = "datasets is not an array";
                logerr << "ViewPointsImportTask: checkParsedData: " << current_error_;
                return;
            }

            for (auto& ds_it : context.at("datasets").get<json::array_t>())
            {
                if (!ds_it.contains("name") || !ds_it.at("name").is_string())
                {
                    current_error_ = "dataset '"+ds_it.dump()+"' does not contain a valid name";
                    logerr << "ViewPointsImportTask: checkParsedData: " << current_error_;
                    return;
                }

                if (!ds_it.contains("filename") || !ds_it.at("filename").is_string())
                {
                    current_error_ = "dataset '"+ds_it.dump()+"' does not contain a valid filename";
                    logerr << "ViewPointsImportTask: checkParsedData: " << current_error_;
                    return;
                }

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
                               << "' found, re-writing path";

                        ds_it.at("filename") = filename;
                    }
                }

                if (!found)
                {
                    current_error_ = "dataset '"+ds_it.dump()+"' does not contain a usable filename";
                    logerr << "ViewPointsImportTask: checkParsedData: " << current_error_;
                    return;
                }
            }
        }
    }

    if (!current_data_.contains("view_points"))
    {
        current_error_ = "current data does not contain view points";
        logerr << "ViewPointsImportTask: checkParsedData: " << current_error_;
        return;
    }

    json& view_points = current_data_.at("view_points");

    if (!view_points.is_array())
    {
        current_error_ = "view_points is not an array";
        logerr << "ViewPointsImportTask: checkParsedData: " << current_error_;
        return;
    }

    for (auto& vp_it : view_points.get<json::array_t>())
    {
        if (!vp_it.contains("id") || !vp_it.at("id").is_number())
        {
            current_error_ = "view point '"+vp_it.dump()+"' does not contain a valid id";
            logerr << "ViewPointsImportTask: checkParsedData: " << current_error_;
            return;
        }

        if (!vp_it.contains("type") || !vp_it.at("type").is_string())
        {
            current_error_ = "view point '"+vp_it.dump()+"' does not contain a valid type";
            logerr << "ViewPointsImportTask: checkParsedData: " << current_error_;
            return;
        }
    }

    loginf << "ViewPointsImportTask: checkParsedData: current data seems to be valid, contains " << view_points.size()
           << " view points";
}

bool ViewPointsImportTask::canImport ()
{
    return current_error_.size() == 0;
}

void ViewPointsImportTask::import ()
{
    loginf << "ViewPointsImportTask: import";

    if (current_data_.contains("view_point_context"))
    {
        json& context = current_data_.at("view_point_context");

        if (context.contains("datasets"))
        {
            for (auto& ds_it : context.at("datasets").get<json::array_t>())
            {
                std::string name = ds_it.at("name");
                std::string filename = ds_it.at("filename");

            }
        }
    }

    loginf << "ViewPointsImportTask: done";
}
