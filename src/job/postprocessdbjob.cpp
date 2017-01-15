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
 * PostProcessDBJob.cpp
 *
 *  Created on: Feb 5, 2013
 *      Author: sk
 */

#include "PostProcessDBJob.h"
#include "DBInterface.h"
#include "SQLGenerator.h"
#include "DBResult.h"
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

PostProcessDBJob::PostProcessDBJob(JobOrderer *orderer, boost::function<void (Job*)> done_function,
        boost::function<void (Job*)> obsolete_function, DBInterface *db_interface)
: DBJob(orderer, done_function, obsolete_function, db_interface)
{
}

PostProcessDBJob::~PostProcessDBJob()
{
}

void PostProcessDBJob::execute ()
{
    loginf  << "PostProcessDBJob: execute: start";

    boost::posix_time::ptime loading_start_time_;
    boost::posix_time::ptime loading_stop_time_;

    loading_start_time_ = boost::posix_time::microsec_clock::local_time();

    db_interface_->updateExists();

    SQLGenerator *sql_generator = db_interface_->getSQLGenerator();


    loginf  << "PostProcessDBJob: execute: creating min max table";

    if (!db_interface_->existsMinMaxTable())
        db_interface_->createMinMaxTable();
    else
        db_interface_->clearTableContent (sql_generator->getMinMaxTableName());

    createMinMaxValuesSpecial();
    createMinMaxValuesNormal();

    loading_stop_time_ = boost::posix_time::microsec_clock::local_time();

    double load_time;
    boost::posix_time::time_duration diff = loading_stop_time_ - loading_start_time_;
    load_time= diff.total_milliseconds()/1000.0;

    loginf  << "PostProcessDBJob: execute: db postprocessing done (" << doubleToString (load_time) << " s).";
    done_=true;
}


/**
 * Iterates over all loadable and existing DBOs, and creates minimum/maximum values for all tables and all columns.
 */
void PostProcessDBJob::createMinMaxValuesSpecial ()
{
    assert (db_interface_);
    assert (db_interface_->existsMinMaxTable());

    std::vector <std::string> minmax;

    std::vector <std::pair <DB_OBJECT_TYPE, std::string> > inserted_variable_names;

    std::map <DB_OBJECT_TYPE, DBObject*> &dobs =  DBObjectManager::getInstance().getDBObjects ();
    std::map <DB_OBJECT_TYPE, DBObject*>::iterator dboit;

    if (dobs.size() == 0)
        return;

    for (dboit=dobs.begin(); dboit != dobs.end(); dboit++)
    {
        if (!dboit->second->isLoadable() || !db_interface_->exists(dboit->first))
        {
            continue;
        }

        std::vector<std::string> table_names = dboit->second->getCurrentMetaTable()->getAllTableNamesAsVector();
        std::vector<std::string>::iterator tableit;

        for (tableit = table_names.begin(); tableit != table_names.end(); tableit++)
        {
            loginf  << "DBInterface: createMinMaxValuesSpecial: getting minimum and maximum of all special variables of table " << *tableit;

            std::map <std::string, DBTableColumn *> &columns =  DBSchemaManager::getInstance().getCurrentSchema()->getTable(*tableit)->getColumns ();
            std::map <std::string, DBTableColumn *>::iterator varit;

            DBTableColumn *column;

            for (varit = columns.begin(); varit != columns.end(); varit++) // over variables/properties
            {
                column = varit->second;

                if (!column->hasSpecialNull())
                    continue;

                loginf  << "DBInterface: createMinMaxValuesSpecial: getting min/max for special variable '" << column->getName()<<"'";
                DBResult *result = db_interface_->queryMinMaxForColumn (column, *tableit);

                assert (result->containsData());

                std::string text;
                Buffer *buffer = result->getBuffer();

                logdbg  << "DBInterface: createMinMaxValuesSpecial: setting variables";

                assert (buffer->getSize() == 1);
                buffer->setIndex (0);
                minmax.clear();

                minmax.push_back (*((std::string*) buffer->get(0)));
                minmax.push_back (*((std::string*) buffer->get(1)));

                if (find(inserted_variable_names.begin(), inserted_variable_names.end(),
                        std::pair<DB_OBJECT_TYPE, std::string> (dboit->first, column->getName())) == inserted_variable_names.end())
                {
                    loginf << "DBInterface: createMinMaxValuesSpecial: inserting id " << column->getName() << " type " <<  dboit->first <<
                            " min " << minmax.at(0) << " max " << minmax.at(1);
                    db_interface_->insertMinMax(column->getName(), dboit->first, minmax.at(0), minmax.at(1));
                    inserted_variable_names.push_back (std::pair<DB_OBJECT_TYPE, std::string> (dboit->first, column->getName()));
                }

                delete result;
                delete buffer;
            }
        }
    }
}

