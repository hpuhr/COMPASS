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
#include "dbcontent/dbcontentmanager.h"
#include "datasourcemanager.h"
#include "filtermanager.h"
#include "jobmanager.h"
#include "logger.h"
#include "taskmanager.h"
#include "viewmanager.h"
#include "evaluationmanager.h"
#include "mainwindow.h"
#include "files.h"
#include "asteriximporttask.h"
#include "rtcommand_runner.h"
#include "rtcommand_manager.h"
#include "rtcommand.h"
#include "util/timeconv.h"
#include "fftmanager.h"
#include "util/async.h"
#include "licensemanager.h"
#include "result.h"
#include "dbinstance.h"
#include "resultmanager.h"

#include <QMessageBox>
#include <QApplication>

#include <osgDB/Registry>

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
using namespace nlohmann;
using namespace Utils;

const bool COMPASS::is_app_image_ = {getenv("APPDIR") != nullptr};

COMPASS::COMPASS() : Configurable("COMPASS", "COMPASS0", 0, "compass.json")
{
    logdbg << "COMPASS: constructor: start";

    std::cout << "APPIMAGE: " << (is_app_image_ ? "yes" : "no") << std::endl;

    simple_config_.reset(new SimpleConfig("config.json"));

    registerParameter("last_db_filename", &last_db_filename_, std::string());
    registerParameter("db_file_list", &db_file_list_, json::array());

    vector<string> cleaned_file_list;
    // clean missing files

    for (auto& filename : db_file_list_.get<std::vector<string>>())
    {
        if (Files::fileExists(filename))
            cleaned_file_list.push_back(filename);
    }
    db_file_list_ = cleaned_file_list;

    registerParameter("dark_mode", &dark_mode_, false);

    registerParameter("last_path", &last_path_, {});

    if (!Files::directoryExists(last_path_))
        last_path_ = QDir::homePath().toStdString();

    registerParameter("hide_evaluation", &hide_evaluation_, false);
    registerParameter("hide_viewpoints", &hide_viewpoints_, false);

    registerParameter("disable_live_to_offline_switch", &disable_live_to_offline_switch_, false);
    registerParameter("disable_menu_config_save", &disable_menu_config_save_, false);

    registerParameter("disable_geographicview_rotate", &disable_geographicview_rotate_, false);

    registerParameter("disable_add_remove_views", &disable_add_remove_views_, false);

    registerParameter("auto_live_running_resume_ask_time", &auto_live_running_resume_ask_time_, 60u);
    registerParameter("auto_live_running_resume_ask_wait_time", &auto_live_running_resume_ask_wait_time_, 1u);

    registerParameter("disable_confirm_reset_views", &disable_confirm_reset_views_, false);

    assert (auto_live_running_resume_ask_time_ > 0);
    assert (auto_live_running_resume_ask_wait_time_ > 0);
    assert (auto_live_running_resume_ask_time_ > auto_live_running_resume_ask_wait_time_);

    JobManager::instance().start();
    RTCommandManager::instance().start();

    try
    {
        createSubConfigurables();
    }
    catch(const std::exception& e)
    {
        JobManager::instance().shutdown();
        RTCommandManager::instance().shutdown();
        
        throw std::runtime_error(e.what());
    }

    assert(db_interface_);
    assert(dbcontent_manager_);
    assert(ds_manager_);
    assert(filter_manager_);
    assert(task_manager_);
    assert(view_manager_);
    assert(eval_manager_);
    assert(fft_manager_);
    assert(license_manager_);

    rt_cmd_runner_.reset(new rtcommand::RTCommandRunner);

    // database opening & closing

    QObject::connect(this, &COMPASS::databaseOpenedSignal,
                     dbcontent_manager_.get(), &DBContentManager::databaseOpenedSlot);
    QObject::connect(this, &COMPASS::databaseClosedSignal,
                     dbcontent_manager_.get(), &DBContentManager::databaseClosedSlot);

    QObject::connect(this, &COMPASS::databaseOpenedSignal,
                     ds_manager_.get(), &DataSourceManager::databaseOpenedSlot);
    QObject::connect(this, &COMPASS::databaseClosedSignal,
                     ds_manager_.get(), &DataSourceManager::databaseClosedSlot);

    QObject::connect(this, &COMPASS::databaseOpenedSignal,
                     filter_manager_.get(), &FilterManager::databaseOpenedSlot);
    QObject::connect(this, &COMPASS::databaseClosedSignal,
                     filter_manager_.get(), &FilterManager::databaseClosedSlot);

    QObject::connect(ds_manager_.get(), &DataSourceManager::dataSourcesChangedSignal,
                     filter_manager_.get(), &FilterManager::dataSourcesChangedSlot);

    QObject::connect(this, &COMPASS::databaseOpenedSignal,
                     view_manager_.get(), &ViewManager::databaseOpenedSlot);
    QObject::connect(this, &COMPASS::databaseClosedSignal,
                     view_manager_.get(), &ViewManager::databaseClosedSlot);

    QObject::connect(this, &COMPASS::databaseOpenedSignal,
                     eval_manager_.get(), &EvaluationManager::databaseOpenedSlot);
    QObject::connect(this, &COMPASS::databaseClosedSignal,
                     eval_manager_.get(), &EvaluationManager::databaseClosedSlot);

    QObject::connect(this, &COMPASS::databaseOpenedSignal,
                     fft_manager_.get(), &FFTManager::databaseOpenedSlot);
    QObject::connect(this, &COMPASS::databaseClosedSignal,
                     fft_manager_.get(), &FFTManager::databaseClosedSlot);

    QObject::connect(this, &COMPASS::databaseOpenedSignal,
                     result_manager_.get(), &ResultManager::databaseOpenedSlot);
    QObject::connect(this, &COMPASS::databaseClosedSignal,
                     result_manager_.get(), &ResultManager::databaseClosedSlot);

    // data sources changed
    QObject::connect(ds_manager_.get(), &DataSourceManager::dataSourcesChangedSignal,
                     eval_manager_.get(), &EvaluationManager::dataSourcesChangedSlot); // update if data sources changed

    // data exchange
    QObject::connect(dbcontent_manager_.get(), &DBContentManager::loadingStartedSignal,
                     view_manager_.get(), &ViewManager::loadingStartedSlot);
    QObject::connect(dbcontent_manager_.get(), &DBContentManager::loadedDataSignal,
                     view_manager_.get(), &ViewManager::loadedDataSlot);
    QObject::connect(dbcontent_manager_.get(), &DBContentManager::loadingDoneSignal,
                     view_manager_.get(), &ViewManager::loadingDoneSlot);

    // appmode
    connect (this, &COMPASS::appModeSwitchSignal,
             filter_manager_.get(), &FilterManager::appModeSwitchSlot);

    // features
    connect (license_manager_.get(), &LicenseManager::changed, task_manager_.get(), &TaskManager::updateFeatures);
    connect (license_manager_.get(), &LicenseManager::changed, view_manager_.get(), &ViewManager::updateFeatures);

    qRegisterMetaType<AppMode>("AppMode");

    rtcommand::RTCommandHelp::init();

    appMode(app_mode_);

    logdbg << "COMPASS: constructor: end";
}

