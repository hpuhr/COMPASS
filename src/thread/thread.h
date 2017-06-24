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
 * Thread.h
 *
 *  Created on: Feb 4, 2013
 *      Author: sk
 */

#ifndef THREAD_H_
#define THREAD_H_

#include <boost/thread.hpp>

/// Thread state variable
enum THREAD_STATE {THREAD_STATE_INIT, THREAD_STATE_IDLE, THREAD_STATE_WORKING, THREAD_STATE_SHUTDOWN, THREAD_STATE_ERROR};

/**
 * @brief Encapsulates a boost thread
 *
 * Created with a string id, can be started (go) and stopped (shutdown). Has state and access functions.
 */
class Thread
{
public:
  /// @brief Constructor
  Thread(std::string id, bool start);
  /// @brief Destructor
  virtual ~Thread();

  /// @brief Starts the thread
  void go();
  /// @brief Stops the thread
  void shutdown ();

  /// @brief Returns true if initializing
  bool isInitializing ();
  /// @brief Returns true if idle
  bool isIdle ();
  /// @brief Returns true if working
  bool isWorking ();
  /// @brief Returns true if shut down
  bool isShutdown ();
  /// @brief Returns true if error
  bool hasError ();


protected:
  /// Flag indicating if thread should be stopped
  volatile bool stop_requested_;
  /// Boost thread
  boost::shared_ptr<boost::thread> thread_;
  /// Mutex protecting the state variable
  boost::mutex state_mutex_;
  /// State variable
  THREAD_STATE state_;
  /// Identifier
  std::string id_;

  /// @brief Stops the thread
  void stop();
  /// @brief Main thread working function
  virtual void do_work()=0;
};

#endif /* THREAD_H_ */
