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

#include "dbominmaxdbjob.h"

#include "boost/date_time/posix_time/posix_time.hpp"
#include "buffer.h"
#include "dbcommand.h"
#include "dbinterface.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbresult.h"
#include "global.h"
#include "sqlgenerator.h"
#include "string.h"
#include "stringconv.h"

using namespace Utils;

DBOMinMaxDBJob::DBOMinMaxDBJob(DBInterface& db_interface, const DBObject& object)
    : Job("DBOMinMaxDBJob"), db_interface_(db_interface), object_(object)
{
    assert(db_interface_.existsMinMaxTable());
    assert(object_.existsInDB());
}

DBOMinMaxDBJob::~DBOMinMaxDBJob() {}

void DBOMinMaxDBJob::run()
{
    loginf << "DBOMinMaxDBJob: run: start";

    started_ = true;

    boost::posix_time::ptime loading_start_time_;
    boost::posix_time::ptime loading_stop_time_;

    loading_start_time_ = boost::posix_time::microsec_clock::local_time();

    assert(db_interface_.existsMinMaxTable());

    // createMinMaxValuesSpecial();
    createMinMaxValuesNormal();

    loading_stop_time_ = boost::posix_time::microsec_clock::local_time();

    double load_time;
    boost::posix_time::time_duration diff = loading_stop_time_ - loading_start_time_;
    load_time = diff.total_milliseconds() / 1000.0;

    loginf << "DBOMinMaxDBJob: run: db postprocessing done (" << load_time << " s).";
    done_ = true;
}

void DBOMinMaxDBJob::createMinMaxValuesNormal()
{
    assert(db_interface_.existsMinMaxTable());

    assert (false); // TODO
//    processTable(object_.currentMetaTable().mainTable());

//    for (auto table_it : object_.currentMetaTable().subTables())
//    {
//        if (table_it.second.existsInDB())
//            processTable(table_it.second);
//        else
//            loginf << "DBOMinMaxDBJob: createMinMaxValuesNormal: table '" << table_it.first
//                   << " does not exist in db";
//    }
}

//void DBOMinMaxDBJob::processTable(const DBTable& table)
//{
//    loginf << "DBOMinMaxDBJob: processTable: getting minimum and maximum of all variables of table "
//           << table.name();

//    assert(table.existsInDB());
//    std::vector<std::string> minmax;

//    std::vector<std::pair<std::string, std::string> >
//        inserted_variable_names;  // object name, variable

//    logdbg << "DBOMinMaxDBJob: processTable: executing command";
//    std::shared_ptr<DBResult> result = db_interface_.queryMinMaxNormalForTable(table);

//    assert(result);
//    assert(result->containsData());

//    // std::string text;
//    std::shared_ptr<Buffer> buffer = result->buffer();

//    logdbg << "DBOMinMaxDBJob: processTable: setting variables";

//    assert(buffer);

//    if (buffer->size() == 0)
//    {
//        logwrn << "DBOMinMaxDBJob: processTable: table '" << table.name() << "' has no data";
//        return;
//    }

//    assert(buffer->size() == 1);

//    std::string min;
//    std::string max;

//    for (auto& col_it : table.columns())  // over variables/properties
//    {
//        minmax.clear();

//        assert(buffer->properties().hasProperty(col_it.first + "MIN"));
//        assert(buffer->properties().hasProperty(col_it.first + "MAX"));

//        if (buffer->get<std::string>(col_it.first + "MIN").isNull(0))
//            min = NULL_STRING;
//        else
//            min = buffer->get<std::string>(col_it.first + "MIN").get(0);

//        if (buffer->get<std::string>(col_it.first + "MAX").isNull(0))
//            max = NULL_STRING;
//        else
//            max = buffer->get<std::string>(col_it.first + "MAX").get(0);

//        if (col_it.second->dataFormat() == "")
//            ;
//        else if (col_it.second->dataFormat() == "hexadecimal")
//        {
//            if (min != NULL_STRING)
//                min = std::to_string(std::stoi(min, 0, 16));
//            if (max != NULL_STRING)
//                max = std::to_string(std::stoi(max, 0, 16));
//        }
//        else if (col_it.second->dataFormat() == "octal")
//        {
//            if (min != NULL_STRING)
//                min = std::to_string(std::stoi(min, 0, 8));
//            if (max != NULL_STRING)
//                max = std::to_string(std::stoi(max, 0, 8));
//        }
//        else
//            logwrn << "DBOMinMaxDBJob: processTable: table '" << table.name()
//                   << "' unknown format '" << col_it.second->dataFormat() << "'";

