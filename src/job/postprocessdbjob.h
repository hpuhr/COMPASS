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
 * PostProcessDBJob.h
 *
 *  Created on: Feb 5, 2013
 *      Author: sk
 */

#ifndef POSTPROCESSDBJOB_H_
#define POSTPROCESSDBJOB_H_

#include "DBJob.h"

/**
 * @brief Post-processing Job
 *
 * Updates meta-information about database content: A list of active data sources and minimum/maximum values of all
 * variables.
 *
 * Note: Not in use, since information is now generated on demand.
 */
class PostProcessDBJob : public DBJob
{
public:
  PostProcessDBJob(JobOrderer *orderer, boost::function<void (Job*)> done_function,
      boost::function<void (Job*)> obsolete_function, DBInterface *db_interface);
  virtual ~PostProcessDBJob();

  virtual void execute ();

protected:
  /// @brief Creates properties table and values
  //void createProperties ();
  /// @brief Creates minimum/maximum table and values
  void createMinMaxValuesSpecial ();
  void createMinMaxValuesNormal ();
};

#endif /* POSTPROCESSDBJOB_H_ */
