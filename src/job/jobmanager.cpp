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

/*
 * JobManager.cpp
 *
 *  Created on: Mar 26, 2012
 *      Author: sk
 */

#include <qtimer.h>
#include <QThreadPool>

#include "jobmanager.h"
#include "jobmanagerwidget.h"
#include "job.h"
#include "logger.h"
#include "stringconv.h"

using namespace Utils;

JobManager::JobManager()
    : Configurable ("JobManager", "JobManager0", 0, "conf/threads.xml"), widget_(nullptr)
{
    logdbg  << "JobManager: constructor";
    boost::mutex::scoped_lock l(mutex_);

    registerParameter ("num_workers", &num_workers_, 3);
    registerParameter ("update_time", &update_time_, 10);

    logdbg  << "JobManager: constructor: end";
}

JobManager::~JobManager()
{
    logdbg  << "JobManager: destructor";
}

void JobManager::shutdown ()
{
    logdbg  << "JobManager: shutdown";
    boost::mutex::scoped_lock l(mutex_);

    for (auto job : db_jobs_)
        job->setObsolete ();

    for (auto job : jobs_)
        job->setObsolete ();

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    while (db_jobs_.size() > 0) // wait for finish
    {
        auto job = std::begin(db_jobs_);

        while (job != std::end(db_jobs_))
        {
            if ((*job)->done ())
                job = db_jobs_.erase(job);
            else
                ++job;
        }
        msleep(update_time_);
    }

    while (jobs_.size() > 0) // wait for finish
    {
        auto job = std::begin(jobs_);

        while (job != std::end(jobs_))
        {
            if ((*job)->done ())
                job = jobs_.erase(job);
            else
                ++job;
        }
        msleep(update_time_);
    }

    stop_requested_ = true;

    logdbg  << "JobManager: shutdown: done";
}

void JobManager::addJob (std::shared_ptr<Job> job)
{
    mutex_.lock();

    QThreadPool::globalInstance()->start(job.get());

    jobs_.push_back(job);

    mutex_.unlock();

    if (widget_)
        widget_->updateSlot();
}

void JobManager::addDBJob (std::shared_ptr<Job> job)
{
    mutex_.lock();

    QThreadPool::globalInstance()->start(job.get());

    db_jobs_.push_back(job);

    mutex_.unlock();

    if (widget_)
        widget_->updateSlot();
}


void JobManager::cancelJob (std::shared_ptr<Job> job)
{
    mutex_.lock();

    job->setObsolete();

    while (!job->done()) // wait for finish
        msleep(1);

    if (find(jobs_.begin(), jobs_.end(), job) != jobs_.end())
        jobs_.erase(find(jobs_.begin(), jobs_.end(), job));
    else
        db_jobs_.erase(find(jobs_.begin(), jobs_.end(), job));

    mutex_.unlock();

    if (widget_)
        widget_->updateSlot();
}

void JobManager::flushFinishedJobs ()
{
    mutex_.lock();

    bool changed=false;

    while (jobs_.size() > 0)
    {
        std::shared_ptr<Job> current = jobs_.front();
        assert (current);

        if( !current->obsolete() && !current->done() )
            break;

        jobs_.pop_front();
        changed = true;
        logdbg << "JobManager: flushFinishedJobs: flushed job";
        if(current->obsolete())
        {
            logdbg << "JobManager: flushFinishedJobs: flushing obsolete job";
            current->emitObsolete();
            continue;
        }

        loginf << "JobManager: flushFinishedJobs: flushing done job";
        current->emitDone();

    }

    while (db_jobs_.size() > 0)
    {
        std::shared_ptr<Job> current = db_jobs_.front();
        assert (current);

        if( !current->obsolete() && !current->done() )
            break;

        db_jobs_.pop_front();
        changed = true;
        logdbg << "JobManager: flushFinishedJobs: flushed job";
        if(current->obsolete())
        {
            logdbg << "JobManager: flushFinishedJobs: flushing obsolete job";
            current->emitObsolete();
            continue;
        }

        loginf << "JobManager: flushFinishedJobs: flushing done job";
        current->emitDone();

    }

    mutex_.unlock();

    if (changed && widget_)
        widget_->updateSlot();

}

bool JobManager::noJobs ()
{
    boost::mutex::scoped_lock l(mutex_);
    //logdbg << "JobManager: noJobs: todos " << todos_signal_.size();
    return jobs_.size() == 0 && db_jobs_.size() == 0;
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
        if (jobs_.size() > 0 || db_jobs_.size() > 0)
            flushFinishedJobs ();

        if (stop_requested_)
            break;

        msleep(update_time_);
    }
}

JobManagerWidget *JobManager::widget()
{
    if (!widget_)
    {
        widget_ = new JobManagerWidget (*this);
    }

    assert (widget_);
    return widget_;
}

unsigned int JobManager::numJobs ()
{
    boost::mutex::scoped_lock l(mutex_);
    return jobs_.size();
}

unsigned int JobManager::numDBJobs ()
{
    boost::mutex::scoped_lock l(mutex_);
    return db_jobs_.size();
}

int JobManager::numThreads ()
{
    return QThreadPool::globalInstance()->activeThreadCount();
}
