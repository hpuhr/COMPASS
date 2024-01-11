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

    registerParameter("last_path", &last_path_, {});

    if (!Files::directoryExists(last_path_))
        last_path_ = QDir::homePath().toStdString();

    registerParameter("hide_evaluation", &hide_evaluation_, false);
    registerParameter("hide_viewpoints", &hide_viewpoints_, false);

    registerParameter("disable_live_to_offline_switch", &disable_live_to_offline_switch_, false);
    registerParameter("disable_menu_config_save", &disable_menu_config_save_, false);

    registerParameter("disable_osgview_rotate", &disable_osgview_rotate_, false);

    registerParameter("disable_add_remove_views", &disable_add_remove_views_, false);

    registerParameter("auto_live_running_resume_ask_time", &auto_live_running_resume_ask_time_, 60u);
    registerParameter("auto_live_running_resume_ask_wait_time", &auto_live_running_resume_ask_wait_time_, 1u);

    registerParameter("disable_confirm_reset_views", &disable_confirm_reset_views_, false);

    assert (auto_live_running_resume_ask_time_ > 0);
    assert (auto_live_running_resume_ask_wait_time_ > 0);
    assert (auto_live_running_resume_ask_time_ > auto_live_running_resume_ask_wait_time_);

    JobManager::instance().start();
    RTCommandManager::instance().start();

    createSubConfigurables();

    assert(db_interface_);
    assert(dbcontent_manager_);
    assert(ds_manager_);
    assert(filter_manager_);
    assert(task_manager_);
    assert(view_manager_);
    assert(eval_manager_);
    assert (fft_manager_);

    rt_cmd_runner_.reset(new rtcommand::RTCommandRunner);

    // database opending

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
    assert (!eval_manager_);
    assert (!fft_manager_);

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
    else
        throw std::runtime_error("COMPASS: generateSubConfigurable: unknown class_id " + class_id);
}

void COMPASS::checkSubConfigurables()
{
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
}

void COMPASS::openDBFile(const std::string& filename)
{
    loginf << "COMPASS: openDBFile: opening file '" << filename << "'";

    assert (!db_opened_);
    assert (db_interface_);

    last_db_filename_ = filename;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    try
    {
        db_interface_->openDBFile(filename, false);
        assert (db_interface_->dbOpen());

        addDBFileToList(filename);
        lastUsedPath(Files::getDirectoryFromPath(filename));

        db_opened_ = true;

        emit databaseOpenedSignal();

    }  catch (std::exception& e)
    {
        QMessageBox m_warning(QMessageBox::Warning, "Opening Database Failed",
                              e.what(), QMessageBox::Ok);
        m_warning.exec();

        db_opened_ = false;
    }


    QApplication::restoreOverrideCursor();
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
    lastUsedPath(Files::getDirectoryFromPath(filename));

    db_opened_ = true;

    emit databaseOpenedSignal();
}

void COMPASS::exportDBFile(const std::string& filename)
{
    loginf << "COMPASS: exportDBFile: exporting as file '" << filename << "'";

    assert (db_opened_);
    assert (db_interface_);
    assert (!db_export_in_progress_);

    db_export_in_progress_ = true;

    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

    QMessageBox* msg_box = new QMessageBox;

    msg_box->setWindowTitle("Exporting Database");
    msg_box->setText("Please wait ...");
    msg_box->setStandardButtons(QMessageBox::NoButton);
    msg_box->setWindowModality(Qt::ApplicationModal);
    msg_box->show();

    while ((boost::posix_time::microsec_clock::local_time()-start_time).total_milliseconds() < 50)
    {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep(1);
    }

    db_interface_->exportDBFile(filename);
    lastUsedPath(Files::getDirectoryFromPath(filename));


    msg_box->close();
    delete msg_box;

    db_export_in_progress_ = false;
}

void COMPASS::closeDB()
{
    loginf << "COMPASS: closeDB: closing db file '" << last_db_filename_ << "'";

    assert (db_opened_);

    ds_manager_->saveDBDataSources();
    dbcontent_manager_->saveTargets();

    db_interface_->closeDBFile();
    assert (!db_interface_->dbOpen());

    db_opened_ = false;

    emit databaseClosedSignal();
}


DBInterface& COMPASS::interface()
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

rtcommand::RTCommandRunner& COMPASS::rtCmdRunner()
{
    assert(rt_cmd_runner_);
    return *rt_cmd_runner_;
}

bool COMPASS::dbOpened()
{
    return db_opened_;
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
    if (db_interface_->dbOpen())
        ds_manager_->saveDBDataSources();
    ds_manager_ = nullptr;

    assert(fft_manager_);
    if (db_interface_->dbOpen())
        fft_manager_->saveDBFFTs();
    fft_manager_ = nullptr;

    assert(dbcontent_manager_);
    if (db_interface_->dbOpen())
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

    if (db_interface_->dbOpen())
        db_interface_->closeDBFile();

    db_interface_ = nullptr;

    //main_window_ = nullptr;

    //shut down command manager at the end
    RTCommandManager::instance().shutdown();

    loginf << "COMPASS: shutdown: end";
}

MainWindow& COMPASS::mainWindow()
{
    if (!main_window_)
        main_window_ = new MainWindow();

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

bool COMPASS::disableOSGViewRotate() const
{
    return disable_osgview_rotate_;
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
