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

#include "global.h"
#include "logger.h"
#include "client.h"
#include "config.h"
#include "logger.h"
#include "files.h"
#include "configurationmanager.h"
#include "stringconv.h"

#include <QApplication>
#include <QMessageBox>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <string>

#include <locale.h>

#if USE_EXPERIMENTAL_SOURCE == true
#include <osgDB/Registry>
#include "cpl_conv.h"
#endif

using namespace std;
using namespace Utils;

namespace po = boost::program_options;

//namespace ATSDB
//{

Client::Client(int& argc, char** argv)
    : QApplication(argc, argv)
{
    setlocale(LC_ALL, "C");

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "produce help message")
            //("compression", po::value<int>(), "set compression level")
            ("reset,r", po::bool_switch(&config_and_data_reset_wanted_), "reset user configuration and data")
            ;

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
        throw runtime_error ("ATSDBClient: unable to parse command line parameters: "+string(e.what()));
    }

    // check if basic configuration works
    try
    {
        cout << "ATSDBClient: setting directory paths" << endl;

        system_install_path_ = SYSTEM_INSTALL_PATH;

#if USE_EXPERIMENTAL_SOURCE == true
        cout <<"ATSDBClient: includes experimental features" << endl;

        const char* appdir = getenv("APPDIR");
        if (appdir)
        {
            cout << "ATSDBClient: assuming fuse environment in " << appdir << endl;
            assert (appdir);

            system_install_path_ = string(appdir) + "/appdir/atsdb/";

            cout << "ATSDBClient: set install path to '" << system_install_path_ << "'" << endl;
            assert (Files::directoryExists(system_install_path_));

            osgDB::FilePathList path_list;

            path_list.push_back("$ORIGIN/appdir/lib");
            path_list.push_back("$ORIGIN/lib");
            path_list.push_back("appdir/lib");

            osgDB::Registry::instance()->setLibraryFilePathList(string(appdir) + "/appdir/lib");

            string gdal_path = string(appdir)+"/appdir/atsdb/data/gdal";
            CPLSetConfigOption("GDAL_DATA", gdal_path.c_str());
        }
#endif

        checkNeededActions ();

        performNeededActions();

        cout << "ATSDBClient: opening simple config file at '" << HOME_CONF_DIRECTORY+"main.conf'" << endl;

        SimpleConfig config ("config.json");
        assert (config.existsId("version"));
        assert (config.existsId("configuration_path"));
        assert (config.existsId("save_config_on_exit"));
        assert (config.existsId("log_properties_file"));
        assert (config.existsId("save_config_on_exit"));

        CURRENT_CONF_DIRECTORY = HOME_CONF_DIRECTORY+config.getString("configuration_path")+"/";

        cout << "ATSDBClient: current configuration path is '" << CURRENT_CONF_DIRECTORY+"'" << endl;

        string log_config_path = HOME_CONF_DIRECTORY+config.getString("log_properties_file");
        Files::verifyFileExists(log_config_path);

        cout << "ATSDBClient: initializing logger using '" << log_config_path << "'" << endl;
        Logger::getInstance().init(log_config_path);

        loginf << "ATSDBClient: startup version " << VERSION;
        string config_version = config.getString("version");
        loginf << "ATSDBClient: configuration version " << config_version;

        ConfigurationManager::getInstance().init (config.getString("main_configuration_file"));
    }
    catch (exception &ex)
    {
        logerr  << "ATSDBClient: Caught Exception '" << ex.what() << "'";
        logerr.flush();
        //assert (false);

        quit_requested_ = true;
        return;
    }
    catch(...)
    {
        logerr  << "ATSDBClient: Caught Exception";
        logerr.flush();
        //assert (false);

        quit_requested_ = true;
        return;
    }

}

bool Client::notify(QObject* receiver, QEvent* event)
{
    try
    {
        return QApplication::notify(receiver, event);
    }
    catch(exception& e)
    {
        logerr  << "Client: Exception thrown: " << e.what();
        //assert (false);
        QMessageBox::critical( NULL, "Client::notify(): Exception", QString( e.what() ) );
    }
    catch(...)
    {
        //assert (false);
        QMessageBox::critical( NULL, "Client::notify(): Exception", "Unknown exception" );
    }
    return false;
}

bool Client::quitRequested() const
{
    return quit_requested_;
}

