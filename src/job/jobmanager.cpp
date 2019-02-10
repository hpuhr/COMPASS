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

void JobManager::addJob (std::shared_ptr<Job> job)
{
    logdbg << "JobManager: addJob: " << job->name() << " num " << jobs_.unsafe_size();
    jobs_.push(job);

    QThreadPool::globalInstance()->start(job.get());

    updateWidget();
}

void JobManager::addNonBlockingJob (std::shared_ptr<Job> job)
{
    loginf << "JobManager: addNonBlockingJob: " << job->name() << " num " << non_blocking_jobs_.unsafe_size();
    non_blocking_jobs_.push(job);

    QThreadPool::globalInstance()->start(job.get());

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

bool JobManager::noJobs ()
{
    return jobs_.empty() && non_blocking_jobs_.empty() && !active_db_job_ && queued_db_jobs_.empty();
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
        if (stop_requested_ && noJobs())
            break;

        bool changed=false;
        bool really_update_widget=false;

        while (!jobs_.empty())
        {
            // process normal jobs
            std::shared_ptr<Job> current = *jobs_.unsafe_begin();
            assert (current);

            if (!current->obsolete() && !current->done())
                break;

            jobs_.try_pop(current);

            changed = true;
            logdbg << "JobManager: run: flushed job "+current->name();

            if(current->obsolete())
            {
                logdbg << "JobManager: run: flushing obsolete job";

                if (!stop_requested_)
                    current->emitObsolete();
                continue;
            }

            logdbg << "JobManager: run: flushing done job";

            if (!stop_requested_)
                current->emitDone();

            logdbg << "JobManager: run: done job emitted "+current->name();

            really_update_widget = jobs_.empty();
        }

        while (!non_blocking_jobs_.empty())
        {
            // process normal jobs
            std::shared_ptr<Job> current = *non_blocking_jobs_.unsafe_begin();
            assert (current);

            if (!current->obsolete() && !current->done())
                break;

            non_blocking_jobs_.try_pop(current);

            changed = true;
            logdbg << "JobManager: run: flushed non-blocking job "+current->name();
            if(current->obsolete())
            {
                logdbg << "JobManager: run: flushing non-blocking obsolete job";

                if (!stop_requested_)
                    current->emitObsolete();

                continue;
            }

            logdbg << "JobManager: run: flushing non-blocking done job";
            current->emitDone();
            logdbg << "JobManager: run: non-blocking done job emitted "+current->name();

            really_update_widget = non_blocking_jobs_.empty();
        }

        //        for(auto it = non_blocking_jobs_.unsafe_begin(); it != non_blocking_jobs_.unsafe_end();)
        //        {
        //            assert (*it);

        //            if((*it)->obsolete() || (*it)->done())
        //            {
        //                std::shared_ptr<Job> job = *it;
        //                logdbg << "JobManager: run: processing non-blocking job "+job->name();

        //                it = non_blocking_jobs_.erase(it);

        //                changed = true;

        //                logdbg << "JobManager: run: flushed non-blocking job "+job->name();
        //                if(job->obsolete())
        //                {
        //                    logdbg << "JobManager: run: flushing obsolete non-blocking job";
        //                    job->emitObsolete();
        //                    continue;
        //                }

        //                logdbg << "JobManager: run: flushing done non-blocking job";
        //                job->emitDone();
        //                logdbg << "JobManager: run: done non-blocking job emitted "+job->name();

        //                really_update_widget = non_blocking_jobs_.size() == 0;
        //            }
        //            else
        //            {
        //                ++it;
        //            }
        //        }

        if (active_db_job_)
        {
            // see if active db job done or obsolete
            if(active_db_job_->obsolete())
            {
                logdbg << "JobManager: run: flushing db obsolete job";

                while (!active_db_job_->done()) // wait for finish
                {
                    logdbg << "JobManager: run: waiting to obsolete finish";
                    //QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                    msleep(10);
                }

                if (!stop_requested_)
                    active_db_job_->emitObsolete();

                active_db_job_ = nullptr;
                changed = true;
            }
            else if (active_db_job_->done())
            {
                logdbg << "JobManager: run: flushing db done job";

                if (!stop_requested_)
                    active_db_job_->emitDone();


                active_db_job_ = nullptr;
                changed = true;
                really_update_widget = true;
            }
        }

        while (!active_db_job_ && !queued_db_jobs_.empty())
        {
            // start a new one if needed
            std::shared_ptr<Job> current;
            queued_db_jobs_.try_pop(current);

            assert (current);
            assert (!current->done());

            if (current->obsolete())
            {
                if (!stop_requested_)
                    current->emitObsolete();

                continue;
            }

            logdbg << "JobManager: run: starting dbjob " << current->name();
            active_db_job_ = current;

            QThreadPool::globalInstance()->start(active_db_job_.get());
            changed = true;

            break;
        }

        if (!stop_requested_ && changed && !active_db_job_ && queued_db_jobs_.empty())
            emit databaseIdle();

        if (!stop_requested_ && changed)
            updateWidget(really_update_widget);

        //QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        msleep(1);
    }

    assert (jobs_.empty());
    assert (non_blocking_jobs_.empty());
    assert (!active_db_job_);
    assert (queued_db_jobs_.empty());

    stopped_=true;
    loginf  << "JobManager: run: stopped";
}

void JobManager::shutdown ()
{
    loginf  << "JobManager: shutdown: setting jobs obsolete";

    stop_requested_ = true;

    if (active_db_job_)
        active_db_job_->setObsolete();

    for (auto job_it = queued_db_jobs_.unsafe_begin(); job_it != queued_db_jobs_.unsafe_end(); ++job_it)
        (*job_it)->setObsolete ();

    for (auto job_it = jobs_.unsafe_begin(); job_it != jobs_.unsafe_end(); ++job_it)
        (*job_it)->setObsolete ();

    for (auto job_it = non_blocking_jobs_.unsafe_begin(); job_it != non_blocking_jobs_.unsafe_end(); ++job_it)
        (*job_it)->setObsolete ();

    loginf  << "JobManager: shutdown: waiting on jobs to quit";

    while (!queued_db_jobs_.empty() || active_db_job_ || !jobs_.empty() || !non_blocking_jobs_.empty())
    {
        loginf  << "JobManager: shutdown: waiting on jobs to finish: empty queued " << !queued_db_jobs_.empty()
                << " db " << (active_db_job_ != nullptr) << " jobs " << !jobs_.empty()
                                                       << " non-locking " << !non_blocking_jobs_.empty();

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

unsigned int JobManager::numJobs ()
{
    return jobs_.unsafe_size() + non_blocking_jobs_.unsafe_size();
}

unsigned int JobManager::numDBJobs ()
{
    return active_db_job_ ? queued_db_jobs_.unsafe_size()+1 : queued_db_jobs_.unsafe_size();
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
