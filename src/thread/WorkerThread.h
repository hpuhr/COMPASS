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
 * WorkerThread.h
 *
 *  Created on: Feb 4, 2013
 *      Author: sk
 */

#ifndef WORKERTHREAD_H_
#define WORKERTHREAD_H_

#include "Thread.h"

class Job;

/**
 * @brief Thread specialization to execute Jobs
 */
class WorkerThread : public Thread
{
public:
  /// @brief Constructor
  WorkerThread(std::string id);
  /// @brief Destructor
  virtual ~WorkerThread();

  /// @brief Adds a TransformationJob
  void addJob (Job *job);

  /// @brief Returns working time, sleep time in percent
  std::pair <double, double> getUsage ();
  /// @brief Returns true if a job is executed
  bool getWorking ();
  /// @brief Returns number of queued jobs
  unsigned int getJobNumber ();

protected:
  /// Mutex protecting the to do list
  boost::mutex todos_mutex_;

  /// Container with all unfinished Jobs
  std::list <Job *> todos_;

  /// Sleep time for every do_work iteration
  const static unsigned int pause_time_=10;
  /// Sleep time when nothing is to do
  const static unsigned int idle_sleep_time_=100;
  /// Sleeping time in percent
  double sleep_time_;
  /// Working time in percent
  double work_time_;

  /// @brief Main thread working function
  virtual void do_work();
};

#endif /* WORKERTHREAD_H_ */
