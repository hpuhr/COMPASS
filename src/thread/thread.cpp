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
 * Thread.cpp
 *
 *  Created on: Feb 4, 2013
 *      Author: sk
 */

#include "thread.h"
#include "logger.h"

Thread::Thread(std::string id, bool start)
: stop_requested_(false), state_ (THREAD_STATE_INIT), id_(id)
{
  assert (id_.size() > 0);

  if( start )
    go();

  boost::mutex::scoped_lock l(state_mutex_);
  state_ = THREAD_STATE_IDLE;
}

Thread::~Thread()
{
}

void Thread::go()
{
  //loginf << "Thread: go"; // breaks for some reason

  if (thread_)
    throw std::runtime_error("Thread: go: thread already exists");

  thread_ = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&Thread::do_work, this)));

  //logdbg << "Thread: go: end";
}

void Thread::stop()
{
  logdbg << "Thread: stop: " << id_;
  assert(thread_);
  stop_requested_ = true;
  logdbg << "Thread: stop: joining";
  thread_->join();
  logdbg << "Thread: stop: end";
}

void Thread::shutdown ()
{
  loginf  << "Thread: shutdown";

  stop();

  boost::mutex::scoped_lock l(state_mutex_);
  state_ = THREAD_STATE_SHUTDOWN;

  logdbg  << "Thread: shutdown: end";
}

bool Thread::isInitializing ()
{
  return state_ == THREAD_STATE_INIT;
}

bool Thread::isIdle ()
{
  return state_ == THREAD_STATE_IDLE;
}

bool Thread::isWorking ()
{
  return state_ == THREAD_STATE_WORKING;
}

bool Thread::isShutdown ()
{
  return state_ == THREAD_STATE_SHUTDOWN;
}

bool Thread::hasError ()
{
  return state_ == THREAD_STATE_ERROR;
}
