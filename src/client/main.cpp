
#include <QSurfaceFormat>
#include <QMessageBox>

#include <iostream>
#include <cstdlib>

#include "atsdb.h"
#include "client.h"
#include "mainwindow.h"

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

//void handler(int sig) {
//  void *array[10];
//  size_t size;

//  // get void*'s for all entries on the stack
//  size = backtrace(array, 10);

//  // print out all the frames to stderr
//  fprintf(stderr, "error: signal %d:\n", sig);
//  backtrace_symbols_fd(array, size, STDERR_FILENO);
//  exit(1);
//}

using namespace std;

int main (int argc, char **argv)
{
    //signal(SIGSEGV, handler);   // install our handler

    bool atsdb_initialized = false;

    // real atsdb stuff
    try
    {
        Client mf(argc, argv);

        if (mf.quitRequested())
            return 0;

        ATSDB::instance().initialize();

        atsdb_initialized = true;

        MainWindow window;

        window.show();

        return mf.exec();

    }
    catch (std::exception& ex)
    {
        cerr << "main: caught exception '" << ex.what() << "'" << endl;

        //assert (false);

        if (atsdb_initialized && ATSDB::instance().ready())
            ATSDB::instance().shutdown();

        return -1;
    }
    catch(...)
    {
        cerr  << "main: caught exception" << endl;

        //assert (false);

        if (atsdb_initialized && ATSDB::instance().ready())
            ATSDB::instance().shutdown();

        return -1;
    }
    loginf << "main: shutdown";
    return 0;
}
