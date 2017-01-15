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
 * Job.cpp
 *
 *  Created on: Feb 4, 2013
 *      Author: sk
 */

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "job.h"
#include "joborderer.h"

/**
 * \param orderer Job creator
 * \param done_function will be connected to the done signal
 * \param obsolete_function will be connected to the obsolete signal
 */
Job::Job(JobOrderer *orderer, boost::function<void (Job*)> done_function, boost::function<void (Job*)> obsolete_function)
: orderer_(orderer), done_ (false), obsolete_(false)
{
  assert (orderer_);
  done_signal_.connect( done_function );
  obsolete_signal_.connect( obsolete_function );
  orderer_->addJob(this);
}

Job::~Job()
{
  orderer_->removeJob(this);
}
