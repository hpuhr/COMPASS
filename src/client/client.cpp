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
#include "stringconv.h"
#include "taskmanager.h"
#include "asteriximporttask.h"
#include "mainwindow.h"

#include "json.hpp"

#include <QApplication>
#include <QMessageBox>
#include <QSurfaceFormat>
#include <QSplashScreen>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

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

    //    std::string import_json_filename;
    //    std::string import_json_schema;




    //    bool load_data {false};

    //    bool quit {false};

    po::options_description desc("Allowed options");
    desc.add_options()("help", "produce help message")
            ("reset,r", po::bool_switch(&config_and_data_copy_wanted_) ,"reset user configuration and data")
            ("expert_mode", po::bool_switch(&expert_mode_) ,"set expert mode")
            ("create_db", po::value<std::string>(&create_new_sqlite3_db_filename_),
             "creates and opens new SQLite3 database with given filename, e.g. '/data/file1.db'")
            ("open_db", po::value<std::string>(&open_sqlite3_db_filename_),
             "opens existing SQLite3 database with given filename, e.g. '/data/file1.db'")
            ("import_data_sources_file", po::value<std::string>(&import_data_sources_filename_),
             "imports data sources JSON file with given filename, e.g. '/data/ds1.json'")
            ("import_view_points", po::value<std::string>(&import_view_points_filename_),
             "imports view points JSON file with given filename, e.g. '/data/file1.json'")
            ("import_asterix_file", po::value<std::string>(&import_asterix_filename_),
             "imports ASTERIX file with given filename, e.g. '/data/file1.ff'")
            ("import_asterix_file_line", po::value<std::string>(&import_asterix_file_line_),
             "imports ASTERIX file with given line, e.g. 'L2'")
            ("import_asterix_network", po::bool_switch(&import_asterix_network_),
             "imports ASTERIX from defined network UDP streams")
            ("import_asterix_network_time_offset", po::value<std::string>(&import_asterix_network_time_offset_),
             "used time offset during ASTERIX network import, in HH:MM:SS.ZZZ'")
            ("import_asterix_network_max_lines", po::value<int>(&import_asterix_network_max_lines_),
             "maximum number of lines per data source during ASTERIX network import, 1..4'")
            ("asterix_framing", po::value<std::string>(&asterix_framing),
             "sets ASTERIX framing, e.g. 'none', 'ioss', 'ioss_seq', 'rff'")
            ("asterix_decoder_cfg", po::value<std::string>(&asterix_decoder_cfg),
             "sets ASTERIX decoder config using JSON string, e.g. ''{\"10\":{\"edition\":\"0.31\"}}''"
                         " (including one pair of single quotes)")
            //            ("import_json", po::value<std::string>(&import_json_filename),
            //             "imports JSON file with given filename, e.g. '/data/file1.json'")
            //            ("json_schema", po::value<std::string>(&import_json_schema),
            //             "JSON file import schema, e.g. 'jASTERIX', 'OpenSkyNetwork', 'ADSBExchange', 'SDDL'")
            ("import_gps_trail", po::value<std::string>(&import_gps_trail_filename_),
             "imports gps trail NMEA with given filename, e.g. '/data/file2.txt'")
            ("import_sectors_json", po::value<std::string>(&import_sectors_filename_),
             "imports exported sectors JSON with given filename, e.g. '/data/sectors.json'")
            ("associate_data", po::bool_switch(&associate_data_), "associate target reports")
            ("load_data", po::bool_switch(&load_data_), "load data after start")
            ("export_view_points_report", po::value<std::string>(&export_view_points_report_filename_),
             "export view points report after start with given filename, e.g. '/data/db2/report.tex")
            ("evaluate", po::bool_switch(&evaluate_), "run evaluation")
            ("evaluation_parameters", po::value<std::string>(&evaluation_parameters_),
               "evaluation parameters as JSON string, e.g. ''{\"current_standard\": \"test\", \"dbcontent_name_ref\": \"CAT062\", \"dbcontent_name_tst\": \"CAT020\"}'' (including one pair of single quotes)")
            ("evaluate_run_filter", po::bool_switch(&evaluate_run_filter_), "run evaluation filter before evaluation")
            ("export_eval_report", po::value<std::string>(&export_eval_report_filename_),
             "export evaluation report after start with given filename, e.g. '/data/eval_db2/report.tex")
            ("quit", po::bool_switch(&quit_), "quit after finishing all previous steps");

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

    if (import_asterix_filename_.size() && import_asterix_network_)
    {
        logerr << "COMPASSClient: unable to import both ASTERIX file and from network at the same time";
        return;
    }

    if (quit_requested_)
        return;

    //    if (import_json_filename.size() && !import_json_schema.size())
    //    {
    //        loginf << "COMPASSClient: schema name must be set for JSON import";
    //        return;
    //    }

}

