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

#include <QThread>

#include "finalizedboreadjob.h"
#include "dbobject.h"
#include "dbovariableset.h"
#include "buffer.h"

FinalizeDBOReadJob::FinalizeDBOReadJob(DBObject &dbobject, DBOVariableSet &read_list, std::shared_ptr<Buffer> buffer)
    : Job("FinalizeDBOReadJob"), dbobject_(dbobject), read_list_(read_list), buffer_ (buffer)
{
    assert (buffer_);
}

FinalizeDBOReadJob::~FinalizeDBOReadJob()
{

}

void FinalizeDBOReadJob::run ()
{
    logdbg << "FinalizeDBOReadJob: run: read_list size " << read_list_.getSize();
    started_ = true;

    buffer_->transformVariables(read_list_, true);
    buffer_->addProperty("selected", PropertyDataType::BOOL); // add boolean to indicate selection

    logdbg << "FinalizeDBOReadJob: run: done";
    done_=true;
}

