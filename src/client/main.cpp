#include "client.h"
#include "mainwindow.h"

#include <iostream>

using namespace std;

int main (int argc, char **argv)
{
    try
    {
        Client client (argc, argv);

        if (client.quitRequested())
            return 0;

        client.mainWindow().show();
        return client.exec();
    }
    catch (std::exception& ex)
    {
        cerr << "main: caught exception '" << ex.what() << "'" << endl;

        return -1;
    }
    catch(...)
    {
        cerr  << "main: caught exception" << endl;

        return -1;
    }
}
