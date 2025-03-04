/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "jobmanager.h"
#include "job.h"
#include "logger.h"
#include "stringconv.h"

#include <QTimer>
#include <QCoreApplication>
#include <QThreadPool>

using namespace Utils;

/**
 */
void JobManager::AsyncJob::exec()
{
    f    = std::async(std::launch::async, [ this ] { this->job->run(); return true; });
    runs = true;
}

/**
 */
bool JobManager::AsyncJob::done() const
{
    return f.valid();
}

/**
 */
JobManager::JobManager()
    : Configurable("JobManager", "JobManager0", 0, "threads.json"),
      stop_requested_(false), stopped_(false)
{
    logdbg << "JobManager: constructor";
}

/**
 */
JobManager::~JobManager() { logdbg << "JobManager: destructor"; }

/**
 */
void JobManager::addBlockingJob(std::shared_ptr<Job> job)
{
    logdbg << "JobManager: addJob: " << job->name() << " num " << blocking_jobs_.unsafe_size();

    std::shared_ptr<AsyncJob> j(new AsyncJob);
    j->job = job;

    blocking_jobs_.push(j);  // only add, do not start
}

/**
 */
void JobManager::addNonBlockingJob(std::shared_ptr<Job> job)
{
    logdbg << "JobManager: addNonBlockingJob: " << job->name() << " num " << non_blocking_jobs_.unsafe_size();

    std::shared_ptr<AsyncJob> j(new AsyncJob);
    j->job = job;

    non_blocking_jobs_.push(j);  // add and start
    j->exec();
}

/**
 */
void JobManager::addDBJob(std::shared_ptr<Job> job)
{
    std::shared_ptr<AsyncJob> j(new AsyncJob);
    j->job = job;

    queued_db_jobs_.push(j);

    //emit databaseBusy();
}

/**
 */
void JobManager::cancelJob(std::shared_ptr<Job> job) { job->setObsolete(); }

/**
 */
bool JobManager::hasAnyJobs() { return hasBlockingJobs() || hasNonBlockingJobs() || hasDBJobs(); }

/**
 */
bool JobManager::hasBlockingJobs() { return active_blocking_job_ || !blocking_jobs_.empty(); }

/**
 */
bool JobManager::hasNonBlockingJobs()
{
    return active_non_blocking_job_ || !non_blocking_jobs_.empty();
}

/**
 */
bool JobManager::hasDBJobs() { return active_db_job_ || !queued_db_jobs_.empty(); }

/**
 * Creates thread if possible.
 *
 * \exception std::runtime_error if thread already existed
 */
void JobManager::run()
{
    logdbg << "JobManager: run: start";

    //boost::posix_time::ptime log_time_ = boost::posix_time::microsec_clock::local_time();

    while (1)
    {
        if (stop_requested_ && !hasAnyJobs())
            break;

//        changed_ = false;
//        really_update_widget_ = false;

        if (hasBlockingJobs())
            handleBlockingJobs();

        if (hasNonBlockingJobs())
            handleNonBlockingJobs();

        if (hasDBJobs())
            handleDBJobs();

//        if (!stop_requested_ && changed_ && !hasDBJobs())
//            emit databaseIdle();

        // if (QCoreApplication::hasPendingEvents())
        //     QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        // else
        msleep(1);

//        if ((boost::posix_time::microsec_clock::local_time() - log_time_).total_seconds() > 1)
//        {
//            loginf << "JobManager: run: jobs " << numJobs() << " db " << numDBJobs();
//            log_time_ = boost::posix_time::microsec_clock::local_time();
//        }
    }

    assert(!hasBlockingJobs());
    assert(!hasNonBlockingJobs());
    assert(!hasDBJobs());

    stopped_ = true;
    loginf << "JobManager: run: stopped";
}

/**
 */
void JobManager::handleBlockingJobs()
{
    if (active_blocking_job_)  // see if active one exists, finalize if possible
    {
        if (active_blocking_job_->done())
        {
//            if (active_blocking_job_->obsolete())
//            {
//                logdbg << "JobManager: run: flushing obsolete blocking job";

//                if (!stop_requested_)
//                    active_blocking_job_->emitObsolete();
//            }

            logdbg << "JobManager: run: flushing blocking done job";

            if (!stop_requested_)
            {
                active_blocking_job_->job->emitDone();
                logdbg << "JobManager: run: done blocking job emitted " + active_blocking_job_->job->name();
            }

            active_blocking_job_ = nullptr;
        }
    }

    if (!active_blocking_job_ && !blocking_jobs_.empty())
    {
        if (blocking_jobs_.try_pop(active_blocking_job_))
        {
            assert(!active_blocking_job_->runs && !active_blocking_job_->job->started());
            active_blocking_job_->exec();

            //changed_ = true;
            //really_update_widget_ = !hasBlockingJobs();
        }
    }
}

/**
 */
