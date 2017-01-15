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
 * WorkerThread.cpp
 *
 *  Created on: Feb 4, 2013
 *      Author: sk
 */

#include "Job.h"
#include "WorkerThread.h"
#include "WorkerThreadManager.h"
#include "Logger.h"

WorkerThread::WorkerThread(std::string id)
: Thread (id), sleep_time_ (0), work_time_ (0)
{
}

WorkerThread::~WorkerThread()
{
}

void WorkerThread::do_work()
{
  logdbg  << "WorkerThread " << id_ << ": do_work: start";

  while (!stop_requested_)
  {
    //logdbg  << "WorkerThread " << id_ << ": do_work: iterating";
    boost::this_thread::sleep(boost::posix_time::milliseconds(pause_time_));
    sleep_time_ += pause_time_;

    boost::mutex::scoped_lock statelock(state_mutex_);
    if (state_== THREAD_STATE_IDLE && todos_.size() > 0)
    {
      //statelock.unlock();
      //logdbg  << "WorkerThread " << id_ << ": do_work: got job";

      state_= THREAD_STATE_WORKING;
      boost::mutex::scoped_lock todoslock(todos_mutex_);
      if (todos_.size() > 0)
      {
        logdbg  << "WorkerThread " << id_ << ": do_work: getting job";
        boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

        Job *job = todos_.back();
        todos_.pop_back();
        todoslock.unlock();

        if (!job->getObsolete())
        {
          logdbg  << "WorkerThread " << id_ << ": do_work: executing job";
          job->execute();
          //WorkerThreadManager::getInstance().flushFinishedJobs();
        }
        boost::posix_time::ptime stop_time = boost::posix_time::microsec_clock::local_time();
        boost::posix_time::time_duration diff = stop_time - start_time;
        work_time_ += diff.total_milliseconds();
        logdbg  << "WorkerThread " << id_ << ": do_work: executing job done";
      }
      else
        todoslock.unlock();

      //statelock.lock();
      state_= THREAD_STATE_IDLE;
    }
    else
    {
      statelock.unlock();
      //logdbg  << "WorkerThread: do_work: state  " << state_ << " idle waiting";
      boost::this_thread::sleep(boost::posix_time::milliseconds(idle_sleep_time_));
      sleep_time_ += idle_sleep_time_;
    }

    sleep_time_ *= 0.9;
    work_time_ *= 0.9;
  }
  //logdbg  << "WorkerThread: do_work: end";
}

void WorkerThread::addJob (Job *job)
{
  boost::mutex::scoped_lock l(todos_mutex_);

  assert (job);
  todos_.push_front(job);
}

std::pair <double, double> WorkerThread::getUsage ()
{
  return std::pair<double, double> (work_time_, sleep_time_);
}

unsigned int WorkerThread::getJobNumber ()
{
  return todos_.size();
}



