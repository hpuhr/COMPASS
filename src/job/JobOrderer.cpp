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
 * JobOrderer.cpp
 *
 *  Created on: Feb 8, 2013
 *      Author: sk
 */

#include <algorithm>
#include "Buffer.h"
#include "JobOrderer.h"
#include "Logger.h"
#include "Job.h"

JobOrderer::JobOrderer()
{
}

JobOrderer::~JobOrderer()
{
  if (active_jobs_.size() != 0)
  {
    logerr << "JobOrderer: destructor: " << active_jobs_.size() << " active jobs not removed";
  }
}

void JobOrderer::addJob (Job *job)
{
  boost::mutex::scoped_lock l(mutex_);
  assert (std::find (active_jobs_.begin(), active_jobs_.end(), job) == active_jobs_.end());
  active_jobs_.push_back(job);
}
void JobOrderer::removeJob (Job *job)
{
  boost::mutex::scoped_lock l(mutex_);
  assert (std::find (active_jobs_.begin(), active_jobs_.end(), job) != active_jobs_.end());
  active_jobs_.erase(std::find (active_jobs_.begin(), active_jobs_.end(), job));
}


void JobOrderer::setJobsObsolete ()
{
  boost::mutex::scoped_lock l(mutex_);
  std::vector <Job *>::iterator it;

  for (it = active_jobs_.begin(); it != active_jobs_.end(); it++)
  {
    (*it)->setObsolete();
  }
}
