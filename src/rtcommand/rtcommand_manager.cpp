#include "rtcommand_manager.h"

#include "logger.h"

#include <QCoreApplication>

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

    //boost::posix_time::ptime log_time_ = boost::posix_time::microsec_clock::local_time();

    while (1)
    {
        if (stop_requested_) //  && !hasAnyJobs()
            break;

//        if (hasBlockingJobs())
//            handleBlockingJobs();

        if (QCoreApplication::hasPendingEvents())
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        else
            msleep(1);

    }

    //assert(!hasBlockingJobs());

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