void Client::run ()
{
    loginf << "COMPASSClient: started with " << std::thread::hardware_concurrency() << " threads";

    QPixmap pixmap(Files::getImageFilepath("logo.png").c_str());
    QSplashScreen splash(pixmap);
    splash.show();

    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

    while ((boost::posix_time::microsec_clock::local_time() - start_time).total_milliseconds() < 50)
    {
        QCoreApplication::processEvents();
    }

    if (expert_mode_)
        COMPASS::instance().expertMode(true);

    MainWindow& main_window = COMPASS::instance().mainWindow();
    splash.raise();

    main_window.show();
    splash.raise();

    splash.finish(&main_window);

    if (create_new_sqlite3_db_filename_.size())
        main_window.createAndOpenNewSqlite3DB(create_new_sqlite3_db_filename_);

    if (open_sqlite3_db_filename_.size())
        main_window.openSqlite3DB(open_sqlite3_db_filename_);

    if (import_data_sources_filename_.size())
        main_window.importDataSourcesFile(import_data_sources_filename_);

    if (import_view_points_filename_.size())
        main_window.importViewPointsFile(import_view_points_filename_);

    TaskManager& task_man = COMPASS::instance().taskManager();

    try
    {
        if (asterix_framing.size())
        {
            if (asterix_framing == "none")
                task_man.asterixImporterTask().asterixFileFraming("");
            else
                task_man.asterixImporterTask().asterixFileFraming(asterix_framing);
        }

        if (asterix_decoder_cfg.size())
            task_man.asterixImporterTask().asterixDecoderConfig(asterix_decoder_cfg);

        if (import_asterix_file_line_.size())
        {
            unsigned int file_line = String::lineFromStr(import_asterix_file_line_);
            task_man.asterixImporterTask().fileLineID(file_line);
        }

    }
    catch (exception& e)
    {
        logerr << "COMPASSClient: setting ASTERIX options resulted in error: " << e.what();
        quit_requested_ = true;
        return;
    }

    if (import_asterix_filename_.size())
        main_window.importASTERIXFile(import_asterix_filename_);
    else if (import_asterix_network_)
        main_window.importASTERIXFromNetwork();

    if (import_asterix_network_time_offset_.size())
        main_window.importASTERIXFromNetworkTimeOffset(String::timeFromString(import_asterix_network_time_offset_));

    if (import_asterix_network_max_lines_ != -1)
    {
        if (import_asterix_network_max_lines_ < 1 || import_asterix_network_max_lines_ > 4)
        {
            loginf << "COMPASSClient: number of maximum network lines must be between 1 and 4";
            main_window.quit(true);
        }

        main_window.importAsterixNetworkMaxLines(import_asterix_network_max_lines_);
    }

    //    if (import_json_filename.size())
    //        task_man.importJSONFile(import_json_filename, import_json_schema);

    if (import_gps_trail_filename_.size())
        main_window.importGPSTrailFile(import_gps_trail_filename_);

    if (import_sectors_filename_.size())
        main_window.importSectorsFile(import_sectors_filename_);

    if (associate_data_)
        main_window.associateData(associate_data_);

    if (load_data_)
        main_window.loadData(load_data_);

    if (export_view_points_report_filename_.size())
        main_window.exportViewPointsReportFile(export_view_points_report_filename_);

    if (evaluate_)
        main_window.evaluate(true);

    if (evaluate_run_filter_)
        main_window.evaluateRunFilter(true);

    if (export_eval_report_filename_.size())
        main_window.exportEvalReportFile(export_eval_report_filename_);

    if (quit_)
        main_window.quit(quit_);
}

Client::~Client()
{
    loginf << "Client: destructor";
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

        if (evaluation_parameters_.size())
        {
            loginf << "COMPASSClient: overriding evaluation parameters";
            using namespace nlohmann;

            try {
                json eval_config = json::parse(evaluation_parameters_);

                assert (ConfigurationManager::getInstance().hasRootConfiguration(
                            "COMPASS", "COMPASS0"));
                Configuration& compass_config = ConfigurationManager::getInstance().getRootConfiguration(
                            "COMPASS", "COMPASS0");

                assert (compass_config.hasSubConfiguration("EvaluationManager", "EvaluationManager0"));
                Configuration& eval_man_config = compass_config.getSubConfiguration(
                            "EvaluationManager", "EvaluationManager0");

                eval_man_config.overrideJSONParameters(eval_config);
            }
            catch (exception& e)
            {
                logerr << "COMPASSClient: JSON parse error in '" << evaluation_parameters_ << "'";
                throw e;
            }
        }
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


