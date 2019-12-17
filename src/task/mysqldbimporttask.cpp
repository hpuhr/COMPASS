#include "mysqldbimporttask.h"
#include "mysqldbimporttaskwidget.h"
#include "taskmanager.h"
#include "taskmanagerwidget.h"
#include "dbobjectmanager.h"
#include "files.h"
#include "savedfile.h"
#include "logger.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "dbconnection.h"
#include "mysqlppconnection.h"
#include "stringconv.h"

#include <QApplication>

using namespace Utils;
using namespace std;

const std::string DONE_PROPERTY_NAME = "mysql_db_imported";

MySQLDBImportTask::MySQLDBImportTask(const std::string& class_id, const std::string& instance_id,
                                     TaskManager& task_manager)
    : Task("MySQLDBImportTask", "Import MySQL DB", false, false, task_manager),
      Configurable (class_id, instance_id, &task_manager)
{
    registerParameter("current_filename", &current_filename_, "");

    createSubConfigurables();
}

MySQLDBImportTask::~MySQLDBImportTask()
{
    for (auto it : file_list_)
        delete it.second;

    file_list_.clear();
}

void MySQLDBImportTask::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    if (class_id == "File")
    {
        SavedFile *file = new SavedFile (class_id, instance_id, this);
        assert (file_list_.count (file->name()) == 0);
        file_list_.insert (std::pair <std::string, SavedFile*> (file->name(), file));
    }
    else
        throw std::runtime_error ("MySQLDBImportTask: generateSubConfigurable: unknown class_id "+class_id );
}

void MySQLDBImportTask::checkSubConfigurables ()
{
}

QWidget* MySQLDBImportTask::widget()
{
    if (!widget_)
    {
        widget_.reset(new MySQLDBImportTaskWidget (*this));
    }

    assert (widget_);
    return widget_.get();
}

void MySQLDBImportTask::addFile (const std::string& filename)
{
    loginf << "MySQLDBImportTask: addFile: filename '" << filename << "'";

    if (file_list_.count (filename) != 0)
        throw std::invalid_argument ("MySQLDBImportTask: addFile: name '"+filename+"' already in use");

    std::string instancename = filename;
    instancename.erase (std::remove(instancename.begin(), instancename.end(), '/'), instancename.end());

    Configuration &config = addNewSubConfiguration ("File", "File"+instancename);
    config.addParameterString("name", filename);
    generateSubConfigurable ("File", "File"+instancename);

    current_filename_ = filename;

    emit statusChangedSignal(name_);

    if (widget_)
        widget_->updateFileListSlot();
}

void MySQLDBImportTask::removeCurrentFilename ()
{
    loginf << "MySQLDBImportTask: removeCurrentFilename: filename '" << current_filename_ << "'";

    assert (current_filename_.size());
    assert (hasFile(current_filename_));

    if (file_list_.count (current_filename_) != 1)
        throw std::invalid_argument ("MySQLDBImportTask: removeCurrentFilename: name '"
                                     +current_filename_+"' not in use");

    delete file_list_.at(current_filename_);
    file_list_.erase(current_filename_);
    current_filename_ = "";

    emit statusChangedSignal(name_);

    if (widget_)
        widget_->updateFileListSlot();
}

void MySQLDBImportTask::currentFilename (const std::string& filename)
{
    loginf << "MySQLDBImportTask: currentFilename: filename '" << filename << "'";

    current_filename_ = filename;

    emit statusChangedSignal(name_);
}

bool MySQLDBImportTask::checkPrerequisites ()
{
    if (!ATSDB::instance().interface().ready())  // must be connected
        return false;

    if (ATSDB::instance().interface().hasProperty(DONE_PROPERTY_NAME))
        done_ = ATSDB::instance().interface().getProperty(DONE_PROPERTY_NAME) == "1";

    if (ATSDB::instance().interface().connection().type() != MYSQL_IDENTIFIER)
        return false;

    return !ATSDB::instance().objectManager().hasData(); // can not run if data exists
}

bool MySQLDBImportTask::isRecommended ()
{
    return checkPrerequisites();
}

bool MySQLDBImportTask::isRequired ()
{
    return false;
}

void MySQLDBImportTask::deleteWidget ()
{
    widget_.reset(nullptr);
}

bool MySQLDBImportTask::canImportFile ()
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

bool MySQLDBImportTask::canRun()
{
    return canImportFile();
}

void MySQLDBImportTask::run()
{
    loginf << "MySQLDBImportTask: run: filename " << current_filename_;

    task_manager_.appendInfo("MySQLDBImportTask: import of file '"+current_filename_+"' started");

    start_time_ = boost::posix_time::microsec_clock::local_time();

    assert (canImportFile());

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    MySQLppConnection* connection = dynamic_cast<MySQLppConnection*> (&ATSDB::instance().interface().connection());
    assert (connection);

    QString tmp = current_filename_.c_str();

    loginf << "MySQLDBImportTask: run: starting import";

    if (tmp.endsWith(".gz") || tmp.endsWith(".tar") || tmp.endsWith(".zip") || tmp.endsWith(".tgz")
            || tmp.endsWith(".rar"))
        connection->importSQLArchiveFile(current_filename_);
    else
        connection->importSQLFile(current_filename_);

    QApplication::restoreOverrideCursor();

    stop_time_ = boost::posix_time::microsec_clock::local_time();

    boost::posix_time::time_duration diff = stop_time_ - start_time_;

    std::string time_str = String::timeStringFromDouble(diff.total_milliseconds()/1000.0, false);

    task_manager_.appendSuccess("MySQLDBImportTask: done after "+time_str);

    done_ = true;

    ATSDB::instance().interface().setProperty(DONE_PROPERTY_NAME, "1");
    emit doneSignal(name_);
}

