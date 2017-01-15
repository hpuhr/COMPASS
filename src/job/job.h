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
 * Job.h
 *
 *  Created on: Feb 4, 2013
 *      Author: sk
 */

#ifndef JOB_H_
#define JOB_H_

#include <boost/signals2.hpp>
#include <boost/function.hpp>

class JobOrderer;

/**
 * @brief Encapsulates a work-package
 *
 * When created, is automatically added to the JobOrderer. Callbacks are performed either when the Job was
 * canceled (set as obsolete) or was completed (done). The work itself is defined in the execute function, which
 * must be overridden (and MUST set the done_ flag to true).
 *
 * Important: The Job and the contained data must be deleted in the callback functions.
 */
class Job
{
public:
  /// Emitted signal if Job was performed
  boost::signals2::signal<void (Job*)> done_signal_;
  /// Emitted signal if Job is obsolete
  boost::signals2::signal<void (Job*)> obsolete_signal_;

public:
  /// @brief Constructor
  Job(JobOrderer *orderer, boost::function<void (Job*)> done_function, boost::function<void (Job*)> obsolete_function);
  /// @brief Destructor
  virtual ~Job();

  // @brief Main operation function
  virtual void execute ()=0;

  // @brief Returns done flag
  bool isDone () { return done_; };
  // @brief Sets obsolete flag
  void setObsolete () { obsolete_=true; };
  // @brief Returns obsolete flag
  bool getObsolete () { return obsolete_; };

protected:
  /// Pointer to creator
  JobOrderer *orderer_;
  /// Done flag
  bool done_;
  /// Obsolete flag
  bool obsolete_;
};

#endif /* JOB_H_ */
