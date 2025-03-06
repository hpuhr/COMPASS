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
#include "logger.h"
#include "stringconv.h"

#include <QTimer>
#include <QCoreApplication>
#include <QThreadPool>

using namespace Utils;

/*************************************************************************************
 * JobManagerBase
 *************************************************************************************/

/**
 */
JobManagerBase::JobManagerBase()
:   stop_requested_(false)
,   stopped_       (false)
{
    set_thread_affinity_ = true;
}

/**
 */
JobManagerBase::~JobManagerBase() = default;

/**
 */
void JobManagerBase::addBlockingJob(std::shared_ptr<Job> job)
{
    logdbg << "JobManagerBase: addBlockingJob: " << job->name() << " num " << numBlockingJobs();

    job->setJobID(blocking_ids_++, set_thread_affinity_);

    addBlockingJob_impl(job);
}

/**
 */
void JobManagerBase::addNonBlockingJob(std::shared_ptr<Job> job)
{
    logdbg << "JobManagerBase: addNonBlockingJob: " << job->name() << " num " << numNonBlockingJobs();

    job->setJobID(non_blocking_ids_++, set_thread_affinity_);

    addNonBlockingJob_impl(job);
}

/**
 */
void JobManagerBase::addDBJob(std::shared_ptr<Job> job)
{
    logdbg << "JobManagerBase: addDBJob: " << job->name() << " num " << numDBJobs();

    job->setJobID(db_ids_++, set_thread_affinity_);

    addDBJob_impl(job);

    //emit databaseBusy();
}

/**
 */
void JobManagerBase::cancelJob(std::shared_ptr<Job> job) 
{ 
    job->setObsolete(); 
}

/**
 */
bool JobManagerBase::hasAnyJobs() const
{ 
    return hasBlockingJobs() || hasNonBlockingJobs() || hasDBJobs(); 
}

/**
 */
unsigned int JobManagerBase::numJobs() const
{ 
    return numBlockingJobs() + numNonBlockingJobs() + numDBJobs(); 
}

/**
 * Creates thread if possible.
 *
 * \exception std::runtime_error if thread already existed
 */
void JobManagerBase::run()
{
    logdbg << "JobManagerBase: run: start";

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

    loginf << "JobManagerBase: run: stopped";
}

/**
 */
void JobManagerBase::shutdown()
{
    loginf << "JobManagerBase: shutdown: setting jobs obsolete";

    stop_requested_ = true;

    setJobsObsolete();

    loginf << "JobManagerBase: shutdown: waiting on jobs to quit";

    while (hasAnyJobs())
    {
        loginf << "JobManagerBase: shutdown: waiting on jobs to finish: db " << hasDBJobs()
               << " blocking " << hasBlockingJobs() << " non-locking " << hasNonBlockingJobs();

        msleep(1000);
    }

    while (!stopped_)
    {
        loginf << "JobManagerBase: shutdown: waiting on run stop";
        //        while (QCoreApplication::hasPendingEvents())
        //            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        msleep(1000);
    }

    assert(numJobs() == 0);

    loginf << "JobManager: shutdown: done";
}

/*************************************************************************************
 * JobManagerAsync
 *************************************************************************************/

/**
 */
void JobManagerAsync::AsyncJob::exec()
{
    future_ = std::async(std::launch::async, [ this ] 
    { 
        this->job_->run();
        return true; 
    });

    is_running_ = true;
}

/**
 */
bool JobManagerAsync::AsyncJob::done() const
{
    assert (job_);
    return job_->done(); //future_.valid()
}

/**
 */
JobManagerAsync::JobManagerAsync() = default;

/**
 */
JobManagerAsync::~JobManagerAsync() = default;

/**
 */
void JobManagerAsync::addBlockingJob_impl(std::shared_ptr<Job> job)
{
    std::shared_ptr<AsyncJob> j(new AsyncJob);
    j->job_ = job;

    blocking_jobs_.push(j);  // only add, do not start
}

