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

#include "boost/date_time/posix_time/posix_time.hpp"

#include "buffer.h"
#include "insertbufferdbjob.h"
#include "dbinterface.h"
#include "dbobject.h"
#include "dbovariable.h"
#include "metadbtable.h"
#include "dbtable.h"
#include "dbtablecolumn.h"

#include "stringconv.h"

using namespace Utils::String;

InsertBufferDBJob::InsertBufferDBJob(DBInterface &db_interface, DBObject &dbobject, std::shared_ptr<Buffer> buffer)
: Job("InsertBufferDBJob"), db_interface_(db_interface), dbobject_(dbobject), buffer_(buffer)
{
    assert (buffer_);
    //assert (dbobject_.existsInDB());
}

InsertBufferDBJob::~InsertBufferDBJob()
{

}

void InsertBufferDBJob::run ()
{
    logdbg  << "InsertBufferDBJob: run: start";

    boost::posix_time::ptime loading_start_time_;
    boost::posix_time::ptime loading_stop_time_;

    loading_start_time_ = boost::posix_time::microsec_clock::local_time();

    loginf  << "InsertBufferDBJob: run: writing object " << dbobject_.name() << " size " << buffer_->size();
    assert (buffer_->size());

    db_interface_.insertBuffer(dbobject_.currentMetaTable(), buffer_);
    loading_stop_time_ = boost::posix_time::microsec_clock::local_time();

    double load_time;
    boost::posix_time::time_duration diff = loading_stop_time_ - loading_start_time_;
    load_time= diff.total_milliseconds()/1000.0;

    loginf  << "InsertBufferDBJob: run: buffer write done (" << doubleToStringPrecision(load_time, 2) << " s).";
    done_=true;
}


