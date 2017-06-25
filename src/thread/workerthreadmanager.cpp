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
 * WorkerThreadManager.cpp
 *
 *  Created on: Mar 26, 2012
 *      Author: sk
 */

#include <qtimer.h>
#include <QThreadPool>

#include "workerthreadmanager.h"
//#include "workerthread.h"
//#include "workerthreadWwdget.h"
//#include "dbjob.h"
#include "job.h"
#include "logger.h"
#include "stringconv.h"

using namespace Utils;

WorkerThreadManager::WorkerThreadManager()
    : Configurable ("WorkerThreadManager", "WorkerThreadManager0", 0, "conf/threads.xml")
{
    logdbg  << "WorkerThreadManager: constructor";
    boost::mutex::scoped_lock l(mutex_);

    registerParameter ("num_workers", &num_workers_, 3);
    registerParameter ("update_time", &update_time_, 10);

//    cnt_=0;

//    for (unsigned int cnt=0; cnt < num_workers_; cnt++)
//    {
//        std::string name = "Worker"+String::intToString(cnt);
//        WorkerThread *worker = new WorkerThread (name);
//        assert (worker);
//        workers_.push_back(worker);
//        worker->start();
//    }

    logdbg  << "WorkerThreadManager: constructor: end";
}

WorkerThreadManager::~WorkerThreadManager()
{
    logdbg  << "WorkerThreadManager: destructor";
//    boost::mutex::scoped_lock l(mutex_);
//    for (unsigned int cnt=0; cnt < num_workers_; cnt++)
//    {
//        while (workers_.at(cnt)->isRunning())
//        {
//            logdbg  << "WorkerThreadManager: destructor: waiting for thread to exist";
//            msleep(100);
//        }
//        delete workers_.at(cnt);
//    }
//    workers_.clear();
}

void WorkerThreadManager::shutdown ()
{
    logdbg  << "WorkerThreadManager: shutdown";
    boost::mutex::scoped_lock l(mutex_);

    stop_requested_ = true;

    //stop();

//    for (unsigned int cnt=0; cnt < num_workers_; cnt++)
//    {
//        WorkerThread *worker = workers_.at(cnt);
//        worker->shutdown();
//    }
    logdbg  << "WorkerThreadManager: shutdown: done";
}

void WorkerThreadManager::addJob (std::shared_ptr<Job> job)
{
    boost::mutex::scoped_lock l(mutex_);

    //unsigned int index = 1 + cnt_ % (num_workers_-1);

    QThreadPool::globalInstance()->start(job.get());

    //logdbg  << "WorkerThreadManager: addJob: to " << index;
    //workers_.at(index)->addJob(job);
    todos_signal_.push_back(job);

    //cnt_++;
}

//void WorkerThreadManager::addDBJob (std::shared_ptr<DBJob> job)
//{
//    boost::mutex::scoped_lock l(mutex_);

//    logdbg  << "WorkerThreadManager: addDBJob: to 0";
//    workers_.at(0)->addJob(job);
//    db_todos_signal_.push_back(job);
//}

void WorkerThreadManager::flushFinishedJobs ()
{
    boost::mutex::scoped_lock l(mutex_);
    while (todos_signal_.size() > 0)
    {
        std::shared_ptr<Job> current = todos_signal_.front();

        if( !current->obsolete() && !current->done() )
            break;

        todos_signal_.pop_front();
        logdbg << "WorkerThreadManager: flushFinishedJobs: flushed job";
        if(current->obsolete())
        {
            logdbg << "WorkerThreadManager: flushFinishedJobs: flushing obsolete job";
            current->emitObsolete();
            continue;
        }

        loginf << "WorkerThreadManager: flushFinishedJobs: flushing done job";
        current->emitDone();
    }

//    while (db_todos_signal_.size() > 0)
//    {
//        std::shared_ptr<DBJob> current = db_todos_signal_.front();

//        if( !current->obsolete() && !current->done() )
//            break;

//        db_todos_signal_.pop_front();
//        logdbg << "WorkerThreadManager: flushFinishedJobs: flushed db job";

//        if( current->obsolete() )
//        {
//            logdbg << "WorkerThreadManager: flushFinishedJobs: flushing obsolete db job";
//            current->emitObsolete();

//            continue;
//        }

//        logdbg << "WorkerThreadManager: flushFinishedJobs: flushing done db job";
//        current->emitDone();
//    }
}

bool WorkerThreadManager::noJobs ()
{
    boost::mutex::scoped_lock l(mutex_);
    //logdbg << "WorkerThreadManager: noJobs: todos " << todos_signal_.size() << " db tods " << db_todos_signal_.size();
    return todos_signal_.size() == 0; //&& db_todos_signal_.size() == 0
}

/**
 * Creates thread if possible.
 *
 * \exception std::runtime_error if thread already existed
 */
void WorkerThreadManager::run()
{
    logdbg  << "WorkerThreadManager: run: start";

    while (1)
    {
        if (todos_signal_.size() > 0) //|| db_todos_signal_.size() > 0
            flushFinishedJobs ();

        if (stop_requested_)
            break;

        msleep(update_time_);
    }
}

unsigned int WorkerThreadManager::numJobs ()
{
    boost::mutex::scoped_lock l(mutex_);
    return todos_signal_.size();
}

