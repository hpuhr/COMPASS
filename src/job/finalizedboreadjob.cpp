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

#include "finalizedboreadjob.h"

#include <QThread>

#include "buffer.h"
#include "dbobject.h"
#include "dbovariableset.h"

FinalizeDBOReadJob::FinalizeDBOReadJob(DBObject& dbobject, DBOVariableSet& read_list,
                                       std::shared_ptr<Buffer> buffer)
    : Job("FinalizeDBOReadJob"), dbobject_(dbobject), read_list_(read_list), buffer_(buffer)
{
    assert(buffer_);
}

FinalizeDBOReadJob::~FinalizeDBOReadJob() {}

void FinalizeDBOReadJob::run()
{
    logdbg << "FinalizeDBOReadJob: run: read_list size " << read_list_.getSize();
    started_ = true;

    // rename db column names into DBOVariable names
    buffer_->transformVariables(read_list_, true);

    // add boolean to indicate selection
    buffer_->addProperty(DBObject::selected_var);

    logdbg << "FinalizeDBOReadJob: run: done";
    done_ = true;
}