void Client::checkNeededActions ()
{
    cout << "ATSDBClient: checking if local configuration exists ... ";

    config_and_data_exists_ = Files::directoryExists(HOME_SUBDIRECTORY);

    if (config_and_data_exists_)
        cout << " yes" << endl;
    else
        cout << " no" << endl;

    if (config_and_data_exists_) // check updating actions
    {
        SimpleConfig config ("config.json");
        string config_version;

        if (config.existsId("version"))
            config_version = config.getString("version");

        if (String::compareVersions(VERSION, config_version) != 0) // not same
        {
            upgrade_needed_ = true;

            // 0 if same, -1 if v1 > v2, 1 if v1 < v2
            bool app_version_newer = String::compareVersions(VERSION, config_version) == -1;

            cout << "ATSDBClient: configuration mismatch detected, local version '" << config_version << "'"
                 << (app_version_newer ? " newer" : " older")
                 << " application version '" << VERSION << "'" << endl;

            if (app_version_newer)  // check if cfg version is so old it needs deleting
                config_and_data_deletion_wanted_ =
                        (String::compareVersions(DELETE_CFG_BEFORE_VERSION, config_version) == -1);
            else
                config_and_data_deletion_wanted_ = // check if cfg version is new so it needs deleting
                        (String::compareVersions(VERSION, config_version) == 1);
        }
        else
            cout << "ATSDBClient: same configuration version '" << config_version << "' found" << endl;
    }
}

void Client::performNeededActions ()
{
    if (!config_and_data_exists_) // simple copy of nothing exists
    {
        cout << "ATSDBClient: no previous installation, copying new information and data" << endl;

        copyConfigurationAndData();
        config_and_data_copied_ = true;

        return;
    }

    if (upgrade_needed_ || config_and_data_reset_wanted_)
    {
        if (config_and_data_deletion_wanted_) // version so old it should be deleted before
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(
                        nullptr, "Delete Previous Configuration & Data",
                        "Deletion of the existing configuration and data is required. This will delete"
                        " the folders ~/.atsdb/conf and ~/.atsdb/data. Do you want to continue?",
                        QMessageBox::Yes|QMessageBox::No);

            if (reply == QMessageBox::Yes)
            {
                cout << "ATSDBClient: config & data delete confirmed" << endl;

                deleteConfigurationAndData ();
                copyConfigurationAndData();
                return;
            }
            else
            {
                cout << "ATSDBClient: required config & data delete denied" << endl;
                quit_requested_ = true;
                return;
            }
        }

        // simple upgrade
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(
                    nullptr, "Upgrade Configuration & Data",
                    "A configuration & data updade is required, do you want to update now?",
                    QMessageBox::Yes|QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            cout << "ATSDBClient: config & data upgrade confirmed" << endl;
            copyConfigurationAndData();
        }
        else
        {
            cout << "ATSDBClient: config & data upgrade denied" << endl;
            quit_requested_ = true;
            return;
        }
    }
}

void Client::deleteConfigurationAndData ()
{
    if (!Files::directoryExists(HOME_CONF_DIRECTORY))
        throw runtime_error ("ATSDBClient: unable to delete conf files at '"+HOME_CONF_DIRECTORY+"'");

    if (!Files::directoryExists(HOME_DATA_DIRECTORY))
        throw runtime_error ("ATSDBClient: unable to delete data files at '"+HOME_DATA_DIRECTORY+"'");

    Files::deleteFolder (HOME_CONF_DIRECTORY);
    Files::deleteFolder (HOME_DATA_DIRECTORY);

    assert (!Files::directoryExists(HOME_CONF_DIRECTORY));
    assert (!Files::directoryExists(HOME_DATA_DIRECTORY));
}

void Client::copyConfigurationAndData ()
{
    if (!Files::directoryExists(system_install_path_))
        throw runtime_error ("ATSDBClient: unable to locate system installation files at '"+system_install_path_+"'");

    cout << "ATSDBClient: copying files from system installation from '" << system_install_path_
         << "' to '" << HOME_SUBDIRECTORY <<  "' ... ";

    if (!Files::copyRecursively(system_install_path_, HOME_SUBDIRECTORY))
        throw runtime_error ("ATSDBClient: copying files from system installation from '" + system_install_path_
                             +"' to '"+HOME_SUBDIRECTORY+" failed");

    cout << " done" << endl;
}

void Client::copyConfiguration ()
{
    string system_conf_path = system_install_path_+"conf/";
    string home_conf_path = HOME_SUBDIRECTORY+"conf/";

    cout << "ATSDBClient: reset config from from '" << system_conf_path
         << "' to '" << home_conf_path <<  "' ... ";

    if (!Files::copyRecursively(system_conf_path, home_conf_path))
        throw runtime_error ("ATSDBClient: reset config from from '" + system_conf_path
                             +"' to '" + home_conf_path + "' failed");

    cout << " done" << endl;
}

//}
