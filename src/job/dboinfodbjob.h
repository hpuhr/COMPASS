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
 * DBOInfoDBJob.h
 *
 *  Created on: Feb 6, 2013
 *      Author: sk
 */

#ifndef DBOINFODBJOB_H_
#define DBOINFODBJOB_H_

#include "PropertyList.h"
#include "DBJob.h"
#include "Global.h"
#include "DBOVariableSet.h"


class Buffer;
class JobOrderer;

/**
 * @brief DBO read info job
 *
 * Reads a defined list of information from a DBO. Used to get detailed information about specific DBO records for
 * either labels or a listbox.
 */
class DBOInfoDBJob : public DBJob
{
public:
  DBOInfoDBJob(JobOrderer *orderer, boost::function<void (Job*)> done_function,
      boost::function<void (Job*)> obsolete_function, DBInterface *interface, DB_OBJECT_TYPE type,
      std::vector<unsigned int> ids, DBOVariableSet read_list, bool use_filters, std::string order_by_variable,
      bool ascending, unsigned int limit_min=0, unsigned int limit_max=0, bool finalize=0);
  virtual ~DBOInfoDBJob();

  virtual void execute ();

  /// @brief Returns DBO type
  DB_OBJECT_TYPE getType () { return type_; }
  /// @brief Returns result Buffer
  Buffer *getResultBuffer () { assert (result_buffer_); return result_buffer_; }
  /// @brief Returns keys for which data is queried
  std::vector<unsigned int> &getIds () { return ids_; }

protected:
  /// DBO type
  DB_OBJECT_TYPE type_;
  /// Keys
  std::vector<unsigned int> ids_;
  /// Data columns to be fetched
  DBOVariableSet read_list_;
  /// If filter configuration should be used
  bool use_filters_;
  /// If result should be ordered, and by which variable
  std::string order_by_variable_;
  /// Ordering type
  bool ascending_;
  /// First result to be returned
  unsigned int limit_min_;
  /// Number of results to be returned
  unsigned int limit_max_;

  /// Result Buffer
  Buffer *result_buffer_;
  /// Finalize result data flag
  bool finalize_;
};

#endif /* DBOINFODBJOB_H_ */
