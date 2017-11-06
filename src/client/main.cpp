
#include <QSurfaceFormat>

#include "config.h"
#include "logger.h"
#include "configurationmanager.h"

#include "atsdb.h"
#include "client.h"
#include "mainwindow.h"

int main (int argc, char **argv)
{
    try
    {
        SimpleConfig config ("conf/client.conf");
        assert (config.existsId("version"));
        assert (config.existsId("main_configuration_file"));
        assert (config.existsId("log_properties_file"));
        assert (config.existsId("save_config_on_exit"));

        Logger::getInstance().init(config.getString("log_properties_file"));

        loginf << "ATSDBClient: startup version " << config.getString("version") << " using file '" << config.getString("main_configuration_file") << "'";

#ifdef EXPERMENTAL_SRC
        loginf << "ATSDBClient: includes experimental features";
#endif

        ConfigurationManager::getInstance().init (config.getString("main_configuration_file"));
        ATSDB::instance().initialize();

        Client mf(argc, argv);

        QSurfaceFormat fmt;
        fmt.setSamples(4);
        QSurfaceFormat::setDefaultFormat(fmt);

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
