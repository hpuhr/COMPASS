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

#include "client.h"

#include "compass.h"
#include "config.h"
#include "configurationmanager.h"
#include "files.h"
#include "global.h"
#include "logger.h"
#include "mainwindow.h"
#include "stringconv.h"
#include "taskmanager.h"

#include <QApplication>
#include <QMessageBox>
#include <QSurfaceFormat>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <string>
#include <locale.h>
#include <thread>

#include <tbb/tbb.h>

#if USE_EXPERIMENTAL_SOURCE == true
#include <osgDB/Registry>

#include "cpl_conv.h"
#endif

using namespace std;
using namespace Utils;

namespace po = boost::program_options;

std::string APP_FILENAME;

Client::Client(int& argc, char** argv) : QApplication(argc, argv)
{
    setlocale(LC_ALL, "C");

    APP_FILENAME = argv[0];

    tbb::task_scheduler_init guard(std::thread::hardware_concurrency());

    //    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

//    QSurfaceFormat format;
//    format.setRenderableType(QSurfaceFormat::OpenGL);
//    format.setProfile(QSurfaceFormat::CoreProfile);
//    format.setVersion(3,3);

    //    QSurfaceFormat format;
    //    format.setSwapBehavior(QSurfaceFormat::SwapBehavior::SingleBuffer); //DoubleBuffer
    //    format.setRedBufferSize(8);
    //    format.setGreenBufferSize(8);
    //    format.setBlueBufferSize(8);
    //    format.setAlphaBufferSize(8);
    //    format.setDepthBufferSize(32); //24
    //    format.setStencilBufferSize(8);
    //    format.setSwapInterval(0);
    //    format.setSamples(8);
    // QSurfaceFormat::setDefaultFormat(format);

    std::string create_new_sqlite3_db_filename;
    std::string open_sqlite3_db_filename;
    std::string import_view_points_filename;
#if USE_JASTERIX
    std::string import_asterix_filename;
#endif
    std::string import_json_filename;
    std::string import_json_schema;
    std::string import_gps_trail_filename;
    std::string import_sectors_filename;

    bool auto_process {false};
    bool associate_data {false};

    bool start {false};
    bool load_data {false};
    std::string export_view_points_report_filename;
    bool evaluate {false};
    std::string export_eval_report_filename;
    bool quit {false};

    po::options_description desc("Allowed options");
    desc.add_options()("help", "produce help message")
            ("reset,r", po::bool_switch(&config_and_data_copy_wanted_) ,"reset user configuration and data")
            ("create_new_sqlite3_db", po::value<std::string>(&create_new_sqlite3_db_filename),
             "creates and opens new SQLite3 database with given filename, e.g. '/data/file1.db'")
            ("open_sqlite3_db", po::value<std::string>(&open_sqlite3_db_filename),
             "opens existing SQLite3 database with given filename, e.g. '/data/file1.db'")
            ("import_view_points", po::value<std::string>(&import_view_points_filename),
             "imports view points JSON file with given filename, e.g. '/data/file1.json'")
        #if USE_JASTERIX
            ("import_asterix", po::value<std::string>(&import_asterix_filename),
             "imports ASTERIX file with given filename, e.g. '/data/file1.ff'")
        #endif
            ("import_json", po::value<std::string>(&import_json_filename),
             "imports JSON file with given filename, e.g. '/data/file1.json'")
            ("json_schema", po::value<std::string>(&import_json_schema),
             "JSON file import schema, e.g. 'jASTERIX', 'OpenSkyNetwork', 'ADSBExchange', 'SDDL'")
            ("import_gps_trail", po::value<std::string>(&import_gps_trail_filename),
             "imports gps trail NMEA with given filename, e.g. '/data/file2.txt'")
            ("import_sectors_json", po::value<std::string>(&import_sectors_filename),
             "imports exported sectors JSON with given filename, e.g. '/data/sectors.json'")
            ("auto_process", po::bool_switch(&auto_process), "start automatic processing of imported data")
            ("associate_data", po::bool_switch(&associate_data), "associate target reports")
            ("start", po::bool_switch(&start), "start after finishing previous steps")
            ("load_data", po::bool_switch(&load_data), "load data after start")
            ("export_view_points_report", po::value<std::string>(&export_view_points_report_filename),
             "export view points report after start with given filename, e.g. '/data/db2/report.tex")
            ("evaluate", po::bool_switch(&evaluate), "run evaluation")
            ("export_eval_report", po::value<std::string>(&export_eval_report_filename),
             "export evaluation report after start with given filename, e.g. '/data/eval_db2/report.tex")
            ("quit", po::bool_switch(&quit), "quit after finishing all previous steps");

    try
    {
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help"))
        {
            cout << desc << "\n";
            quit_requested_ = true;
            return;
        }
    }
    catch (exception& e)
    {
        throw runtime_error("COMPASSClient: unable to parse command line parameters: " +
                            string(e.what()));
    }

    checkAndSetupConfig();

    if (quit_requested_)
        return;

    if (import_json_filename.size() && !import_json_schema.size())
    {
        loginf << "COMPASSClient: schema name must be set for JSON import";
        return;
    }

    loginf << "COMPASSClient: started with " << std::thread::hardware_concurrency() << " threads";

    TaskManager& task_man = COMPASS::instance().taskManager();

    if (create_new_sqlite3_db_filename.size())
        task_man.createAndOpenNewSqlite3DB(create_new_sqlite3_db_filename);

    if (open_sqlite3_db_filename.size())
        task_man.openSqlite3DB(open_sqlite3_db_filename);

    if (import_view_points_filename.size())
        task_man.importViewPointsFile(import_view_points_filename);

#if USE_JASTERIX
    if (import_asterix_filename.size())
        task_man.importASTERIXFile(import_asterix_filename);
#endif

    if (import_json_filename.size())
        task_man.importJSONFile(import_json_filename, import_json_schema);

    if (import_gps_trail_filename.size())
        task_man.importGPSTrailFile(import_gps_trail_filename);

    if (import_sectors_filename.size())
        task_man.importSectorsFile(import_sectors_filename);

    if (auto_process)
        task_man.autoProcess(auto_process);

    if (associate_data)
        task_man.associateData(associate_data);

    if (start)
        task_man.start(start);

    if (load_data)
        task_man.loadData(load_data);

    if (export_view_points_report_filename.size())
        task_man.exportViewPointsReportFile(export_view_points_report_filename);

    if (evaluate)
        task_man.evaluate(true);

    if (export_eval_report_filename.size())
        task_man.exportEvalReportFile(export_eval_report_filename);

    if (quit)
        task_man.quit(quit);

}

