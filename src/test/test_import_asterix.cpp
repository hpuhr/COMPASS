//#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include "mainwindow.h"
#include "logger.h"
#include "client.h"
#include "files.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "taskmanager.h"
#include "taskmanagerwidget.h"
#include "databaseopentask.h"
#include "sqliteconnectionwidget.h"
#include "asteriximporttask.h"
#include "asteriximporttaskwidget.h"

#include <QThread>

using namespace Utils;

std::string data_path;
std::string filename;

TEST_CASE( "ATSDB Import ASTERIX", "[ATSDB]" )
{
    int argc = 1;
    char* argv[1];

    // create client
    Client client (argc, argv);

    QThread::msleep(100); //delay

    while (client.hasPendingEvents())
        client.processEvents();

    REQUIRE (!client.quitRequested() );

    // create main window
    client.mainWindow().show();
    client.mainWindow().disableConfigurationSaving();

    QThread::msleep(100); // delay

    while (client.hasPendingEvents())
        client.processEvents();

    QThread::msleep(100); // delay

    // create and open sqlite3 database
    // check if data file exists
    std::string recording_filename = data_path+filename;
    std::string db_filename = recording_filename+".db";

    REQUIRE(Files::fileExists(recording_filename));

    if (Files::fileExists(db_filename))
        Files::deleteFile(db_filename);

    REQUIRE(!Files::fileExists(db_filename));

    TaskManager& task_manager = ATSDB::instance().taskManager();
    TaskManagerWidget* task_manager_widget = task_manager.widget();

    DatabaseOpenTask& db_open_task = task_manager.databaseOpenTask();
    db_open_task.useConnection("SQLite Connection");

    SQLiteConnectionWidget* connection_widget = dynamic_cast<SQLiteConnectionWidget*> (
                ATSDB::instance().interface().connectionWidget());
    REQUIRE(connection_widget);

    connection_widget->addFile(db_filename);
    connection_widget->openFileSlot();

    while (client.hasPendingEvents())
        client.processEvents();

    // import asterix
    ASTERIXImportTask& asterix_import_task = task_manager.asterixImporterTask();
    REQUIRE (task_manager_widget->getCurrentTaskName() == asterix_import_task.name());
    REQUIRE (asterix_import_task.isRecommended());

    ASTERIXImportTaskWidget* asterix_import_task_widget = dynamic_cast<ASTERIXImportTaskWidget*> (
                asterix_import_task.widget());
    REQUIRE(asterix_import_task_widget);

    asterix_import_task_widget->addFile(recording_filename);
    REQUIRE(asterix_import_task.canRun());

    task_manager_widget->runCurrentTaskSlot();

    QThread::msleep(100);

    while (!asterix_import_task.done())
        client.processEvents();

    QThread::msleep(1000); // delay

    client.mainWindow().close();
}

int main (int argc, char* argv[])
{
    Catch::Session session;

    // Build a new parser on top of Catch's
    using namespace Catch::clara;
    auto cli = session.cli() // Get Catch's composite command line parser
            | Opt( data_path, "data_path" ) // bind variable to a new option, with a hint string
            ["--data_path"]    // the option names it will respond to
            ("path for data files")
            | Opt( filename, "filename" ) // bind variable to a new option, with a hint string
            ["--filename"]    // the option names it will respond to
            ("filename to use");        // description string for the help output

    // Now pass the new composite back to Catch so it uses that
    session.cli(cli);

    // Let Catch (using Clara) parse the command line
    int returnCode = session.applyCommandLine (argc, argv);
    if( returnCode != 0 ) // Indicates a command line error
        return returnCode;

    if(data_path.size())
        std::cout << "data_path: '" << data_path << "'" << std::endl;
    else
    {
        std::cout << "data_path variable missing" << std::endl;
        return -1;
    }

    if(filename.size())
        std::cout << "filename: '" << filename << "'" << std::endl;
    else
    {
        std::cout << "filename variable missing" << std::endl;
        return -1;
    }

    return session.run();
}