void PostProcessDBJob::createMinMaxValuesNormal ()
{
    assert (db_interface_);
    assert (db_interface_->existsMinMaxTable());

    std::string tmpstr;
    std::vector <std::string> minmax;

    std::vector <std::pair <DB_OBJECT_TYPE, std::string> > inserted_variable_names;

    std::map <DB_OBJECT_TYPE, DBObject*> &dobs =  DBObjectManager::getInstance().getDBObjects ();
    std::map <DB_OBJECT_TYPE, DBObject*>::iterator dboit;

    if (dobs.size() == 0)
        return;

    unsigned int dbo_cnt=0;
    for (dboit=dobs.begin(); dboit != dobs.end(); dboit++)
    {
        if (!dboit->second->isLoadable() || !db_interface_->exists(dboit->first))
        {
            dbo_cnt++;
            continue;
        }

        std::vector<std::string> table_names = dboit->second->getCurrentMetaTable()->getAllTableNamesAsVector();
        std::vector<std::string>::iterator tableit;

        for (tableit = table_names.begin(); tableit != table_names.end(); tableit++)
        {
            loginf  << "DBInterface: createMinMaxValues: getting minimum and maximum of all variables of table " << *tableit;

            logdbg  << "DBInterface: createMinMaxValues: getting command";

            logdbg  << "DBInterface: createMinMaxValues: executing command";
            DBResult *result = db_interface_->queryMinMaxNormalForTable (*tableit);

            assert (result->containsData());

            std::string text;
            Buffer *buffer = result->getBuffer();

            logdbg  << "DBInterface: createMinMaxValues: setting variables";

            assert (buffer->getSize() == 1);

            std::map <std::string, DBTableColumn *> &columns =  DBSchemaManager::getInstance().getCurrentSchema()->getTable(*tableit)->getColumns ();
            std::map <std::string, DBTableColumn *>::iterator varit;

            buffer->setIndex (0);
            unsigned int cnt2=0;
            DBTableColumn *column;

            for (varit = columns.begin(); varit != columns.end(); varit++) // over variables/properties
            {
                minmax.clear();

                column = varit->second;

                if (column->hasSpecialNull())
                {
                    cnt2++;
                    continue;
                }

                for (unsigned int cnt3=0; cnt3 < 2; cnt3++) //over min/max
                {
                    tmpstr.clear();

                    tmpstr = *((std::string*) buffer->get(cnt2*2+cnt3)); // has to be string in sqlgenerator

                    minmax.push_back (tmpstr);

//                    if (tmpstr == "NULL")
//                        loginf << "DBInterface: createMinMaxValues: id " << column->getName() << " type " <<  dboit->first
//                            << (cnt3 == 0 ? " min " : " max ") << "has NULL value";
                }

                assert (minmax.size() == 2);

                //                if (minmax.at(0) == minmax.at(1))
                //                {
                //                    loginf << "DBInterface: createMinMaxValues: id " << column->getName() << " type " <<  dboit->first
                //                            << " has same min/max value '" << minmax.at(0) << "'";
                //                    continue;
                //                }

                if (minmax.at(0) == "NULL" || minmax.at(1) == "NULL")
                {
                    loginf << "DBInterface: createMinMaxValues: id " << column->getName() << " type " <<  dboit->first
                            << " has NULL values";
                    cnt2++;
                    continue;
                }

                if (find(inserted_variable_names.begin(), inserted_variable_names.end(),
                        std::pair<DB_OBJECT_TYPE, std::string> (dboit->first, column->getName())) == inserted_variable_names.end())
                {
                    loginf << "DBInterface: createMinMaxValues: inserting id " << column->getName() << " type " <<  dboit->first <<
                            " min " << minmax.at(0) << " max " << minmax.at(1);
                    db_interface_->insertMinMax(column->getName(), dboit->first, minmax.at(0), minmax.at(1));
                    inserted_variable_names.push_back (std::pair<DB_OBJECT_TYPE, std::string> (dboit->first, column->getName()));
                }
                cnt2++;
            }
            delete result;
            //delete command;
            delete buffer;
        }

        dbo_cnt++;
    }

}