COMPASS::~COMPASS()
{
    logdbg << "COMPASS: destructor: start";

    if (app_state_ != AppState::Shutdown)
    {
        logerr << "COMPASS: destructor: not shut down";
        shutdown();
    }

    assert(!dbcontent_manager_);
    assert(!db_interface_);
    assert(!filter_manager_);
    assert(!task_manager_);
    assert(!view_manager_);
    assert(!eval_manager_);
    assert(!fft_manager_);
    assert(!license_manager_);

    logdbg << "COMPASS: destructor: end";
}

void COMPASS::setAppState(AppState state)
{
    app_state_ = state;

    //notify someone about changed app state?
}

std::string COMPASS::getPath() const
{
    return "";
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
    else if (class_id == "DBContentManager")
    {
        assert(!dbcontent_manager_);
        dbcontent_manager_.reset(new DBContentManager(class_id, instance_id, this));
        assert(dbcontent_manager_);
    }
    else if (class_id == "DataSourceManager")
    {
        assert(!ds_manager_);
        ds_manager_.reset(new DataSourceManager(class_id, instance_id, this));
        assert(ds_manager_);
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
    else if (class_id == "FFTManager")
    {
        assert(!fft_manager_);
        fft_manager_.reset(new FFTManager(class_id, instance_id, this));
        assert(fft_manager_);
    }
    else if (class_id == "LicenseManager")
    {
        assert(!license_manager_);
        license_manager_.reset(new LicenseManager(class_id, instance_id, this));
        assert(license_manager_);
    }
    else if (class_id == "ResultManager")
    {
        assert(!result_manager_);
        result_manager_.reset(new ResultManager(class_id, instance_id, this));
        assert(result_manager_);
    }
    else
        throw std::runtime_error("COMPASS: generateSubConfigurable: unknown class_id " + class_id);
}

void COMPASS::checkSubConfigurables()
{
    if (!license_manager_)
    {
        generateSubConfigurableFromConfig("LicenseManager", "LicenseManager0");
        assert(license_manager_);
    }
    if (!db_interface_)
    {
        generateSubConfigurableFromConfig("DBInterface", "DBInterface0");
        assert(db_interface_);
    }
    if (!dbcontent_manager_)
    {
        generateSubConfigurableFromConfig("DBContentManager", "DBContentManager0");
        assert(dbcontent_manager_);
    }
    if (!ds_manager_)
    {
        generateSubConfigurableFromConfig("DataSourceManager", "DataSourceManager0");
        assert(dbcontent_manager_);
    }
    if (!filter_manager_)
    {
        generateSubConfigurableFromConfig("FilterManager", "FilterManager0");
        assert(filter_manager_);
    }
    if (!task_manager_)
    {
        generateSubConfigurableFromConfig("TaskManager", "TaskManager0");
        assert(task_manager_);
    }
    if (!view_manager_)
    {
        generateSubConfigurableFromConfig("ViewManager", "ViewManager0");
        assert(view_manager_);
    }
    if (!eval_manager_)
    {
        generateSubConfigurableFromConfig("EvaluationManager", "EvaluationManager0");
        assert(eval_manager_);
    }
    if (!fft_manager_)
    {
        generateSubConfigurableFromConfig("FFTManager", "FFTManager0");
        assert(fft_manager_);
    }
    if (!result_manager_)
    {
        generateSubConfigurableFromConfig("ResultManager", "ResultManager0");
        assert(result_manager_);
    }
}

bool COMPASS::openDBFile(const std::string& filename)
{
    loginf << "COMPASS: openDBFile: opening file '" << filename << "'";

    assert (!db_opened_);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    auto result = openDBFileInternal(filename);

    lastUsedPath(Files::getDirectoryFromPath(filename));

    QApplication::restoreOverrideCursor();

    if (!result.ok())
        QMessageBox::critical(nullptr, "Error", QString::fromStdString(result.error()));
    else
        addDBFileToList(filename);

    return result.ok();
}

Result COMPASS::openDBFileInternal(const std::string& filename)
{
    assert (db_interface_);

    if (dbOpened())
        return Result::failed("Database already open");

    last_db_filename_ = filename;
    db_inmem_ = false;

    Result res = Result::succeeded();

    try
    {
        db_interface_->openDBFile(filename, false);
        assert (db_interface_->ready());

        db_opened_ = true;

        emit databaseOpenedSignal();
    }  
    catch (std::exception& e)
    {
        res = Result::failed(e.what());

        db_opened_ = false;
    }

    return res;
}

bool COMPASS::createNewDBFile(const std::string& filename)
{
    loginf << "COMPASS: createNewDBFile: creating new file '" << filename << "'";

    assert (!db_opened_);

    auto result = createNewDBFileInternal(filename);

    lastUsedPath(Files::getDirectoryFromPath(filename));

    if (!result.ok())
        QMessageBox::critical(nullptr, "Error", QString::fromStdString(result.error()));
    else
        addDBFileToList(filename);

    return result.ok();
}

Result COMPASS::createNewDBFileInternal(const std::string& filename)
{
    assert (db_interface_);

    if (dbOpened())
        return Result::failed("Database already open");

    last_db_filename_ = filename;
    db_inmem_ = false;

    Result res = Result::succeeded();

    try
    {
        db_interface_->openDBFile(filename, true);
        assert (db_interface_->ready());

        db_opened_ = true;

        emit databaseOpenedSignal();
    }
    catch(const std::exception& e)
    {
        res = Result::failed(e.what());

        db_opened_ = false;
    }

    return res;
}

bool COMPASS::createInMemDBFile(const std::string& future_filename)
{
    loginf << "COMPASS: createInMemDBFile: future filename '" << future_filename << "'";

    assert (!db_opened_);

    auto result = createInMemDBFileInternal(future_filename);

    if (!result.ok())
        QMessageBox::critical(nullptr, "Error", QString::fromStdString(result.error()));

    return result.ok();
}

Result COMPASS::createInMemDBFileInternal(const std::string& future_filename)
{
    assert (db_interface_);

    if (dbOpened())
        return Result::failed("Database already open");

    inmem_future_filename_ = future_filename;
    last_db_filename_ = DBInstance::InMemFilename;
    db_inmem_ = true;

    Result res = Result::succeeded();

    try
    {
        db_interface_->openDBInMemory();
        assert (db_interface_->ready());

        db_opened_ = true;

        emit databaseOpenedSignal();
    }
    catch(const std::exception& e)
    {
        res = Result::failed(e.what());

        db_opened_ = false;
    }

    return res;
}

bool COMPASS::createNewDBFileFromMemory()
{
    loginf << "COMPASS: createNewDBFileFromMemory: filename '" << inmem_future_filename_ << "'";

    assert (canCreateDBFileFromMemory());

    QMessageBox* msg_box = new QMessageBox;

    msg_box->setWindowTitle("Exporting Database");
    msg_box->setText("Please wait ...");
    msg_box->setStandardButtons(QMessageBox::NoButton);
    msg_box->setWindowModality(Qt::ApplicationModal);
    msg_box->show();

    Async::waitAndProcessEventsFor(50);
        
    auto result = createNewDBFileFromMemoryInternal();

    //@TODO: filename should be set as last path?

    msg_box->close();
    delete msg_box;

    if (!result.ok())
        QMessageBox::critical(nullptr, "Error", QString::fromStdString(result.error()));
    else
        addDBFileToList(last_db_filename_);
    
    return result.ok();
}

Result COMPASS::createNewDBFileFromMemoryInternal()
{
    //check first
    if (!canCreateDBFileFromMemory())
        return Result::failed("Cannot create database from memory in current state");

    Result res;
    
    //export mem db to file
    res = exportDBFileInternal(inmem_future_filename_);
    if (!res.ok())
        return res;

    //close mem db
    res = closeDBInternal();
    if (!res.ok())
        return res;

    //open exportted file db
    res = openDBFileInternal(inmem_future_filename_);
    if (!res.ok())
        return res;
    
    return Result::succeeded();
}

bool COMPASS::exportDBFile(const std::string& filename)
{
    loginf << "COMPASS: exportDBFile: exporting as file '" << filename << "'";

    assert (db_opened_);
    assert (!db_export_in_progress_);

    QMessageBox* msg_box = new QMessageBox;

    msg_box->setWindowTitle("Exporting Database");
    msg_box->setText("Please wait ...");
    msg_box->setStandardButtons(QMessageBox::NoButton);
    msg_box->setWindowModality(Qt::ApplicationModal);
    msg_box->show();

    Async::waitAndProcessEventsFor(50);

    auto result = exportDBFileInternal(filename);

    lastUsedPath(Files::getDirectoryFromPath(filename));
    
    msg_box->close();
    delete msg_box;

    if (!result.ok())
        QMessageBox::critical(nullptr, "Error", QString::fromStdString(result.error()));

    return result.ok();
}

Result COMPASS::exportDBFileInternal(const std::string& filename)
{
    assert (db_interface_);

    if (!db_opened_)
        return Result::failed("No database opened");
    if (db_export_in_progress_)
        return Result::failed("Export already in progress");
    
    db_export_in_progress_ = true;

    Result res = Result::succeeded();

    try
    {
        db_interface_->exportDBFile(filename);
    }
    catch(const std::exception& ex)
    {
        res = Result::failed(ex.what());
    }

    db_export_in_progress_ = false;

    return res;
}

bool COMPASS::closeDB()
{
    loginf << "COMPASS: closeDB: closing db file '" << last_db_filename_ << "'";

    assert (db_opened_);

    auto result = closeDBInternal();

    if (!result.ok())
        QMessageBox::critical(nullptr, "Error", QString::fromStdString(result.error()));

    return result.ok();
}

Result COMPASS::closeDBInternal()
{
    if (!db_opened_)
        return Result::failed("No database opened");

    Result res = Result::succeeded();

    try
    {
        ds_manager_->saveDBDataSources();
        dbcontent_manager_->saveTargets();

        db_interface_->closeDB();
        assert (!db_interface_->ready());

        db_opened_ = false;
        db_inmem_ = false;

        emit databaseClosedSignal();
    }
    catch(const std::exception& ex)
    {
        res = Result::failed(ex.what());
    }
    
    return res;
}

DBInterface& COMPASS::dbInterface()
{
    assert(db_interface_);
    return *db_interface_;
}

DBContentManager& COMPASS::dbContentManager()
{
    assert(dbcontent_manager_);
    return *dbcontent_manager_;
}

DataSourceManager& COMPASS::dataSourceManager()
{
    assert(ds_manager_);
    return *ds_manager_;
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

FFTManager& COMPASS::fftManager()
{
    assert(fft_manager_);
    return *fft_manager_;
}

LicenseManager& COMPASS::licenseManager()
{
    assert(license_manager_);
    return *license_manager_;
}

ResultManager& COMPASS::resultManager()
{
    assert(result_manager_);
    return *result_manager_;
}

rtcommand::RTCommandRunner& COMPASS::rtCmdRunner()
{
    assert(rt_cmd_runner_);
    return *rt_cmd_runner_;
}

bool COMPASS::dbOpened()
{
    return db_opened_;
}

bool COMPASS::dbInMem() const
{
    return db_inmem_;
}

bool COMPASS::canCreateDBFileFromMemory() const
{
    return db_opened_ && db_inmem_ && db_interface_ && db_interface_->canCreateDBFileFromMemory() && !inmem_future_filename_.empty();
}

void COMPASS::init()
{
    assert(task_manager_);
    task_manager_->init();
}

void COMPASS::shutdown()
{
    loginf << "COMPASS: database shutdown";

    if (app_state_ == AppState::Shutdown)
    {
        logerr << "COMPASS: already shut down";
        return;
    }

    app_state_ = AppState::Shutdown;

    assert(task_manager_);
    task_manager_->shutdown();
    task_manager_ = nullptr;

    assert(db_interface_);

    assert(ds_manager_);
    if (db_interface_->ready())
        ds_manager_->saveDBDataSources();
    ds_manager_ = nullptr;

    assert(fft_manager_);
    if (db_interface_->ready())
        fft_manager_->saveDBFFTs();
    fft_manager_ = nullptr;

    assert(dbcontent_manager_);
    if (db_interface_->ready())
        dbcontent_manager_->saveTargets();
    dbcontent_manager_ = nullptr;

    JobManager::instance().shutdown();

    assert(eval_manager_);
    eval_manager_->close();
    eval_manager_ = nullptr;

    assert(view_manager_);
    view_manager_->close();
    view_manager_ = nullptr;

    //osgDB::Registry::instance(true);

    assert(filter_manager_);
    filter_manager_ = nullptr;

    if (db_interface_->ready())
        db_interface_->closeDB();

    db_interface_ = nullptr;

    assert(license_manager_);
    license_manager_ = nullptr;

    //main_window_ = nullptr;

    //shut down command manager at the end
    RTCommandManager::instance().shutdown();

    loginf << "COMPASS: shutdown: end";
}

MainWindow& COMPASS::mainWindow()
{
    if (!main_window_)
    {
        main_window_ = new MainWindow();
        
        QObject::connect(dbcontent_manager_.get(), &DBContentManager::loadingStartedSignal,
                        main_window_, &MainWindow::loadingStarted);
        QObject::connect(dbcontent_manager_.get(), &DBContentManager::loadingDoneSignal,
                        main_window_, &MainWindow::loadingDone);
    }

    assert(main_window_);
    return *main_window_;
}

std::string COMPASS::lastUsedPath()
{
    loginf << "COMPASS: lastUsedPath: return '" << last_path_ << "'";
    return last_path_;
}

void COMPASS::lastUsedPath(const std::string& last_path)
{
    loginf << "COMPASS: lastUsedPath: set '" << last_path << "'";
    last_path_ = last_path;
}

bool COMPASS::darkMode() const
{
    return dark_mode_;
}

void COMPASS::darkMode(bool value)
{
    dark_mode_ = value;
}

const char* COMPASS::lineEditInvalidStyle()
{
    if (dark_mode_)
        return "QLineEdit { background: rgb(255, 50, 50); selection-background-color:"
                          " rgb(255, 100, 100); }";
    else
        return "QLineEdit { background: rgb(255, 100, 100); selection-background-color:"
                          " rgb(255, 200, 200); }";
}

bool COMPASS::disableConfirmResetViews() const
{
    return disable_confirm_reset_views_;
}

unsigned int COMPASS::autoLiveRunningResumeAskWaitTime() const
{
    return auto_live_running_resume_ask_wait_time_;
}

unsigned int COMPASS::autoLiveRunningResumeAskTime() const
{
    return auto_live_running_resume_ask_time_;
}

bool COMPASS::dbExportInProgress() const
{
    return db_export_in_progress_;
}

bool COMPASS::disableAddRemoveViews() const
{
    return disable_add_remove_views_;
}

bool COMPASS::disableGeographicViewRotate() const
{
    return disable_geographicview_rotate_;
}

bool COMPASS::disableMenuConfigSave() const
{
    return disable_menu_config_save_;
}

bool COMPASS::disableLiveToOfflineSwitch() const
{
    return disable_live_to_offline_switch_;
}

unsigned int COMPASS::maxFPS() const
{
    return max_fps_;
}

void COMPASS::maxFPS(unsigned int max_fps)
{
    max_fps_ = max_fps;
}

bool COMPASS::hideViewpoints() const
{
    return hide_viewpoints_;
}

bool COMPASS::hideEvaluation() const
{
    return hide_evaluation_;
}

bool COMPASS::isShutDown() const
{
    return (app_state_ == AppState::Shutdown);
}

bool COMPASS::isRunning() const
{
    return (app_state_ == AppState::Running);
}

bool COMPASS::expertMode() const
{
    return expert_mode_;
}

void COMPASS::expertMode(bool expert_mode)
{
    loginf << "COMPASS: expertMode: setting expert mode " << expert_mode;

    expert_mode_ = expert_mode;
}

AppMode COMPASS::appMode() const
{
    return app_mode_;
}

void COMPASS::appMode(const AppMode& app_mode)
{
    if (app_mode_ != app_mode)
    {
        AppMode last_app_mode = app_mode_;

        app_mode_ = app_mode;

        loginf << "COMPASS: appMode: app_mode_current " << toString(app_mode_)
               << " previous " << toString(last_app_mode);

        QMessageBox* msg_box{nullptr};

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        if (last_app_mode == AppMode::LiveRunning && app_mode == AppMode::LivePaused)
        {
            // switch first, load after
            // do manually to be first and avoid trailing inserts
            taskManager().asterixImporterTask().appModeSwitchSlot(last_app_mode, app_mode_);

            emit appModeSwitchSignal(last_app_mode, app_mode_);

            // load all data in db
            msg_box = new QMessageBox;
            assert(msg_box);
            msg_box->setWindowTitle(("Switching to "+toString(app_mode_)).c_str());
            msg_box->setText("Loading data");
            msg_box->setStandardButtons(QMessageBox::NoButton);
            msg_box->show();

            dbcontent_manager_->load();

            while (dbcontent_manager_->loadInProgress())
            {
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                QThread::msleep(10);
            }

            msg_box->close();
            delete msg_box;
        }
        else if (last_app_mode == AppMode::LivePaused && app_mode == AppMode::LiveRunning)
        {
            // load first, switch after to add to existing cache

            msg_box = new QMessageBox;
            assert(msg_box);
            msg_box->setWindowTitle(("Switching to "+toString(app_mode_)).c_str());
            msg_box->setText("Loading data");
            msg_box->setStandardButtons(QMessageBox::NoButton);
            msg_box->show();

            boost::posix_time::ptime min_ts =
                    Time::currentUTCTime() - boost::posix_time::minutes(dbcontent_manager_->maxLiveDataAgeCache());

            string custom_filter = "timestamp >= " + to_string(Time::toLong(min_ts));

            loginf << "COMPASS: appMode: resuming with custom filter load '" << custom_filter << "'";

            dbcontent_manager_->load(custom_filter);

            while (dbcontent_manager_->loadInProgress())
            {
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                QThread::msleep(10);
            }

            assert(msg_box);
            msg_box->close();
            delete msg_box;

            // switch later to add to loaded cache
            taskManager().asterixImporterTask().appModeSwitchSlot(last_app_mode, app_mode_);

            emit appModeSwitchSignal(last_app_mode, app_mode_);
        }
        else
        {
            // just do it
            taskManager().asterixImporterTask().appModeSwitchSlot(last_app_mode, app_mode_);

            emit appModeSwitchSignal(last_app_mode, app_mode_);
        }

        QApplication::restoreOverrideCursor();
    }
}

std::string COMPASS::appModeStr() const
{
    if (!appModes2Strings().count(app_mode_))
    {
        std::cout << "COMPASS: appModeStr: unkown type " << (unsigned int) app_mode_ << std::endl;
        logerr << "COMPASS: appModeStr: unkown type " << (unsigned int) app_mode_;
    }

    assert(appModes2Strings().count(app_mode_) > 0);
    return appModes2Strings().at(app_mode_);
}

const std::map<AppMode, std::string>& COMPASS::appModes2Strings()
{
    static const auto* map = new std::map<AppMode, std::string>
    {{AppMode::Offline, "Offline Mode"},
        {AppMode::LiveRunning, "Live Mode: Running"},
        {AppMode::LivePaused, "Live Mode: Paused"}};

    return *map;
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
        loginf << "COMPASS: addDBFileToList: adding filename '" << filename << "'";

        tmp_list.push_back(filename);

        sort(tmp_list.begin(), tmp_list.end());

        db_file_list_ = tmp_list;
    }
}

std::string COMPASS::versionString(bool open_ats, 
                                   bool license_type) const
{
    assert(COMPASS::instance().config().existsId("version"));
    std::string version = COMPASS::instance().config().getString("version");

    std::string version_str;
    if (open_ats) 
        version_str += "OpenATS ";

    version_str += "COMPASS v" + version;

    if (license_type)
    {
        const auto& license_manager = COMPASS::instance().licenseManager();
        auto vl = license_manager.activeLicense();

        if (vl)
            version_str += " " + license::License::typeToString(vl->type);
        else
            version_str += " " + license::License::typeToString(license::License::Type::Free);
    }

    return version_str;
}

std::string COMPASS::licenseeString(bool licensed_to) const
{
    const auto& license_manager = COMPASS::instance().licenseManager();
    auto vl = license_manager.activeLicense();

    if (!vl)
        return "";

    return (licensed_to ? "Licensed to " : "") + vl->licensee;
}
