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

#include "dbcontentreaddbjob.h"
#include "buffer.h"
#include "dbinterface.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/variable/variable.h"
#include "logger.h"
#include "propertylist.h"
#include "compass.h"
#include "viewmanager.h"

using namespace dbContent;

DBContentReadDBJob::DBContentReadDBJob(DBInterface& db_interface, DBContent& dbobject, VariableSet read_list,
                           const std::vector<std::string>& extra_from_parts,
                           std::string custom_filter_clause,
                           std::vector<Variable*> filtered_variables, bool use_order,
                           Variable* order_variable, bool use_order_ascending,
                           const std::string& limit_str)
    : Job("DBContentReadDBJob"),
      db_interface_(db_interface),
      dbobject_(dbobject),
      read_list_(read_list),
      extra_from_parts_(extra_from_parts),
      custom_filter_clause_(custom_filter_clause),
      filtered_variables_(filtered_variables),
      use_order_(use_order),
      order_variable_(order_variable),
      use_order_ascending_(use_order_ascending),
      limit_str_(limit_str)
{
    assert(dbobject_.existsInDB());
}

DBContentReadDBJob::~DBContentReadDBJob() {}

void DBContentReadDBJob::run()
{
    loginf << "DBContentReadDBJob: run: " << dbobject_.name() << ": start";
    started_ = true;

    if (obsolete_)
    {
        loginf << "DBContentReadDBJob: run: " << dbobject_.name() << ": obsolete before prepared";
        done_ = true;
        return;
    }

    start_time_ = boost::posix_time::microsec_clock::local_time();

    db_interface_.prepareRead(dbobject_, read_list_, extra_from_parts_, custom_filter_clause_, filtered_variables_,
                              use_order_, order_variable_, use_order_ascending_, limit_str_);

    unsigned int cnt = 0;

    ViewManager &view_manager = COMPASS::instance().viewManager();

    bool last_buffer;

    while (!obsolete_)
    {
        std::shared_ptr<Buffer> buffer = db_interface_.readDataChunk(dbobject_);
        assert(buffer);
        last_buffer = buffer->lastOne();

        cnt++;

        if (obsolete_)
            break;

        assert(buffer->dbContentName() == dbobject_.name());

        logdbg << "DBContentReadDBJob: run: " << dbobject_.name() << ": intermediate signal, #buffers "
               << cnt << " last one " << buffer->lastOne();
        row_count_ += buffer->size();

        // add data to cache
        if (!cached_buffer_) // no cache
            cached_buffer_ = buffer;
        else
        {
            cached_buffer_->seizeBuffer(*buffer);
            buffer = nullptr;
        }

        if (obsolete_)
            break;

        if (!view_manager.isProcessingData() || last_buffer) // distribute data
        {
            loginf << "DBContentReadDBJob: run: " << dbobject_.name() << ": emitting intermediate read, size " << row_count_;
            emit intermediateSignal(cached_buffer_);

            cached_buffer_ = nullptr;
        }

        if (last_buffer)
        {
            loginf << "DBContentReadDBJob: run: " << dbobject_.name() << ": last buffer";
            break;
        }
    }

    assert (!cached_buffer_);

    logdbg << "DBContentReadDBJob: run: " << dbobject_.name() << ": finalizing statement";
    db_interface_.finalizeReadStatement(dbobject_);

    stop_time_ = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration diff = stop_time_ - start_time_;

    if (diff.total_seconds() > 0)
        loginf << "DBContentReadDBJob: run: " << dbobject_.name() << ": done after " << diff << ", "
               << 1000.0 * row_count_ / diff.total_milliseconds() << " el/s";
    else
        loginf << "DBContentReadDBJob: run: " << dbobject_.name() << ": done";

    done_ = true;

    return;
}

unsigned int DBContentReadDBJob::rowCount() const { return row_count_; }