/**
 */
void JobManagerAsync::addNonBlockingJob_impl(std::shared_ptr<Job> job)
{
    std::shared_ptr<AsyncJob> j(new AsyncJob);
    j->job_ = job;

    non_blocking_jobs_.push(j);  // add and start
    j->exec();
}

/**
 */
void JobManagerAsync::addDBJob_impl(std::shared_ptr<Job> job)
{
    std::shared_ptr<AsyncJob> j(new AsyncJob);
    j->job_ = job;

    queued_db_jobs_.push(j);

    //emit databaseBusy();
}

/**
 */
bool JobManagerAsync::hasBlockingJobs() const
{ 
    return active_blocking_job_ || !blocking_jobs_.empty(); 
}

/**
 */
bool JobManagerAsync::hasNonBlockingJobs() const
{
    return active_non_blocking_job_ || !non_blocking_jobs_.empty();
}

/**
 */
bool JobManagerAsync::hasDBJobs() const
{ 
    return active_db_job_ || !queued_db_jobs_.empty(); 
}

/**
 */
unsigned int JobManagerAsync::numBlockingJobs() const
{
    return active_blocking_job_ ? blocking_jobs_.unsafe_size() + 1 : blocking_jobs_.unsafe_size();
}

/**
 */
unsigned int JobManagerAsync::numNonBlockingJobs() const
{
    return active_non_blocking_job_ ? non_blocking_jobs_.unsafe_size() + 1
                                    : non_blocking_jobs_.unsafe_size();
}

/**
 */
unsigned int JobManagerAsync::numDBJobs() const
{
    return active_db_job_ ? queued_db_jobs_.unsafe_size() + 1 : queued_db_jobs_.unsafe_size();
}

/**
 */
int JobManagerAsync::numThreads() const
{ 
    return QThreadPool::globalInstance()->activeThreadCount(); 
}

/**
 */
void JobManagerAsync::handleBlockingJobs()
{
    if (active_blocking_job_)  // see if active one exists, finalize if possible
    {
        if (active_blocking_job_->done())
        {
//            if (active_blocking_job_->obsolete())
//            {
//                logdbg << "JobManagerAsync: run: flushing obsolete blocking job";

//                if (!stop_requested_)
//                    active_blocking_job_->emitObsolete();
//            }

            loginf << "JobManagerAsync: run: flushing blocking done job";

            if (!stop_requested_)
            {
                active_blocking_job_->job_->emitDone();
                loginf << "JobManagerAsync: run: blocking job " << active_blocking_job_->job_->name() << " emitted done ";
            }

            active_blocking_job_ = nullptr;
        }
    }

    if (!active_blocking_job_ && !blocking_jobs_.empty())
    {
        if (blocking_jobs_.try_pop(active_blocking_job_))
        {
            loginf << "JobManagerAsync: run: running blocking job";

            assert(!active_blocking_job_->is_running_ && !active_blocking_job_->job_->started());
            active_blocking_job_->exec();

            //changed_ = true;
            //really_update_widget_ = !hasBlockingJobs();
        }
    }
}

/**
 */