Client::~Client()
{
    loginf << "Client: destructor";
}

MainWindow& Client::mainWindow()
{
    if (!main_window_)
        main_window_.reset(new MainWindow());

    assert(main_window_);
    return *main_window_;
}

bool Client::notify(QObject* receiver, QEvent* event)
{
    try
    {
        return QApplication::notify(receiver, event);
    }
    catch (exception& e)
    {
        logerr << "COMPASSClient: Exception thrown: " << e.what();
        // assert (false);
        QMessageBox::critical(nullptr, "COMPASSClient: notify: exception", QString(e.what()));
    }
    catch (...)
    {
        // assert (false);
        QMessageBox::critical(nullptr, "COMPASSClient: notify: exception", "Unknown exception");
    }
    return false;
}

bool Client::quitRequested() const { return quit_requested_; }

void Client::checkAndSetupConfig()
{
    // check if basic configuration works
    try
    {
        cout << "COMPASSClient: setting directory paths" << endl;

        system_install_path_ = SYSTEM_INSTALL_PATH;

#if USE_EXPERIMENTAL_SOURCE == true
        cout << "COMPASSClient: includes experimental features" << endl;

        const char* appdir = getenv("APPDIR");
        if (appdir)
        {
            cout << "COMPASSClient: assuming fuse environment in " << appdir << endl;
            assert(appdir);

            system_install_path_ = string(appdir) + "/appdir/compass/";

            cout << "COMPASSClient: set install path to '" << system_install_path_ << "'" << endl;
            assert(Files::directoryExists(system_install_path_));

            osgDB::FilePathList path_list;

            path_list.push_back("$ORIGIN/appdir/lib");
            path_list.push_back("$ORIGIN/lib");
            path_list.push_back("appdir/lib");

            osgDB::Registry::instance()->setLibraryFilePathList(string(appdir) + "/appdir/lib");

            string gdal_path = string(appdir) + "/appdir/compass/data/gdal";
            CPLSetConfigOption("GDAL_DATA", gdal_path.c_str());
        }
#endif

        checkNeededActions();

        performNeededActions();

        cout << "COMPASSClient: opening simple config file at '" << HOME_CONF_DIRECTORY + "main.conf'"
             << endl;

        SimpleConfig config("config.json");
        assert(config.existsId("version"));
        assert(config.existsId("configuration_path"));
        assert(config.existsId("save_config_on_exit"));
        assert(config.existsId("log_properties_file"));
        assert(config.existsId("save_config_on_exit"));

        CURRENT_CONF_DIRECTORY = HOME_CONF_DIRECTORY + config.getString("configuration_path") + "/";

        cout << "COMPASSClient: current configuration path is '" << CURRENT_CONF_DIRECTORY + "'"
             << endl;

        string log_config_path = HOME_CONF_DIRECTORY + config.getString("log_properties_file");
        Files::verifyFileExists(log_config_path);

        cout << "COMPASSClient: initializing logger using '" << log_config_path << "'" << endl;
        Logger::getInstance().init(log_config_path);

        loginf << "COMPASSClient: startup version " << VERSION;
        string config_version = config.getString("version");
        loginf << "COMPASSClient: configuration version " << config_version;

        ConfigurationManager::getInstance().init(config.getString("main_configuration_file"));
    }
    catch (exception& ex)
    {
        logerr << "COMPASSClient: Caught Exception '" << ex.what() << "'";
        logerr.flush();
        // assert (false);

        quit_requested_ = true;
        return;
    }
    catch (...)
    {
        logerr << "COMPASSClient: Caught Exception";
        logerr.flush();
        // assert (false);

        quit_requested_ = true;
        return;
    }
}

