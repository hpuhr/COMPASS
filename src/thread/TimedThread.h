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


#ifndef TIMEDTHREAD_H
#define TIMEDTHREAD_H

#include "Thread.h"


/**
@brief A thread that is timed to trigger at a defined interval.

Derive and reimplement workFun().
 */
class TimedThread : public Thread
{
public:
  /// @brief Constructor
  TimedThread( unsigned int interval_ms, const std::string& id );
  /// @brief Destructor
  virtual ~TimedThread();

protected:
  /// @brief Reimplemented main thread working function
  void do_work();
  /// @brief Function to be called at each interval
  virtual void workFun() = 0;

  /// Interval time in milliseconds
  unsigned int interval_;
};

#endif /* WORKERTHREAD_H_ */
