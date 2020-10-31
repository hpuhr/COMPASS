/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DBOMINMAXDBJOB_H_
#define DBOMINMAXDBJOB_H_

#include "job.h"

class DBObject;
class DBInterface;
class DBTable;

/**
 * @brief Post-processing Job
 *
 * Updates meta-information about database content: A list of active data sources and
 * minimum/maximum values of all variables.
 *
 */
class DBOMinMaxDBJob : public Job
{
  public:
    DBOMinMaxDBJob(DBInterface& db_interface, const DBObject& object);
    virtual ~DBOMinMaxDBJob();

    virtual void run();

  protected:
    DBInterface& db_interface_;
    const DBObject& object_;
    /// @brief Creates minimum/maximum table and values
    // void createMinMaxValuesSpecial ();
    void createMinMaxValuesNormal();
    void processTable(const DBTable& table);
};

#endif /* DBOMINMAXDBJOB_H_ */