void Client::checkNeededActions()
{
    cout << "COMPASSClient: checking if compass home directory exists ... ";

    bool home_subdir_exists = Files::directoryExists(HOME_SUBDIRECTORY);

    if (home_subdir_exists)
        cout << " yes" << endl;
    else
    {
        cout << " no" << endl;

        cout << "COMPASSClient: complete copy into compass home directory needed";
        config_and_data_copy_wanted_ = true;

        return;
    }

    // home subdir exists
    cout << "COMPASSClient: checking if old compass config exists ... ";

    bool old_cfg_exists = Files::fileExists(HOME_SUBDIRECTORY + "conf/config.json");

    if (old_cfg_exists) // complete delete needed
    {
        cout << " yes" << endl;

        cout << "COMPASSClient: complete delete of compass home directory needed";

        home_subdir_deletion_wanted_ = true;
        config_and_data_copy_wanted_ = true;

        return;
    }
    else
        cout << " no" << endl;

    // home subdir exists, no old config exists
    cout << "COMPASSClient: checking if current compass config exists ... ";

    bool current_cfg_subdir_exists = Files::directoryExists(HOME_SUBDIRECTORY)
            && Files::directoryExists(HOME_VERSION_SUBDIRECTORY);

    if (current_cfg_subdir_exists)
    {
        cout << " yes" << endl;

        SimpleConfig config("config.json");
        string config_version;

        if (config.existsId("version"))
            config_version = config.getString("version");

        if (String::compareVersions(VERSION, config_version) != 0)
            cerr << "COMPASSClient: app version '" << VERSION << "' config version " << config_version << "'" << endl;

        assert (String::compareVersions(VERSION, config_version) == 0);  // must be same
        return; // nothing to do
    }
    else
    {
        cout << " no" << endl;

        cout << "COMPASSClient: complete copy into compass home directory needed";
        config_and_data_copy_wanted_ = true;

        return;
    }
}

void Client::performNeededActions()
{
    if (home_subdir_deletion_wanted_)  // version so old it should be deleted before
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(
                    nullptr, "Delete Previous Configuration & Data",
                    "Complete deletion of the previous configuration and data is required. This will delete"
                    " the folder '~/.compass'. Do you want to continue?",
                    QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            cout << "COMPASSClient: config & data delete confirmed, deleting" << endl;

            deleteCompleteHomeSubDir();
        }
        else
        {
            cout << "COMPASSClient: required config & data delete denied" << endl;
            quit_requested_ = true;
            return;
        }
    }

    if (config_and_data_copy_wanted_)
    {
        cout << "COMPASSClient: copying current config & data" << endl;

        copyConfigurationAndData();
    }

}

void Client::deleteCompleteHomeSubDir()
{
    if (!Files::directoryExists(HOME_SUBDIRECTORY))
        throw runtime_error("COMPASSClient: unable to delete directory'" + HOME_SUBDIRECTORY +
                            "'");

    Files::deleteFolder(HOME_SUBDIRECTORY);

    assert(!Files::directoryExists(HOME_SUBDIRECTORY));
}

void Client::copyConfigurationAndData()
{
    if (!Files::directoryExists(system_install_path_))
        throw runtime_error("COMPASSClient: unable to locate system installation files at '" +
                            system_install_path_ + "'");

    if (!Files::directoryExists(OSGEARTH_CACHE_SUBDIRECTORY))
    {
        cout << "COMPASSClient: creating GDAL cache directory '" << OSGEARTH_CACHE_SUBDIRECTORY
             << "'";
        Files::createMissingDirectories(OSGEARTH_CACHE_SUBDIRECTORY);
        assert (Files::directoryExists(OSGEARTH_CACHE_SUBDIRECTORY));
    }

    cout << "COMPASSClient: copying files from system installation from '" << system_install_path_
         << "' to '" << HOME_VERSION_SUBDIRECTORY << "' ... ";

    if (!Files::copyRecursively(system_install_path_, HOME_VERSION_SUBDIRECTORY))
        throw runtime_error("COMPASSClient: copying files from system installation from '" +
                            system_install_path_ + "' to '" + HOME_VERSION_SUBDIRECTORY + " failed");

    cout << " done" << endl;
}

//void Client::copyConfiguration()
//{
//    string system_conf_path = system_install_path_ + "conf/";
//    string home_conf_path = HOME_SUBDIRECTORY + "conf/";

//    cout << "COMPASSClient: reset config from from '" << system_conf_path << "' to '"
//         << home_conf_path << "' ... ";

//    if (!Files::copyRecursively(system_conf_path, home_conf_path))
//        throw runtime_error("COMPASSClient: reset config from from '" + system_conf_path + "' to '" +
//                            home_conf_path + "' failed");

//    cout << " done" << endl;
//}


