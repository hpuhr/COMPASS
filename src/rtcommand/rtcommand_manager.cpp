#include "rtcommand_manager.h"
#include "tcpserver.h"
#include "stringconv.h"

#include "logger.h"

#include <QCoreApplication>

#include <boost/bind.hpp>
#include <boost/thread.hpp>

using namespace std;
using namespace Utils;

RTCommandManager::RTCommandManager()
    : Configurable("RTCommandManager", "RTCommandManager0", 0, "rtcommand.json"),
      stop_requested_(false), stopped_(false)
{
    loginf << "JobManager: constructor";

    registerParameter("port_num", &port_num_, 27960);
}


RTCommandManager::~RTCommandManager() { loginf << "JobManager: destructor"; }


void RTCommandManager::run()
{
    loginf<< "RTCommandManager: run: start";

    loginf<< "RTCommandManager: run: io context";
    boost::asio::io_context io_context;

    TCPServer server (io_context, port_num_);

    loginf<< "RTCommandManager: run: running io context";

    boost::thread t(boost::bind(&boost::asio::io_context::run, &io_context));
    t.detach();

    loginf<< "RTCommandManager: run: starting loop";

    while (1)
    {
        if (stop_requested_) //  && !hasAnyJobs()
            break;

        if (server.hasSession() && server.hasStrData())
        {
            logdbg << "RTCommandManager: run: getting commands";

            std::vector<std::string> cmds = server.getStrData();

            loginf<< "RTCommandManager: run: got " << cmds.size() << " commands '" << String::compress(cmds, ';');

            server.sendStrData("accepted '"+String::compress(cmds, ';')+"'\n");

            //loginf<< "RTCommandManager: run: sent " << cmds.size() << " commands";
        }

        // do commands

        msleep(1);
    }

    io_context.stop();
    assert (io_context.stopped());

    t.timed_join(100);

    stopped_ = true;
    loginf << "RTCommandManager: run: stopped";
}


void RTCommandManager::shutdown()
{
    loginf << "RTCommandManager: shutdown";

    stop_requested_ = true;

    //    if (active_db_job_)
    //        active_db_job_->setObsolete();

    //    for (auto job_it = queued_db_jobs_.unsafe_begin(); job_it != queued_db_jobs_.unsafe_end();
    //         ++job_it)
    //        (*job_it)->setObsolete();

    //    while (hasAnyJobs())
    //    {
    //        loginf << "JobManager: shutdown: waiting on jobs to finish: db " << hasDBJobs()
    //               << " blocking " << hasBlockingJobs() << " non-locking " << hasNonBlockingJobs();

    //        msleep(1000);
    //    }

    while (!stopped_)
    {
        loginf << "RTCommandManager: shutdown: waiting on run stop";
        msleep(1000);
    }

    //    assert(!active_blocking_job_);
    //    assert(blocking_jobs_.empty());

    loginf << "RTCommandManager: shutdown: done";
}
