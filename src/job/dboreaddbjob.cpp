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
 * DBOReadDBJob.cpp
 *
 *  Created on: Feb 6, 2013
 *      Author: sk
 */

#include <boost/thread/thread.hpp>

#include <QThread>

#include "dboreaddbjob.h"
#include "dbobject.h"
#include "dbovariable.h"
#include "propertylist.h"
//#include "DBTableColumn.h"
//#include "DBSchemaManager.h"
//#include "DBOVariableSet.h"
#include "dbinterface.h"
//#include "UnitManager.h"
//#include "Unit.h"
#include "buffer.h"
//#include "DBSchema.h"
//#include "MetaDBTable.h"
#include "logger.h"

DBOReadDBJob::DBOReadDBJob(DBInterface &db_interface, DBObject &dbobject, DBOVariableSet read_list, std::string custom_filter_clause,
                           DBOVariable *order, bool activate_key_search)
: Job (db_interface), dbobject_(dbobject), read_list_(read_list), custom_filter_clause_ (custom_filter_clause), order_(order), activate_key_search_(activate_key_search)
{
    // AVIBIT HACK
//    if (order == 0)
//    {
//        order_ = DBObjectManager::getInstance().getDBOVariable (DBO_UNDEFINED, "id");
//        assert (order_->existsIn (type));
//    }
}

DBOReadDBJob::~DBOReadDBJob()
{

}

void DBOReadDBJob::run ()
{
    logdbg << "DBOReadDBJob: execute: start";

    if (custom_filter_clause_.size() == 0 && order_ )
        db_interface_.prepareRead (dbobject_, read_list_);
    else
        db_interface_.prepareRead (dbobject_, read_list_, custom_filter_clause_, order_);

    while (!done_ && !obsolete_)
    {
        // AVIBIT HACK
        //Buffer *buffer = db_interface_->readDataChunk(type_, order_->getName() == "id");
        std::shared_ptr<Buffer> buffer = db_interface_.readDataChunk(dbobject_, activate_key_search_);

        assert (buffer->dboName() == dbobject_.name());

        if (!buffer)
        {
            logwrn << "DBOReadDBJob: execute: got null buffer";
            db_interface_.finalizeReadStatement(dbobject_);
            done_=true;
            return;
        }
        else
        {
            buffers_.push_back(buffer);
            loginf << "DBOReadDBJob: execute: intermediate signal, #buffers " << buffers_.size() << " last one " << buffer->lastOne();
            emit intermediateSignal(buffer);
        }

        if (buffer->lastOne())
            break;
        //QThread::currentThread()->msleep(50);
        //msleep (10);
    }

    db_interface_.finalizeReadStatement(dbobject_);
    done_=true;

    logdbg << "DBOReadDBJob: execute: done";
    return;
}
