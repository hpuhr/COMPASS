#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "mainwindow.h"
#include "logger.h"
#include "client.h"

#include <QThread>

TEST_CASE( "ATSDB MainWindow", "[ATSDB]" )
{
    int argc = 1;
    char* argv[1];

    Client client (argc, argv);

    QThread::msleep(100); //delay

    while (client.hasPendingEvents())
        client.processEvents();

    REQUIRE (!client.quitRequested() );

    client.mainWindow().show();

    QThread::msleep(100); // delay

    while (client.hasPendingEvents())
        client.processEvents();

    QThread::msleep(100); // delay

    client.mainWindow().close();
}
