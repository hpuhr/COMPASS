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

#include <QApplication>
#include <QMessageBox>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <string>

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
    bool reset_config = false;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "produce help message")
            //("compression", po::value<int>(), "set compression level")
            ("reset-config,rc", po::bool_switch(&reset_config), "reset user configuration files")
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

        string system_install_path = SYSTEM_INSTALL_PATH;

#if USE_EXPERIMENTAL_SOURCE == true
        cout <<"ATSDBClient: includes experimental features" << endl;

        const char* appdir = getenv("APPDIR");
        if (appdir)
        {
            cout << "ATSDBClient: assuming fuse environment in " << appdir << endl;
            assert (appdir);

            system_install_path = string(appdir) + "/appdir/atsdb/";

            cout << "ATSDBClient: set install path to '" << system_install_path << "'" << endl;
            assert (Files::directoryExists(system_install_path));


            osgDB::FilePathList path_list;

            path_list.push_back("$ORIGIN/appdir/lib");
            path_list.push_back("$ORIGIN/lib");
            path_list.push_back("appdir/lib");

            osgDB::Registry::instance()->setLibraryFilePathList(string(appdir) + "/appdir/lib");

            string gdal_path = string(appdir)+"/appdir/atsdb/data/gdal";
            CPLSetConfigOption("GDAL_DATA", gdal_path.c_str());
        }
#endif

        cout << "ATSDBClient: checking if local configuration exists ... ";

        bool config_and_data_exists = Files::directoryExists(HOME_SUBDIRECTORY);
        bool config_and_data_copied = false;

        if (config_and_data_exists)
        {
            cout << " yes" << endl;
        }
        else
        {
            cout << " no" << endl;
            copyConfigurationAndData(system_install_path);
            config_and_data_copied = true;
        }

        {
            SimpleConfig config ("main.conf");
            string config_version;

            if (config.existsId("version"))
                config_version = config.getString("version");

            if (config_version != VERSION)
            {
                cout << "ATSDBClient: configuration mismatch detected, local version '" << config_version << "'"
                          << " application version '" << VERSION << "'" << endl;

                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(nullptr, "Upgrade Configuration & Data",
                                              "A configuration & data updade is required, do you want to update now?",
                                              QMessageBox::Yes|QMessageBox::No);
                if (reply == QMessageBox::Yes)
                {
                    cout << "ATSDBClient: config & data upgrade confirmed" << endl;
                    copyConfigurationAndData(system_install_path);
                }
                else
                {
                    cout << "ATSDBClient: config & data upgrade denied" << endl;
                    quit_requested_ = true;
                    return;
                }
            }
        }

        if (reset_config && !config_and_data_copied)
            copyConfiguration(system_install_path);

        cout << "ATSDBClient: opening simple config file at '" << HOME_CONF_DIRECTORY+"main.conf'" << endl;

        SimpleConfig config ("main.conf");
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

bool Client::notify(QObject * receiver, QEvent * event)
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

void Client::copyConfigurationAndData (const string& system_install_path)
{
    if (!Files::directoryExists(system_install_path))
        throw runtime_error ("ATSDBClient: unable to locate system installation files at '"+system_install_path
                             +"'");

    cout << "ATSDBClient: copying files from system installation from '" << system_install_path
              << "' to '" << HOME_SUBDIRECTORY <<  "' ... ";

    if (!Files::copyRecursively(system_install_path, HOME_SUBDIRECTORY))
        throw runtime_error ("ATSDBClient: copying files from system installation from '" + system_install_path
                             +"' to '"+HOME_SUBDIRECTORY+" failed");

    cout << " done" << endl;
}

void Client::copyConfiguration (const string& system_install_path)
{
    string system_conf_path = system_install_path+"conf/";
    string home_conf_path = HOME_SUBDIRECTORY+"conf/";

    cout << "ATSDBClient: reset config from from '" << system_conf_path
              << "' to '" << home_conf_path <<  "' ... ";

    if (!Files::copyRecursively(system_conf_path, home_conf_path))
        throw runtime_error ("ATSDBClient: reset config from from '" + system_conf_path
                             +"' to '" + home_conf_path + "' failed");

    cout << " done" << endl;
}

//}
