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
 * DBOVariableMinMaxDBJob.cpp
 *
 *  Created on: Mar 5, 2013
 *      Author: sk
 */

#include "boost/date_time/posix_time/posix_time.hpp"
#include "DBResult.h"
#include "DBInterface.h"
#include "DBOVariableMinMaxDBJob.h"
#include "DBOVariable.h"
#include "DBSchemaManager.h"
#include "DBTableColumn.h"
#include "Buffer.h"
#include "DBSchema.h"
#include "DBObject.h"
#include "DBObjectManager.h"
#include "MetaDBTable.h"
#include "DBTable.h"
#include "DBCommand.h"
#include "String.h"

using namespace Utils::String;

DBOVariableMinMaxDBJob::DBOVariableMinMaxDBJob(JobOrderer *orderer, boost::function<void (Job*)> done_function,
        boost::function<void (Job*)> obsolete_function, DBInterface *db_interface,
        DBOVariable *variable)
: DBJob(orderer, done_function, obsolete_function, db_interface), variable_(variable)
{

}

DBOVariableMinMaxDBJob::~DBOVariableMinMaxDBJob()
{

}

void DBOVariableMinMaxDBJob::execute ()
{
    boost::posix_time::ptime loading_start_time_;
    boost::posix_time::ptime loading_stop_time_;

    loading_start_time_ = boost::posix_time::microsec_clock::local_time();

    if (!db_interface_->existsMinMaxTable())
        db_interface_->createMinMaxTable();

    loading_stop_time_ = boost::posix_time::microsec_clock::local_time();

    std::string meta_table_name = variable_->getCurrentMetaTable();
    loginf << "DBOVariableMinMaxDBJob: execute: meta table " << meta_table_name;
    std::string db_varname = variable_->getCurrentVariableName();
    loginf << "DBOVariableMinMaxDBJob: execute: db varname " << db_varname;
    std::string table_db_name = DBSchemaManager::getInstance().getCurrentSchema()->getMetaTable(meta_table_name)->getTableDBNameForVariable (db_varname);
    std::string table_name = DBSchemaManager::getInstance().getCurrentSchema()->getTableName(table_db_name);
    loginf << "DBOVariableMinMaxDBJob: execute: table name " << table_name;

    DBTableColumn *column =  DBSchemaManager::getInstance().getCurrentSchema()->getMetaTable(meta_table_name)->getTableColumn(db_varname);

    DBResult *result = db_interface_->queryMinMaxForColumn (column, table_name);//column->getDBTableName());
    assert (result->containsData());

    Buffer *buffer = result->getBuffer();


    assert (buffer->getSize() == 1);

    min_ = *((std::string*) buffer->get(0,0)); // has to be string in sqlgenerator
    max_ = *((std::string*) buffer->get(0,1));

    logdbg << "DBOVariableMinMaxDBJob: execute: inserting id " << column->getName() << " type " <<  variable_->getDBOType() <<
            " min " << min_ << " max " << max_;
    db_interface_->insertMinMax(column->getName(), variable_->getDBOType(), min_, max_);

    delete result;

    delete buffer;

    double load_time;
    boost::posix_time::time_duration diff = loading_stop_time_ - loading_start_time_;
    load_time= diff.total_milliseconds()/1000.0;

    loginf  << "DBOVariableMinMaxDBJob: execute: minmax of " << variable_->getName() <<" done (" << doubleToString (load_time) << " s).";
    done_=true;
}
