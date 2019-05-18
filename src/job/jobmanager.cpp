/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <qtimer.h>
#include <QThreadPool>
#include <QCoreApplication>

#include "jobmanager.h"
#include "jobmanagerwidget.h"
#include "job.h"
#include "logger.h"
#include "stringconv.h"

using namespace Utils;

JobManager::JobManager()
    : Configurable ("JobManager", "JobManager0", 0, "threads.xml"), stop_requested_(false), stopped_(false),
      widget_(nullptr)
{
    logdbg  << "JobManager: constructor";
}

JobManager::~JobManager()
{
    logdbg  << "JobManager: destructor";
}

void JobManager::addBlockingJob (std::shared_ptr<Job> job)
{
    logdbg << "JobManager: addJob: " << job->name() << " num " << blocking_jobs_.unsafe_size();
    blocking_jobs_.push(job);

    updateWidget();
}

void JobManager::addNonBlockingJob (std::shared_ptr<Job> job)
{
    loginf << "JobManager: addNonBlockingJob: " << job->name() << " num " << non_blocking_jobs_.unsafe_size();
    non_blocking_jobs_.push(job);

    updateWidget();
}

void JobManager::addDBJob (std::shared_ptr<Job> job)
{
    queued_db_jobs_.push(job);

    updateWidget();

    emit databaseBusy();
}


void JobManager::cancelJob (std::shared_ptr<Job> job)
{
    job->setObsolete();
}

bool JobManager::hasAnyJobs()
{
    return hasBlockingJobs() || hasNonBlockingJobs() || hasDBJobs();
}

bool JobManager::hasBlockingJobs ()
{
    return active_blocking_job_ || !blocking_jobs_.empty();
}

bool JobManager::hasNonBlockingJobs ()
{
    return active_non_blocking_job_ || !non_blocking_jobs_.empty();

}

bool JobManager::hasDBJobs()
{
    return active_db_job_ || !queued_db_jobs_.empty();
}

/**
 * Creates thread if possible.
 *
 * \exception std::runtime_error if thread already existed
 */
void JobManager::run()
{
    logdbg  << "JobManager: run: start";

    while (1)
    {
        if (stop_requested_ && !hasAnyJobs())
            break;

        changed_=false;
        really_update_widget_=false;

        if (hasBlockingJobs())
            handleBlockingJobs();

        if (hasNonBlockingJobs())
            handleNonBlockingJobs();

        if (hasDBJobs())
            handleDBJobs();

        if (!stop_requested_ && changed_ && !hasDBJobs())
            emit databaseIdle();

        if (!stop_requested_ && changed_)
            updateWidget(really_update_widget_);

        //QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        msleep(1);
    }

    assert (!hasBlockingJobs());
    assert (!hasNonBlockingJobs());
    assert (!hasDBJobs());

    stopped_=true;
    loginf  << "JobManager: run: stopped";
}

