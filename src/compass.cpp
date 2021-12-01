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

#include "compass.h"
#include "config.h"
#include "dbinterface.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbtableinfo.h"
#include "filtermanager.h"
#include "global.h"
#include "jobmanager.h"
#include "logger.h"
#include "projectionmanager.h"
#include "taskmanager.h"
#include "viewmanager.h"
#include "evaluationmanager.h"
#include "mainwindow.h"
#include "files.h"

#include <qobject.h>

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
using namespace nlohmann;
using namespace Utils;

COMPASS::COMPASS() : Configurable("COMPASS", "COMPASS0", 0, "compass.json")
{
    logdbg << "COMPASS: constructor: start";

    simple_config_.reset(new SimpleConfig("config.json"));

    registerParameter("last_db_filename", &last_db_filename_, "");
    registerParameter("db_file_list", &db_file_list_, json::array());

    JobManager::instance().start();

    createSubConfigurables();

    assert(db_interface_);
    assert(dbo_manager_);
    assert(filter_manager_);
    assert(task_manager_);
    assert(view_manager_);
    assert(eval_manager_);

//    QObject::connect(db_interface_.get(), &DBInterface::databaseContentChangedSignal,
//                     dbo_manager_.get(), &DBObjectManager::databaseContentChangedSlot,
//                     Qt::QueuedConnection);

//    QObject::connect(dbo_manager_.get(), &DBObjectManager::dbObjectsChangedSignal,
//                     task_manager_.get(), &TaskManager::dbObjectsChangedSlot);
//    QObject::connect(dbo_manager_.get(), &DBObjectManager::schemaChangedSignal, task_manager_.get(),
//                     &TaskManager::schemaChangedSlot);

    QObject::connect(db_interface_.get(), &DBInterface::databaseOpenedSignal,
                     dbo_manager_.get(), &DBObjectManager::databaseOpenedSlot);

    QObject::connect(db_interface_.get(), &DBInterface::databaseOpenedSignal,
                     eval_manager_.get(), &EvaluationManager::databaseOpenedSlot);

    QObject::connect(db_interface_.get(), &DBInterface::databaseClosedSignal,
                     eval_manager_.get(), &EvaluationManager::databaseClosedSlot);

    dbo_manager_->updateSchemaInformationSlot();

    logdbg << "COMPASS: constructor: end";
}

COMPASS::~COMPASS()
{
    logdbg << "COMPASS: destructor: start";

    if (!shut_down_)
    {
        logerr << "COMPASS: destructor: not shut down";
        shutdown();
    }

    assert(!dbo_manager_);
    assert(!db_interface_);
    assert(!filter_manager_);
    assert(!task_manager_);
    assert(!view_manager_);
    assert (!eval_manager_);

    logdbg << "COMPASS: destructor: end";
}

void COMPASS::generateSubConfigurable(const std::string& class_id, const std::string& instance_id)
{
    logdbg << "COMPASS: generateSubConfigurable: class_id " << class_id << " instance_id "
           << instance_id;
    if (class_id == "DBInterface")
    {
        assert(!db_interface_);
        db_interface_.reset(new DBInterface(class_id, instance_id, this));
        assert(db_interface_);
    }
    else if (class_id == "DBObjectManager")
    {
        assert(!dbo_manager_);
        dbo_manager_.reset(new DBObjectManager(class_id, instance_id, this));
        assert(dbo_manager_);
    }
    else if (class_id == "FilterManager")
    {
        assert(!filter_manager_);
        filter_manager_.reset(new FilterManager(class_id, instance_id, this));
        assert(filter_manager_);
    }
    else if (class_id == "TaskManager")
    {
        assert(!task_manager_);
        task_manager_.reset(new TaskManager(class_id, instance_id, this));
        assert(task_manager_);
    }
    else if (class_id == "ViewManager")
    {
        assert(!view_manager_);
        view_manager_.reset(new ViewManager(class_id, instance_id, this));
        assert(view_manager_);
    }
    else if (class_id == "EvaluationManager")
    {
        assert(!eval_manager_);
        eval_manager_.reset(new EvaluationManager(class_id, instance_id, this));
        assert(eval_manager_);
    }
    else
        throw std::runtime_error("COMPASS: generateSubConfigurable: unknown class_id " + class_id);
}

