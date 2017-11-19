
#include <QSurfaceFormat>

#include <iostream>

#include "global.h"
#include "config.h"
#include "logger.h"
#include "configurationmanager.h"

#include "atsdb.h"
#include "client.h"
#include "mainwindow.h"
#include "files.h"

using namespace Utils;

int main (int argc, char **argv)
{
    // check if basic configuration works
    try
    {
        std::cout << "ATSDBClient: setting directory paths" << std::endl;

        std::cout << "ATSDBClient: checking if local configuration exists ... ";

        if (!Files::directoryExists(HOME_SUBDIRECTORY))
        {
            std::cout << " no" << std::endl;

            if (!Files::directoryExists(SYSTEM_INSTALL_PATH))
            {
                std::cerr << "ATSDBClient: unable to locate system installation files at '" << SYSTEM_INSTALL_PATH
                          << "'" << std::endl;
                return -1;
            }
            std::cout << "ATSDBClient: copying files from system installation from '" << SYSTEM_INSTALL_PATH
                      << "' to '" << HOME_SUBDIRECTORY <<  "' ... ";
            if (!Files::copyRecursively(SYSTEM_INSTALL_PATH, HOME_SUBDIRECTORY))
            {
                std::cout << " failed" << std::endl;
                return -1;
            }
            std::cout << " done" << std::endl;
        }
        else
        {
            std::cout << " yes" << std::endl;
        }

        std::cout << "ATSDBClient: opening simple config file at '" << HOME_CONF_DIRECTORY+"main.conf'" << std::endl;

        SimpleConfig config ("main.conf");
        assert (config.existsId("version"));
        assert (config.existsId("configuration_path"));
        assert (config.existsId("save_config_on_exit"));
        assert (config.existsId("log_properties_file"));
        assert (config.existsId("save_config_on_exit"));

        CURRENT_CONF_DIRECTORY = HOME_CONF_DIRECTORY+config.getString("configuration_path")+"/";

        std::cout << "ATSDBClient: current configuration path is '" << CURRENT_CONF_DIRECTORY+"'" << std::endl;

        std::string log_config_path = HOME_CONF_DIRECTORY+config.getString("log_properties_file");
        Files::verifyFileExists(log_config_path);

        std::cout << "ATSDBClient: initializing logger using '" << log_config_path << "'" << std::endl;
        Logger::getInstance().init(log_config_path);

        loginf << "ATSDBClient: startup version " << VERSION;
        std::string config_version = config.getString("version");
        loginf << "ATSDBClient: configuration version " << config_version;

#if USE_EXPERIMENTAL_SOURCE == true
        loginf << "ATSDBClient: includes experimental features";
#endif

        if (config.existsId("enable_multisampling") && config.getBool("enable_multisampling")
                && config.existsId("multisampling"))
        {
            unsigned int num_samples = config.getUnsignedInt("multisampling");
            loginf << "ATSDBClient: enabling multisampling with " << num_samples << " samples";
            QSurfaceFormat fmt;
            fmt.setSamples(num_samples);
            QSurfaceFormat::setDefaultFormat(fmt);
        }

        ConfigurationManager::getInstance().init (config.getString("main_configuration_file"));
    }
    catch (std::exception &ex)
    {
        logerr  << "Main: Caught Exception '" << ex.what() << "'";
        logerr.flush();

        return -1;
    }
    catch(...)
    {
        logerr  << "Main: Caught Exception";
        logerr.flush();

        return -1;
    }

    // real atsdb stuff
    try
    {
        ATSDB::instance().initialize();

        Client mf(argc, argv);

        MainWindow window;

        window.show();

        return mf.exec();

    }
    catch (std::exception &ex)
    {
        logerr  << "Main: Caught Exception '" << ex.what() << "'";
        logerr.flush();

        if (ATSDB::instance().ready())
            ATSDB::instance().shutdown();

        return -1;
    }
    catch(...)
    {
        logerr  << "Main: Caught Exception";
        logerr.flush();

        if (ATSDB::instance().ready())
            ATSDB::instance().shutdown();

        return -1;
    }
    loginf << "Main: Shutdown";
    return 0;
}