void JobManager::handleBlockingJobs ()
{
    if (active_blocking_job_) // see if active one exists
    {
        if (active_blocking_job_->obsolete() || active_blocking_job_->done())
        {
            if(active_blocking_job_->obsolete())
            {
                logdbg << "JobManager: run: flushing obsolete blocking job";

                if (!stop_requested_)
                    active_blocking_job_->emitObsolete();
            }

            logdbg << "JobManager: run: flushing blocking done job";

            if (!stop_requested_)
            {
                active_blocking_job_->emitDone();
                logdbg << "JobManager: run: done blocking job emitted "+active_blocking_job_->name();
            }

            active_blocking_job_ = nullptr;
        }
    }

    if (!active_blocking_job_ && !blocking_jobs_.empty())
    {
        if (blocking_jobs_.try_pop(active_blocking_job_))
        {
            QThreadPool::globalInstance()->start(active_blocking_job_.get());

            changed_ = true;
            really_update_widget_ = !hasBlockingJobs();
        }
    }
}
void JobManager::handleNonBlockingJobs ()
{
    if (active_non_blocking_job_) // see if active one exists
    {
        if (active_non_blocking_job_->obsolete() || active_non_blocking_job_->done())
        {
            if(active_non_blocking_job_->obsolete())
            {
                logdbg << "JobManager: run: flushing obsolete non-blocking job";

                if (!stop_requested_)
                    active_non_blocking_job_->emitObsolete();
            }

            logdbg << "JobManager: run: flushing non-blocking done job";

            if (!stop_requested_)
            {
                active_non_blocking_job_->emitDone();
                logdbg << "JobManager: run: done non-blocking job emitted "+active_non_blocking_job_->name();
            }

            active_non_blocking_job_ = nullptr;
        }
    }

    if (!active_non_blocking_job_ && !non_blocking_jobs_.empty())
    {
        if (non_blocking_jobs_.try_pop(active_non_blocking_job_))
        {
            QThreadPool::globalInstance()->start(active_non_blocking_job_.get());

            changed_ = true;
            really_update_widget_ = !hasNonBlockingJobs();
        }
    }
}
void JobManager::handleDBJobs ()
{
    if (active_db_job_) // see if active one exists
    {
        if (active_db_job_->obsolete() || active_db_job_->done())
        {
            if(active_db_job_->obsolete())
            {
                logdbg << "JobManager: run: flushing obsolete non-blocking job";

                if (!stop_requested_)
                    active_db_job_->emitObsolete();
            }

            logdbg << "JobManager: run: flushing non-blocking done job";

            if (!stop_requested_)
            {
                active_db_job_->emitDone();
                logdbg << "JobManager: run: done non-blocking job emitted "+active_db_job_->name();
            }

            active_db_job_ = nullptr;
        }
    }

    if (!active_db_job_ && !queued_db_jobs_.empty())
    {
        if (queued_db_jobs_.try_pop(active_db_job_))
        {
            QThreadPool::globalInstance()->start(active_db_job_.get());

            changed_ = true;
            really_update_widget_ = !hasDBJobs();
        }
    }
}


void JobManager::shutdown ()
{
    loginf  << "JobManager: shutdown: setting jobs obsolete";

    stop_requested_ = true;

    if (active_db_job_)
        active_db_job_->setObsolete();

    for (auto job_it = queued_db_jobs_.unsafe_begin(); job_it != queued_db_jobs_.unsafe_end(); ++job_it)
        (*job_it)->setObsolete ();

    for (auto job_it = blocking_jobs_.unsafe_begin(); job_it != blocking_jobs_.unsafe_end(); ++job_it)
        (*job_it)->setObsolete ();

    for (auto job_it = non_blocking_jobs_.unsafe_begin(); job_it != non_blocking_jobs_.unsafe_end(); ++job_it)
        (*job_it)->setObsolete ();

    loginf  << "JobManager: shutdown: waiting on jobs to quit";

    while (hasAnyJobs())
    {
        loginf  << "JobManager: shutdown: waiting on jobs to finish: db " << hasDBJobs()
                << " blocking " << hasBlockingJobs() << " non-locking " << hasNonBlockingJobs();

        msleep(1000);
    }

    while (!stopped_)
    {
        loginf  << "JobManager: shutdown: waiting on run stop";
//        while (QCoreApplication::hasPendingEvents())
//            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        msleep(1000);
    }

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    loginf  << "JobManager: shutdown: done";
}

JobManagerWidget *JobManager::widget()
{
    if (!widget_)
    {
        widget_ = new JobManagerWidget (*this);
        last_update_time_ = boost::posix_time::microsec_clock::local_time();
    }

    assert (widget_);
    return widget_;
}

unsigned int JobManager::numBlockingJobs ()
{
    return active_blocking_job_ ? blocking_jobs_.unsafe_size()+1 : blocking_jobs_.unsafe_size();
}

unsigned int JobManager::numNonBlockingJobs ()
{
    return active_non_blocking_job_ ? non_blocking_jobs_.unsafe_size()+1 : non_blocking_jobs_.unsafe_size();
}

unsigned int JobManager::numDBJobs ()
{
    return active_db_job_ ? queued_db_jobs_.unsafe_size()+1 : queued_db_jobs_.unsafe_size();
}

unsigned int JobManager::numJobs ()
{
    return numBlockingJobs() + numNonBlockingJobs();
}

int JobManager::numThreads ()
{
    return QThreadPool::globalInstance()->activeThreadCount();
}

void JobManager::updateWidget (bool really)
{
    if (widget_)
    {
        boost::posix_time::ptime current_time = boost::posix_time::microsec_clock::local_time();
        boost::posix_time::time_duration diff = current_time - last_update_time_;

        if (diff.total_milliseconds() > 500 || really)
        {
            widget_->updateSlot();
            last_update_time_=current_time;
        }
    }
}