//        minmax.push_back(min);
//        minmax.push_back(max);

//        if (minmax.at(0) == NULL_STRING || minmax.at(1) == NULL_STRING)
//        {
//            logdbg << "DBOMinMaxDBJob: processTable: id " << col_it.first << " object "
//                   << object_.name() << " has NULL values";
//            continue;
//        }

//        if (find(inserted_variable_names.begin(), inserted_variable_names.end(),
//                 std::pair<std::string, std::string>(object_.name(), col_it.first)) ==
//            inserted_variable_names.end())
//        {
//            logdbg << "DBOMinMaxDBJob: processTable: inserting id " << col_it.first << " object "
//                   << object_.name() << " min " << minmax.at(0) << " max " << minmax.at(1);
//            db_interface_.insertMinMax(col_it.first, object_.name(), minmax.at(0), minmax.at(1));
//            inserted_variable_names.push_back(
//                std::pair<std::string, std::string>(object_.name(), col_it.first));
//        }
//    }
//}

/**
 * Iterates over all loadable and existing DBOs, and creates minimum/maximum values for all tables
 * and all columns.
 */
// void PostProcessDBJob::createMinMaxValuesSpecial ()
//{
//    assert (db_interface_);
//    assert (db_interface_->existsMinMaxTable());

//    std::vector <std::string> minmax;

//    std::vector <std::pair <DB_OBJECT_TYPE, std::string> > inserted_variable_names;

//    std::map <DB_OBJECT_TYPE, DBObject*> &dobs =  DBObjectManager::getInstance().getDBObjects ();
//    std::map <DB_OBJECT_TYPE, DBObject*>::iterator dboit;

//    if (dobs.size() == 0)
//        return;

//    for (dboit=dobs.begin(); dboit != dobs.end(); dboit++)
//    {
//        if (!dboit->second->isLoadable() || !db_interface_->exists(dboit->first))
//        {
//            continue;
//        }

//        std::vector<std::string> table_names =
//        dboit->second->getCurrentMetaTable()->getAllTableNamesAsVector();
//        std::vector<std::string>::iterator tableit;

//        for (tableit = table_names.begin(); tableit != table_names.end(); tableit++)
//        {
//            loginf  << "DBInterface: createMinMaxValuesSpecial: getting minimum and maximum of all
//            special variables of table " << *tableit;

//            std::map <std::string, DBTableColumn *> &columns =
//            DBSchemaManager::getInstance().getCurrentSchema()->getTable(*tableit)->getColumns ();
//            std::map <std::string, DBTableColumn *>::iterator varit;

//            DBTableColumn *column;

//            for (varit = columns.begin(); varit != columns.end(); varit++) // over
//            variables/properties
//            {
//                column = varit->second;

//                if (!column->hasSpecialNull())
//                    continue;

//                loginf  << "DBInterface: createMinMaxValuesSpecial: getting min/max for special
//                variable '" << column->getName()<<"'"; DBResult *result =
//                db_interface_->queryMinMaxForColumn (column, *tableit);

//                assert (result->containsData());

//                std::string text;
//                Buffer *buffer = result->getBuffer();

//                logdbg  << "DBInterface: createMinMaxValuesSpecial: setting variables";

//                assert (buffer->getSize() == 1);
//                buffer->setIndex (0);
//                minmax.clear();

//                minmax.push_back (*((std::string*) buffer->get(0)));
//                minmax.push_back (*((std::string*) buffer->get(1)));

//                if (find(inserted_variable_names.begin(), inserted_variable_names.end(),
//                        std::pair<DB_OBJECT_TYPE, std::string> (dboit->first, column->getName()))
//                        == inserted_variable_names.end())
//                {
//                    loginf << "DBInterface: createMinMaxValuesSpecial: inserting id " <<
//                    column->getName() << " type " <<  dboit->first <<
//                            " min " << minmax.at(0) << " max " << minmax.at(1);
//                    db_interface_->insertMinMax(column->getName(), dboit->first, minmax.at(0),
//                    minmax.at(1)); inserted_variable_names.push_back (std::pair<DB_OBJECT_TYPE,
//                    std::string> (dboit->first, column->getName()));
//                }

//                delete result;
//                delete buffer;
//            }
//        }
//    }
//}