void COMPASS::checkSubConfigurables()
{
    if (!db_interface_)
    {
        addNewSubConfiguration("DBInterface", "DBInterface0");
        generateSubConfigurable("DBInterface", "DBInterface0");
        assert(db_interface_);
    }
    if (!dbo_manager_)
    {
        assert(db_interface_);
        addNewSubConfiguration("DBObjectManager", "DBObjectManager0");
        generateSubConfigurable("DBObjectManager", "DBObjectManager0");
        assert(dbo_manager_);
    }
    if (!filter_manager_)
    {
        addNewSubConfiguration("FilterManager", "FilterManager0");
        generateSubConfigurable("FilterManager", "FilterManager0");
        assert(filter_manager_);
    }
    if (!task_manager_)
    {
        addNewSubConfiguration("TaskManager", "TaskManager0");
        generateSubConfigurable("TaskManager", "TaskManager0");
        assert(task_manager_);
    }
    if (!view_manager_)
    {
        addNewSubConfiguration("ViewManager", "ViewManager0");
        generateSubConfigurable("ViewManager", "ViewManager0");
        assert(view_manager_);
    }
    if (!eval_manager_)
    {
        addNewSubConfiguration("EvaluationManager", "EvaluationManager0");
        generateSubConfigurable("EvaluationManager", "EvaluationManager0");
        assert(eval_manager_);
    }
}

void COMPASS::openDBFile(const std::string& filename)
{
    loginf << "COMPASS: openDBFile: opening file '" << filename << "'";

    assert (!db_opened_);
    assert (db_interface_);

    last_db_filename_ = filename;

    db_interface_->openDBFile(filename, false);
    assert (db_interface_->dbOpen());

    addDBFileToList(filename);

    db_opened_ = true;
}

void COMPASS::createNewDBFile(const std::string& filename)
{
    loginf << "COMPASS: createNewDBFile: creating new file '" << filename << "'";

    assert (!db_opened_);
    assert (db_interface_);

    if (Files::fileExists(filename))
    {
        // confirmation already done by dialog
        loginf << "COMPASS: createNewDBFile: deleting pre-existing file '" << filename << "'";
        Files::deleteFile(filename);
    }

    last_db_filename_ = filename;

    db_interface_->openDBFile(filename, true);
    assert (db_interface_->dbOpen());

    addDBFileToList(filename);

    db_opened_ = true;
}

void COMPASS::closeDB()
{
    loginf << "COMPASS: closeDB: closing db file '" << last_db_filename_ << "'";

    assert (db_opened_);

    dbo_manager_->saveDBDataSources();

    db_interface_->closeDBFile();
    assert (!db_interface_->dbOpen());

    db_opened_ = false;
}


DBInterface& COMPASS::interface()
{
    assert(db_interface_);
    return *db_interface_;
}

DBObjectManager& COMPASS::objectManager()
{
    assert(dbo_manager_);
    return *dbo_manager_;
}

FilterManager& COMPASS::filterManager()
{
    assert(filter_manager_);
    return *filter_manager_;
}

TaskManager& COMPASS::taskManager()
{
    assert(task_manager_);
    return *task_manager_;
}

ViewManager& COMPASS::viewManager()
{
    assert(view_manager_);
    return *view_manager_;
}

SimpleConfig& COMPASS::config()
{
    assert(simple_config_);
    return *simple_config_;
}

EvaluationManager& COMPASS::evaluationManager()
{
    assert(eval_manager_);
    return *eval_manager_;
}

bool COMPASS::dbOpened()
{
    return db_opened_;
}

void COMPASS::shutdown()
{
    loginf << "COMPASS: database shutdown";

    if (shut_down_)
    {
        logerr << "COMPASS: already shut down";
        return;
    }
    assert(db_interface_);

    assert(dbo_manager_);
    if (db_interface_->dbOpen())
        dbo_manager_->saveDBDataSources();
    dbo_manager_ = nullptr;

    JobManager::instance().shutdown();
    ProjectionManager::instance().shutdown();

    assert(eval_manager_);
    eval_manager_->close();
    eval_manager_ = nullptr;

    assert(view_manager_);
    view_manager_->close();
    view_manager_ = nullptr;

    assert(task_manager_);
    task_manager_->shutdown();
    task_manager_ = nullptr;

    assert(filter_manager_);
    filter_manager_ = nullptr;

    if (db_interface_->dbOpen())
        db_interface_->closeDBFile();

    db_interface_ = nullptr;

    //main_window_ = nullptr;

    shut_down_ = true;

    loginf << "COMPASS: shutdown: end";
}

MainWindow& COMPASS::mainWindow()
{
    if (!main_window_)
        main_window_ = new MainWindow();

    assert(main_window_);
    return *main_window_;
}

std::string COMPASS::lastDbFilename() const
{
    return last_db_filename_;
}

std::vector<std::string> COMPASS::dbFileList() const
{
    return db_file_list_.get<std::vector<string>>();
}

void COMPASS::clearDBFileList()
{
    db_file_list_.clear();
}

void COMPASS::addDBFileToList(const std::string filename)
{
    vector<string> tmp_list = db_file_list_.get<std::vector<string>>();

    if (find(tmp_list.begin(), tmp_list.end(), filename) == tmp_list.end())
    {
        loginf << "COMPASS: addDBFileToLost: adding filename '" << filename << "'";

        tmp_list.push_back(filename);

        sort(tmp_list.begin(), tmp_list.end());

        db_file_list_ = tmp_list;
    }
}

