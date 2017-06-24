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

#include "workerthreadmanager.h"
#include "workerthread.h"
//#include "workerthreadWwdget.h"
//#include "dbjob.h"
#include "job.h"
#include "logger.h"
//#include <boost/asio.hpp>
//#include <boost/bind.hpp>
//#include <boost/date_time/posix_time/posix_time.hpp>
#include "stringconv.h"

using namespace Utils;

WorkerThreadManager::WorkerThreadManager()
    : Configurable ("WorkerThreadManager", "WorkerThreadManager0", 0, "conf/threads.xml")//, timer_ (nullptr)
      //timer_ (io_, boost::posix_time::milliseconds(100))
{
    logdbg  << "WorkerThreadManager: constructor";
    boost::mutex::scoped_lock l(mutex_);

    registerParameter ("num_workers", &num_workers_, 3);
    registerParameter ("update_time", &update_time_, 10);

    cnt_=0;

    for (unsigned int cnt=0; cnt < num_workers_; cnt++)
    {
        std::string name = "Worker"+String::intToString(cnt);
        WorkerThread *worker = new WorkerThread (name);
        assert (worker);
        workers_.push_back(worker);
        worker->go();
    }

//    timer_ = new QTimer(this);
//    connect(timer_, SIGNAL(timeout()), this, SLOT(timerEventSlot()));
//    timer_->start(update_time_);

//    timer_.async_wait(boost::bind(&WorkerThreadManager::timerEvent, this));

    //go();

    logdbg  << "WorkerThreadManager: constructor: end";
}

WorkerThreadManager::~WorkerThreadManager()
{
    logdbg  << "WorkerThreadManager: destructor";
    boost::mutex::scoped_lock l(mutex_);
    for (unsigned int cnt=0; cnt < num_workers_; cnt++)
    {
        delete workers_.at(cnt);
    }
    workers_.clear();

//    timer_->stop();
//    delete timer_;
//    timer_=nullptr;
}

void WorkerThreadManager::shutdown ()
{
    logdbg  << "WorkerThreadManager: shutdown";
    boost::mutex::scoped_lock l(mutex_);

    stop_requested_ = true;

    //stop();

    for (unsigned int cnt=0; cnt < num_workers_; cnt++)
    {
        WorkerThread *worker = workers_.at(cnt);
        worker->shutdown();
    }
    logdbg  << "WorkerThreadManager: shutdown: done";
}

void WorkerThreadManager::addJob (std::shared_ptr<Job> job)
{
    boost::mutex::scoped_lock l(mutex_);

    unsigned int index = 1 + cnt_ % (num_workers_-1);

    logdbg  << "WorkerThreadManager: addJob: to " << index;
    workers_.at(index)->addJob(job);
    todos_signal_.push_back(job);

    cnt_++;
}

void WorkerThreadManager::addDBJob (std::shared_ptr<DBJob> job)
{
    boost::mutex::scoped_lock l(mutex_);

    logdbg  << "WorkerThreadManager: addDBJob: to 0";
    workers_.at(0)->addJob(job);
    db_todos_signal_.push_back(job);
}

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
        if( current->obsolete() )
        {
            current->emitObsolete();
                    //current->obsolete_signal_(current);
            //delete current;
            continue;
        }

        current->emitDone();
    }

    while (db_todos_signal_.size() > 0)
    {
        std::shared_ptr<DBJob> current = db_todos_signal_.front();
        if( !current->obsolete() && !current->done() )
            break;

        db_todos_signal_.pop_front();
        logdbg << "WorkerThreadManager: flushFinishedJobs: flushed db job";
        if( current->obsolete() )
        {
            current->emitObsolete();
            //delete current;
            continue;
        }

        current->emitDone();
    }
}

bool WorkerThreadManager::noJobs ()
{
    boost::mutex::scoped_lock l(mutex_);
    //logdbg << "WorkerThreadManager: noJobs: todos " << todos_signal_.size() << " db tods " << db_todos_signal_.size();
    return todos_signal_.size() == 0 && db_todos_signal_.size() == 0;
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
        if (todos_signal_.size() > 0 || db_todos_signal_.size() > 0)
            flushFinishedJobs ();

        if (stop_requested_)
            break;

        msleep(update_time_);
    }
//    if (thread_)
//        throw std::runtime_error("WorkerThreadManager: go: thread already exists");
//    thread_ = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&WorkerThreadManager::do_work, this)));
//    logdbg  << "WorkerThreadManager: go: end";
}

//void WorkerThreadManager::stop()
//{
//    logdbg  << "WorkerThreadManager: stop: start";
//    assert(thread_);
//    stop_requested_ = true;
//    logdbg  << "WorkerThreadManager: stop: stop";
//    timer_->stop();

//    logdbg  << "WorkerThreadManager: stop: joining";
//    thread_->join();

//    logdbg  << "WorkerThreadManager: stop: end";
//}

///**
// * Main function executed in thread. Uses while loop (terminating if stop_requested_ is set),
// * checking the state_ and performing tasks according to a state machine.
// */
//void WorkerThreadManager::do_work()
//{
//    logdbg  << "WorkerThreadManager: do_work: start";
//    io_.run();
//    logdbg  << "WorkerThreadManager: do_work: end";
//}

//void WorkerThreadManager::timerEvent (QTimerEvent *e)
//void WorkerThreadManager::timerEventSlot()
//{
//    logdbg << "WorkerThreadManager: timerEvent: got " << todos_signal_.size()+ db_todos_signal_.size() << " jobs";
//    if (todos_signal_.size() > 0 || db_todos_signal_.size() > 0)
//        flushFinishedJobs();

////    timer_.expires_at(timer_.expires_at() + boost::posix_time::milliseconds(500));
////    timer_.async_wait(boost::bind(&WorkerThreadManager::timerEvent, this));
//}

WorkerThread *WorkerThreadManager::getWorker (unsigned int index)
{
    assert (index < num_workers_);
    return workers_.at(index);
}
unsigned int WorkerThreadManager::getJobNumber ()
{
    return todos_signal_.size();
}

unsigned int WorkerThreadManager::getNumWorkers ()
{
    return num_workers_;
}

