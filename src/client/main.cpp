#include <iostream>

#include "client.h"
#include "mainwindow.h"
#include "atsdb.h"
#include "taskmanager.h"

#include <QThread>

using namespace std;

int main(int argc, char** argv)
{
    try
    {
        Client client(argc, argv);

        if (client.quitRequested())
            return 0;

        client.mainWindow().show();

        if (ATSDB::instance().taskManager().automaticTasksDefined())
        {
            QThread::msleep(100);

            while (QCoreApplication::hasPendingEvents())
                QCoreApplication::processEvents();

            ATSDB::instance().taskManager().performAutomaticTasks();
        }

        return client.exec();
    }
    catch (std::exception& ex)
    {
        cerr << "main: caught exception '" << ex.what() << "'" << endl;

        return -1;
    }
    catch (...)
    {
        cerr << "main: caught exception" << endl;

        return -1;
    }
}
