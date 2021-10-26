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

#include "dboreaddbjob.h"

#include "buffer.h"
#include "dbinterface.h"
#include "dbobject.h"
#include "dbovariable.h"
#include "logger.h"
#include "propertylist.h"

DBOReadDBJob::DBOReadDBJob(DBInterface& db_interface, DBObject& dbobject, DBOVariableSet read_list,
                           std::string custom_filter_clause,
                           std::vector<DBOVariable*> filtered_variables, bool use_order,
                           DBOVariable* order_variable, bool use_order_ascending,
                           const std::string& limit_str)
    : Job("DBOReadDBJob"),
      db_interface_(db_interface),
      dbobject_(dbobject),
      read_list_(read_list),
      custom_filter_clause_(custom_filter_clause),
      filtered_variables_(filtered_variables),
      use_order_(use_order),
      order_variable_(order_variable),
      use_order_ascending_(use_order_ascending),
      limit_str_(limit_str)
{
    assert(dbobject_.existsInDB());
}

DBOReadDBJob::~DBOReadDBJob() {}

void DBOReadDBJob::run()
{
    loginf << "DBOReadDBJob: run: " << dbobject_.name() << ": start";
    started_ = true;

    if (obsolete_)
    {
        loginf << "DBOReadDBJob: run: " << dbobject_.name() << ": obsolete before prepared";
        done_ = true;
        return;
    }

    start_time_ = boost::posix_time::microsec_clock::local_time();

    db_interface_.prepareRead(dbobject_, read_list_, custom_filter_clause_, filtered_variables_,
                              use_order_, order_variable_, use_order_ascending_, limit_str_);

    unsigned int cnt = 0;

    while (!done_)
    {
        std::shared_ptr<Buffer> buffer = db_interface_.readDataChunk(dbobject_);
        assert(buffer);

        cnt++;

        if (obsolete_)
        {
            loginf << "DBOReadDBJob: run: " << dbobject_.name() << ": obsolete after prepared";
            break;
        }

        assert(buffer->dboName() == dbobject_.name());

        logdbg << "DBOReadDBJob: run: " << dbobject_.name() << ": intermediate signal, #buffers "
               << cnt << " last one " << buffer->lastOne();
        row_count_ += buffer->size();
        emit intermediateSignal(buffer);

        if (buffer->lastOne())
            break;
    }

    logdbg << "DBOReadDBJob: run: " << dbobject_.name() << ": finalizing statement";
    db_interface_.finalizeReadStatement(dbobject_);

    stop_time_ = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration diff = stop_time_ - start_time_;

    if (diff.total_seconds() > 0)
        loginf << "DBOReadDBJob: run: " << dbobject_.name() << ": done after " << diff << ", "
               << 1000.0 * row_count_ / diff.total_milliseconds() << " el/s";
    else
        loginf << "DBOReadDBJob: run: " << dbobject_.name() << ": done";

    done_ = true;

    return;
}

unsigned int DBOReadDBJob::rowCount() const { return row_count_; }
