//#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include "mainwindow.h"
#include "logger.h"
#include "client.h"
#include "files.h"

#include <QThread>

using namespace Utils;

std::string data_path;
std::string filename;

TEST_CASE( "ATSDB Import ASTERIX", "[ATSDB]" )
{
    int argc = 1;
    char* argv[1];

    Client client (argc, argv);

    QThread::msleep(100); //delay

    while (client.hasPendingEvents())
        client.processEvents();

    REQUIRE (!client.quitRequested() );

    client.mainWindow().show();
    client.mainWindow().disableConfigurationSaving();

    QThread::msleep(100); // delay

    while (client.hasPendingEvents())
        client.processEvents();

    QThread::msleep(100); // delay

    // check if data file exists
    REQUIRE(Files::fileExists(data_path+filename));

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