void JobManager::handleNonBlockingJobs()
{
    while (1)
    {
        if (active_non_blocking_job_)  // see if active one exists
        {
            if (active_non_blocking_job_->done())  // can be finalized
            {
//                if (active_non_blocking_job_->obsolete())
//                {
//                    logdbg << "JobManager: run: flushing obsolete non-blocking job";

//                    if (!stop_requested_)
//                        active_non_blocking_job_->emitObsolete();
//                }

                logdbg << "JobManager: run: flushing non-blocking done job";

                if (!stop_requested_)
                {
                    active_non_blocking_job_->job->emitDone();
                    logdbg << "JobManager: run: done non-blocking job emitted " + active_non_blocking_job_->job->name();
                }

                active_non_blocking_job_ = nullptr;
            }
            else
                break;  // active not done, quit loop
        }

        assert(!active_non_blocking_job_);

        if (!non_blocking_jobs_.empty())
        {
            if (non_blocking_jobs_.try_pop(active_non_blocking_job_))
            {
                // QThreadPool::globalInstance()->start(active_non_blocking_job_.get());

                //changed_ = true;
                //really_update_widget_ = !hasNonBlockingJobs();
            }
        }
        else  // no jobs left
            break;
    }
}

/**
 */
void JobManager::handleDBJobs()
{
    if (active_db_job_)  // see if active one exists
    {
        if (active_db_job_->done())
        {
//            if (active_db_job_->obsolete())
//            {
//                logdbg << "JobManager: run: flushing obsolete non-blocking job";

//                if (!stop_requested_)
//                    active_db_job_->emitObsolete();
//            }

            logdbg << "JobManager: run: flushing non-blocking done job";

            if (!stop_requested_)
            {
                active_db_job_->job->emitDone();
                logdbg << "JobManager: run: done non-blocking job emitted " + active_db_job_->job->name();
            }

            active_db_job_ = nullptr;
        }
    }

    if (!active_db_job_ && !queued_db_jobs_.empty())
    {
        if (queued_db_jobs_.try_pop(active_db_job_))
        {
            active_db_job_->exec();

            //changed_ = true;
            //really_update_widget_ = !hasDBJobs();
        }
    }
}

/**
 */
void JobManager::shutdown()
{
    loginf << "JobManager: shutdown: setting jobs obsolete";

    stop_requested_ = true;

    if (active_db_job_)
        active_db_job_->job->setObsolete();

    for (auto job_it = queued_db_jobs_.unsafe_begin(); job_it != queued_db_jobs_.unsafe_end();
         ++job_it)
        (*job_it)->job->setObsolete();

    if (active_blocking_job_)
        active_blocking_job_->job->setObsolete();

    for (auto job_it = blocking_jobs_.unsafe_begin(); job_it != blocking_jobs_.unsafe_end();
         ++job_it)
        (*job_it)->job->setObsolete();

    if (active_non_blocking_job_)
        active_non_blocking_job_->job->setObsolete();

    for (auto job_it = non_blocking_jobs_.unsafe_begin(); job_it != non_blocking_jobs_.unsafe_end();
         ++job_it)
        (*job_it)->job->setObsolete();

    loginf << "JobManager: shutdown: waiting on jobs to quit";

    while (hasAnyJobs())
    {
        loginf << "JobManager: shutdown: waiting on jobs to finish: db " << hasDBJobs()
               << " blocking " << hasBlockingJobs() << " non-locking " << hasNonBlockingJobs();

        msleep(1000);
    }

    while (!stopped_)
    {
        loginf << "JobManager: shutdown: waiting on run stop";
        //        while (QCoreApplication::hasPendingEvents())
        //            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        msleep(1000);
    }

    assert(!active_blocking_job_);
    assert(blocking_jobs_.empty());

    assert(!active_non_blocking_job_);
    assert(non_blocking_jobs_.empty());

    assert(!active_db_job_);
    assert(queued_db_jobs_.empty());

    loginf << "JobManager: shutdown: done";
}

/**
 */
unsigned int JobManager::numBlockingJobs()
{
    return active_blocking_job_ ? blocking_jobs_.unsafe_size() + 1 : blocking_jobs_.unsafe_size();
}

/**
 */
unsigned int JobManager::numNonBlockingJobs()
{
    return active_non_blocking_job_ ? non_blocking_jobs_.unsafe_size() + 1
                                    : non_blocking_jobs_.unsafe_size();
}

/**
 */
unsigned int JobManager::numDBJobs()
{
    return active_db_job_ ? queued_db_jobs_.unsafe_size() + 1 : queued_db_jobs_.unsafe_size();
}

/**
 */
unsigned int JobManager::numJobs() 
{ 
    return numBlockingJobs() + numNonBlockingJobs(); 
}

/**
 */
int JobManager::numThreads() 
{ 
    return QThreadPool::globalInstance()->activeThreadCount(); 
}