void JobManagerAsync::handleNonBlockingJobs()
{
    while (1)
    {
        if (active_non_blocking_job_)  // see if active one exists
        {
            if (active_non_blocking_job_->done())  // can be finalized
            {
//                if (active_non_blocking_job_->obsolete())
//                {
//                    logdbg << "JobManagerAsync: run: flushing obsolete non-blocking job";

//                    if (!stop_requested_)
//                        active_non_blocking_job_->emitObsolete();
//                }

                logdbg << "JobManagerAsync: run: flushing non-blocking done job";

                if (!stop_requested_)
                {
                    active_non_blocking_job_->job_->emitDone();
                    logdbg << "JobManagerAsync: run: done non-blocking job emitted " + active_non_blocking_job_->job_->name();
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
void JobManagerAsync::handleDBJobs()
{
    if (active_db_job_)  // see if active one exists
    {
        if (active_db_job_->done())
        {
//            if (active_db_job_->obsolete())
//            {
//                logdbg << "JobManagerAsync: run: flushing obsolete non-blocking job";

//                if (!stop_requested_)
//                    active_db_job_->emitObsolete();
//            }

            logdbg << "JobManagerAsync: run: flushing non-blocking done job";

            if (!stop_requested_)
            {
                active_db_job_->job_->emitDone();
                logdbg << "JobManagerAsync: run: done non-blocking job emitted " + active_db_job_->job_->name();
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
void JobManagerAsync::setJobsObsolete()
{
    if (active_db_job_)
        active_db_job_->job_->setObsolete();

    for (auto job_it = queued_db_jobs_.unsafe_begin(); job_it != queued_db_jobs_.unsafe_end();
         ++job_it)
        (*job_it)->job_->setObsolete();

    if (active_blocking_job_)
        active_blocking_job_->job_->setObsolete();

    for (auto job_it = blocking_jobs_.unsafe_begin(); job_it != blocking_jobs_.unsafe_end();
         ++job_it)
        (*job_it)->job_->setObsolete();

    if (active_non_blocking_job_)
        active_non_blocking_job_->job_->setObsolete();

    for (auto job_it = non_blocking_jobs_.unsafe_begin(); job_it != non_blocking_jobs_.unsafe_end();
         ++job_it)
        (*job_it)->job_->setObsolete();
}

/*************************************************************************************
 * JobManagerThreadPool
 *************************************************************************************/

/**
 */
JobManagerThreadPool::JobManagerThreadPool() = default;

/**
 */
JobManagerThreadPool::~JobManagerThreadPool() = default;

/**
 */
void JobManagerThreadPool::addBlockingJob_impl(std::shared_ptr<Job> job)
{
    blocking_jobs_.push(job);  // only add, do not start
}

/**
 */
void JobManagerThreadPool::addNonBlockingJob_impl(std::shared_ptr<Job> job)
{
    non_blocking_jobs_.push(job);  // add and start
    QThreadPool::globalInstance()->start(job.get());
}

/**
 */
void JobManagerThreadPool::addDBJob_impl(std::shared_ptr<Job> job)
{
    queued_db_jobs_.push(job);
}

/**
 */
bool JobManagerThreadPool::hasBlockingJobs() const
{ 
    return active_blocking_job_ || !blocking_jobs_.empty(); 
}

/**
 */
bool JobManagerThreadPool::hasNonBlockingJobs() const
{
    return active_non_blocking_job_ || !non_blocking_jobs_.empty();
}

/**
 */
bool JobManagerThreadPool::hasDBJobs() const
{ 
    return active_db_job_ || !queued_db_jobs_.empty(); 
}

/**
 */
unsigned int JobManagerThreadPool::numBlockingJobs() const
{
    return active_blocking_job_ ? blocking_jobs_.unsafe_size() + 1 : blocking_jobs_.unsafe_size();
}

/**
 */
unsigned int JobManagerThreadPool::numNonBlockingJobs() const
{
    return active_non_blocking_job_ ? non_blocking_jobs_.unsafe_size() + 1
                                    : non_blocking_jobs_.unsafe_size();
}

/**
 */
unsigned int JobManagerThreadPool::numDBJobs() const
{
    return active_db_job_ ? queued_db_jobs_.unsafe_size() + 1 : queued_db_jobs_.unsafe_size();
}

/**
 */
int JobManagerThreadPool::numThreads() const 
{ 
    return QThreadPool::globalInstance()->activeThreadCount(); 
}

/**
 */
void JobManagerThreadPool::handleBlockingJobs()
{
    if (active_blocking_job_)  // see if active one exists, finalize if possible
    {
        if (active_blocking_job_->done())
        {
//            if (active_blocking_job_->obsolete())
//            {
//                logdbg << "JobManagerThreadPool: run: flushing obsolete blocking job";

//                if (!stop_requested_)
//                    active_blocking_job_->emitObsolete();
//            }

            logdbg << "JobManagerThreadPool: run: flushing blocking done job";

            if (!stop_requested_)
            {
                active_blocking_job_->emitDone();
                logdbg << "JobManagerThreadPool: run: done blocking job emitted " +
                              active_blocking_job_->name();
            }

            active_blocking_job_ = nullptr;
        }
    }

    if (!active_blocking_job_ && !blocking_jobs_.empty())
    {
        if (blocking_jobs_.try_pop(active_blocking_job_))
        {
            assert(!active_blocking_job_->started());
            QThreadPool::globalInstance()->start(active_blocking_job_.get(), QThread::HighPriority);

            //changed_ = true;
            //really_update_widget_ = !hasBlockingJobs();
        }
    }
}

/**
 */
void JobManagerThreadPool::handleNonBlockingJobs()
{
    while (1)
    {
        if (active_non_blocking_job_)  // see if active one exists
        {
            if (active_non_blocking_job_->done())  // can be finalized
            {
//                if (active_non_blocking_job_->obsolete())
//                {
//                    logdbg << "JobManagerThreadPool: run: flushing obsolete non-blocking job";

//                    if (!stop_requested_)
//                        active_non_blocking_job_->emitObsolete();
//                }

                logdbg << "JobManagerThreadPool: run: flushing non-blocking done job";

                if (!stop_requested_)
                {
                    active_non_blocking_job_->emitDone();
                    logdbg << "JobManagerThreadPool: run: done non-blocking job emitted " +
                                  active_non_blocking_job_->name();
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
                // QThreadPool::globalInstance()->start(active_non_blocking_job_.get(), QThread::HighPriority);

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
void JobManagerThreadPool::handleDBJobs()
{
    if (active_db_job_)  // see if active one exists
    {
        if (active_db_job_->done())
        {
//            if (active_db_job_->obsolete())
//            {
//                logdbg << "JobManagerThreadPool: run: flushing obsolete non-blocking job";

//                if (!stop_requested_)
//                    active_db_job_->emitObsolete();
//            }

            logdbg << "JobManagerThreadPool: run: flushing non-blocking done job";

            if (!stop_requested_)
            {
                active_db_job_->emitDone();
                logdbg << "JobManagerThreadPool: run: done non-blocking job emitted " +
                              active_db_job_->name();
            }

            active_db_job_ = nullptr;
        }
    }

    if (!active_db_job_ && !queued_db_jobs_.empty())
    {
        if (queued_db_jobs_.try_pop(active_db_job_))
        {
            QThreadPool::globalInstance()->start(active_db_job_.get(), QThread::HighPriority);

            //changed_ = true;
            //really_update_widget_ = !hasDBJobs();
        }
    }
}

/**
 */
void JobManagerThreadPool::setJobsObsolete()
{
    if (active_db_job_)
        active_db_job_->setObsolete();

    for (auto job_it = queued_db_jobs_.unsafe_begin(); job_it != queued_db_jobs_.unsafe_end();
         ++job_it)
        (*job_it)->setObsolete();

    if (active_blocking_job_)
        active_blocking_job_->setObsolete();

    for (auto job_it = blocking_jobs_.unsafe_begin(); job_it != blocking_jobs_.unsafe_end();
         ++job_it)
        (*job_it)->setObsolete();

    if (active_non_blocking_job_)
        active_non_blocking_job_->setObsolete();

    for (auto job_it = non_blocking_jobs_.unsafe_begin(); job_it != non_blocking_jobs_.unsafe_end();
         ++job_it)
        (*job_it)->setObsolete();
}

/*************************************************************************************
 * JobManager
 *************************************************************************************/

/**
 */
JobManager::JobManager()
:   Configurable("JobManager", "JobManager0", 0, "threads.json")
{
    logdbg << "JobManager: constructor";
}

/**
 */
JobManager::~JobManager() 
{ 
    logdbg << "JobManager: destructor"; 
}
