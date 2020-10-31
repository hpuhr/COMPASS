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

#include "mysqldbimporttask.h"

#include <QApplication>
#include <QMessageBox>

#include "atsdb.h"
#include "dbconnection.h"
#include "dbinterface.h"
#include "dbobjectmanager.h"
#include "files.h"
#include "jobmanager.h"
#include "logger.h"
#include "mysqldbimportjob.h"
#include "mysqldbimporttaskwidget.h"
#include "mysqlppconnection.h"
#include "savedfile.h"
#include "stringconv.h"
#include "taskmanager.h"
#include "taskmanagerwidget.h"

using namespace Utils;
using namespace std;

const std::string DONE_PROPERTY_NAME = "mysql_db_imported";

MySQLDBImportTask::MySQLDBImportTask(const std::string& class_id, const std::string& instance_id,
                                     TaskManager& task_manager)
    : Task("MySQLDBImportTask", "Import MySQL DB", false, false, task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_import_mysqldb.json")
{
    tooltip_ = "Allows importing of an exported SASS-C Verif job database.";

    qRegisterMetaType<std::string>("std::string");

    registerParameter("current_filename", &current_filename_, "");

    createSubConfigurables();
}

MySQLDBImportTask::~MySQLDBImportTask()
{
    for (auto it : file_list_)
        delete it.second;

    file_list_.clear();
}

void MySQLDBImportTask::generateSubConfigurable(const std::string& class_id,
                                                const std::string& instance_id)
{
    if (class_id == "File")
    {
        SavedFile* file = new SavedFile(class_id, instance_id, this);
        assert(file_list_.count(file->name()) == 0);
        file_list_.insert(std::pair<std::string, SavedFile*>(file->name(), file));
    }
    else
        throw std::runtime_error("MySQLDBImportTask: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

void MySQLDBImportTask::checkSubConfigurables() {}

TaskWidget* MySQLDBImportTask::widget()
{
    if (!widget_)
    {
        widget_.reset(new MySQLDBImportTaskWidget(*this));

        connect(&task_manager_, &TaskManager::expertModeChangedSignal, widget_.get(),
                &MySQLDBImportTaskWidget::expertModeChangedSlot);
    }

    assert(widget_);
    return widget_.get();
}

void MySQLDBImportTask::addFile(const std::string& filename)
{
    loginf << "MySQLDBImportTask: addFile: filename '" << filename << "'";

    if (file_list_.count(filename) != 0)
        throw std::invalid_argument("MySQLDBImportTask: addFile: name '" + filename +
                                    "' already in use");

    std::string instancename = filename;
    instancename.erase(std::remove(instancename.begin(), instancename.end(), '/'),
                       instancename.end());

    Configuration& config = addNewSubConfiguration("File", "File" + instancename);
    config.addParameterString("name", filename);
    generateSubConfigurable("File", "File" + instancename);

    current_filename_ = filename;

    emit statusChangedSignal(name_);

    if (widget_)
        widget_->updateFileListSlot();
}

void MySQLDBImportTask::removeCurrentFilename()
{
    loginf << "MySQLDBImportTask: removeCurrentFilename: filename '" << current_filename_ << "'";

    assert(current_filename_.size());
    assert(hasFile(current_filename_));

    if (file_list_.count(current_filename_) != 1)
        throw std::invalid_argument("MySQLDBImportTask: removeCurrentFilename: name '" +
                                    current_filename_ + "' not in use");

    delete file_list_.at(current_filename_);
    file_list_.erase(current_filename_);
    current_filename_ = "";

    emit statusChangedSignal(name_);

    if (widget_)
        widget_->updateFileListSlot();
}

void MySQLDBImportTask::removeAllFiles ()
{
    loginf << "MySQLDBImportTask: removeAllFiles";

    while (file_list_.size())
    {
        delete file_list_.begin()->second;
        file_list_.erase(file_list_.begin());
    }

    current_filename_ = "";

    emit statusChangedSignal(name_);

    if (widget_)
        widget_->updateFileListSlot();
}

void MySQLDBImportTask::currentFilename(const std::string& filename)
{
    loginf << "MySQLDBImportTask: currentFilename: filename '" << filename << "'";

    current_filename_ = filename;

    emit statusChangedSignal(name_);
}

bool MySQLDBImportTask::checkPrerequisites()
{
    if (!ATSDB::instance().interface().ready())  // must be connected
        return false;

    if (ATSDB::instance().interface().hasProperty(DONE_PROPERTY_NAME))
        done_ = ATSDB::instance().interface().getProperty(DONE_PROPERTY_NAME) == "1";

    if (ATSDB::instance().interface().connection().type() != MYSQL_IDENTIFIER)
        return false;

    return !ATSDB::instance().objectManager().hasData();  // can not run if data exists
}

bool MySQLDBImportTask::isRecommended() { return checkPrerequisites(); }

bool MySQLDBImportTask::isRequired() { return false; }

void MySQLDBImportTask::deleteWidget() { widget_.reset(nullptr); }

bool MySQLDBImportTask::canImportFile()
{
    if (!current_filename_.size())
        return false;

    if (!Files::fileExists(current_filename_))
    {
        loginf << "MySQLDBImportTask: canImportFile: not possible since file '" << current_filename_
               << "'does not exist";
        return false;
    }

    return true;
}

bool MySQLDBImportTask::canRun() { return canImportFile(); }

void MySQLDBImportTask::run()
{
    loginf << "MySQLDBImportTask: run: filename " << current_filename_;

    task_manager_.appendInfo("MySQLDBImportTask: import of file '" + current_filename_ +
                             "' started");

    start_time_ = boost::posix_time::microsec_clock::local_time();

    assert(canImportFile());
    assert(!import_job_);

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    MySQLppConnection* connection =
        dynamic_cast<MySQLppConnection*>(&ATSDB::instance().interface().connection());
    assert(connection);

    QString tmp = current_filename_.c_str();
    bool archive = tmp.endsWith(".gz") || tmp.endsWith(".tar") || tmp.endsWith(".zip") ||
                   tmp.endsWith(".tgz") || tmp.endsWith(".rar");

    import_job_ = make_shared<MySQLDBImportJob>(current_filename_, archive, *connection);

    connect(import_job_.get(), &MySQLDBImportJob::obsoleteSignal, this,
            &MySQLDBImportTask::importObsoleteSlot, Qt::QueuedConnection);
    connect(import_job_.get(), &MySQLDBImportJob::doneSignal, this,
            &MySQLDBImportTask::importDoneSlot, Qt::QueuedConnection);
    connect(import_job_.get(), &MySQLDBImportJob::statusSignal, this,
            &MySQLDBImportTask::importStatusSlot, Qt::QueuedConnection);

    JobManager::instance().addBlockingJob(import_job_);
}

void MySQLDBImportTask::importDoneSlot()
{
    assert(import_job_);

    if (msg_box_)
    {
        delete msg_box_;
        msg_box_ = nullptr;
    }

    size_t num_lines = import_job_->numLines();
    size_t num_errors = import_job_->numErrors();
    bool error_quit = import_job_->quitBecauseOfErrors();

    import_job_ = nullptr;

    QApplication::restoreOverrideCursor();

    stop_time_ = boost::posix_time::microsec_clock::local_time();

    boost::posix_time::time_duration diff = stop_time_ - start_time_;

    std::string time_str = String::timeStringFromDouble(diff.total_milliseconds() / 1000.0, false);

    QMessageBox msg_box;
    std::string message;

    if (error_quit)
    {
        task_manager_.appendError("MySQLDBImportTask: failed because of too many errors after " +
                                  time_str);
        message = "The MySQL DB file import failed because of too many errors.";
    }
    else if (num_errors)
    {
        task_manager_.appendWarning("MySQLDBImportTask: done with " + std::to_string(num_errors) +
                                    " errors after " + time_str);
        message =
            "The MySQL DB file import succeeded with " + std::to_string(num_errors) + " errors.";
    }
    else
    {
        task_manager_.appendSuccess("MySQLDBImportTask: done after " + time_str);
        message = "The MySQL DB file import succeeded with no errors.";
    }

    msg_box.setWindowTitle("Import of MySQL DB Status");
    msg_box.setText(message.c_str());
    msg_box.exec();

    done_ = true;

    ATSDB::instance().interface().setProperty(DONE_PROPERTY_NAME, "1");
    emit doneSignal(name_);
}

void MySQLDBImportTask::importObsoleteSlot()
{
    import_job_ = nullptr;
    QApplication::restoreOverrideCursor();

    done_ = true;
}

void MySQLDBImportTask::importStatusSlot(std::string message)
{
    if (!msg_box_)
    {
        msg_box_ = new QMessageBox();
        msg_box_->setWindowTitle("Import of MySQL DB Status");
        msg_box_->setStandardButtons(QMessageBox::NoButton);
        msg_box_->show();
    }

    msg_box_->setText(message.c_str());
}
