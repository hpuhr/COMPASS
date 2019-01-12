
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

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

int main (int argc, char **argv)
{
    signal(SIGSEGV, handler);   // install our handler

    // real atsdb stuff
    try
    {
        Client mf(argc, argv);

        if (mf.quitRequested())
            return 0;

        ATSDB::instance().initialize();

        MainWindow window;

        window.show();

        return mf.exec();

    }
    catch (std::exception& ex)
    {
        logerr  << "main: caught exception '" << ex.what() << "'";
        logerr.flush();

        //assert (false);

        if (ATSDB::instance().ready())
            ATSDB::instance().shutdown();

        return -1;
    }
    catch(...)
    {
        logerr  << "main: caught exception";
        logerr.flush();

        //assert (false);

        if (ATSDB::instance().ready())
            ATSDB::instance().shutdown();

        return -1;
    }
    loginf << "Main: Shutdown";
    return 0;
}
