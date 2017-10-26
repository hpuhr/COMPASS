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

#include <iomanip>
#include <algorithm>
#include "buffer.h"
#include "bufferwriter.h"
#include "dbconnection.h"
#include "sqlgenerator.h"
#include "logger.h"

//#include "data.h"
#include "stringconv.h"

using namespace Utils;

BufferWriter::BufferWriter(DBConnection *db_connection, SQLGenerator *sql_generator)
{
    logdbg  << "BufferWriter: constructor";
    db_connection_=db_connection;
    sql_generator_=sql_generator;
}

BufferWriter::~BufferWriter()
{
}

/**
 * Creates the table doesn't exist (wasn't created). If one was created, also creates a bind statement using the SQLGenerator.
 * Prepares the bound statement, begins a transaction (performance), steps throuth the buffer and calls
 * insertBindStatementForCurrentIndex. Then ends and finalizes the bound statement.
 */
//void BufferWriter::write (Buffer *data, std::string tablename)
//{
//    logdbg  << "BufferWriter: write: buffer size " << data->size() << " into table " << tablename;
//    if (!existsTableForBuffer(tablename))
//    {
//        createTableForBuffer (data, tablename);
//        std::string bind_statement = sql_generator_->createDBInsertStringBind(data, tablename);
//        created_binds_.insert(std::pair<std::string, std::string> (tablename, bind_statement));
//    }
//    assert (existsTableForBuffer(tablename));

//    std::string bind_statement =  created_binds_ [tablename];

//    logdbg  << "BufferWriter: write: preparing bind statement";
//    db_connection_->prepareBindStatement(bind_statement);
//    db_connection_->beginBindTransaction();

//    // TODO INSERT DATA HERE
////    data->setIndex(0);

////    logdbg  << "BufferWriter: write: starting inserts";
////    for (unsigned int cnt=0; cnt < data->getSize(); cnt++)
////    {
////        if (cnt != 0)
////            data->incrementIndex();

////        insertBindStatementForCurrentIndex(data);
////    }

//    logdbg  << "BufferWriter: write: ending bind transactions";
//    db_connection_->endBindTransaction();
//    logdbg  << "BufferWriter: write: finalizing bind statement";
//    db_connection_->finalizeBindStatement();

//    logdbg  << "BufferWriter: write: end";
//}

void BufferWriter::update (std::shared_ptr<Buffer> buffer, DBObject &object, DBOVariable &key_var, std::string tablename)
{
    loginf  << "BufferWriter: update: buffer size " << buffer->size() << " into table " << tablename;

    std::string bind_statement =  sql_generator_->createDBUpdateStringBind(buffer, object, key_var, tablename);

    loginf  << "BufferWriter: update: preparing bind statement";
    db_connection_->prepareBindStatement(bind_statement);
    db_connection_->beginBindTransaction();

    loginf  << "BufferWriter: update: starting inserts";
    for (unsigned int cnt=0; cnt < buffer->size(); cnt++)
    {
        insertBindStatementUpdateForCurrentIndex(buffer, cnt);

        if (cnt % 100000 == 0)
            loginf  << "BufferWriter: update: bind transactions cnt " << cnt;
    }

    loginf  << "BufferWriter: update: ending bind transactions";
    db_connection_->endBindTransaction();
    loginf  << "BufferWriter: update: finalizing bind statement";
    db_connection_->finalizeBindStatement();

    logdbg  << "BufferWriter: update: end";
}

/**
 * Steps through all properties and binds the variables to the bound statement, the calls stepAndClearBindings.
 * Relies on the auto-increment function since value is bound to the key.
 *
 * \exception std::runtime_error if unkown database type
 */
//void BufferWriter::insertBindStatementForCurrentIndex (Buffer *buffer)
//{
//    assert (buffer);
//    logdbg  << "BufferWriter: insertBindStatementForCurrentIndex: start";
//    const PropertyList &list =buffer->properties();
//    unsigned int size = list.size();
//    logdbg  << "BufferWriter: insertBindStatementForCurrentIndex: creating bind for " << size << " elements";

//    //DB_CONNECTION_TYPE db_type_=db_connection_->getDBInfo()->getType();

//    // TODO INSERT DATA HERE
////    std::vector <void *> *adresses = buffer->getAdresses();
////    void *ptr;
////    unsigned int index_cnt=0;

////    logdbg << "BufferWriter: insertBindStatementForCurrentIndex: starting for loop";
////    for (unsigned int cnt=0; cnt < size; cnt++)
////    {
////        Property *prop = list->getProperty(cnt);
////        logdbg  << "BufferWriter: insertBindStatementForCurrentIndex: for at cnt " << cnt;

////        assert (prop);

////        ptr = adresses->at(cnt);

////        if (db_type_ == DB_TYPE_SQLITE)
////            index_cnt=cnt+2;
////        else if (db_type_ == DB_TYPE_MYSQLpp)
////            index_cnt=cnt;
////        else if (db_type_ == DB_TYPE_MYSQLCon)
////            index_cnt=cnt+1;
////        else
////            throw std::runtime_error ("BufferWriter: insertBindStatementForCurrentIndex: unknown db type");

////        if (isNan(prop->data_type_int_, ptr))
////        {
////            db_connection_->bindVariableNull (index_cnt);
////            continue;
////        }
////        PROPERTY_DATA_TYPE type = (PROPERTY_DATA_TYPE) prop->data_type_int_;

////        switch (type)
////        {
////        case P_TYPE_UCHAR:
////        case P_TYPE_CHAR:
////        case P_TYPE_BOOL:
////            logdbg  << "BufferWriter: insertBindStatementForCurrentIndex: loop byte";
////            db_connection_->bindVariable (index_cnt, (int)*((unsigned char*)ptr));
////            break;
////        case P_TYPE_INT:
////        case P_TYPE_UINT:
////            logdbg  << "BufferWriter: insertBindStatementForCurrentIndex: loop int";
////            db_connection_->bindVariable (index_cnt, *((int*)ptr));
////            break;
////        case P_TYPE_STRING:
////            logdbg  << "BufferWriter: insertBindStatementForCurrentIndex: loop string '" << *((std::string*)ptr) << "' size " << ((std::string*)ptr)->size();
////            if (db_type_ == DB_TYPE_SQLITE)
////                db_connection_->bindVariable (index_cnt, (*((std::string*)ptr)).c_str());
////            else if (db_type_ == DB_TYPE_MYSQLpp || db_type_ == DB_TYPE_MYSQLCon)
////                db_connection_->bindVariable (index_cnt, (*((std::string*)ptr)).c_str());
////                //db_connection_->bindVariable (index_cnt, ("'"+*((std::string*)ptr)+"'").c_str());
////            break;
////        case P_TYPE_FLOAT:
////        case P_TYPE_DOUBLE:
////            logdbg  << "BufferWriter: insertBindStatementForCurrentIndex: loop double";
////            db_connection_->bindVariable (index_cnt, *((double*)ptr));
////            break;
////        default:
////            throw std::runtime_error("SQLGenerator: insertBindStatementForCurrentIndex: unspecified data type "+intToString(type));
////        }
////    }

//    db_connection_->stepAndClearBindings();

//    logdbg  << "BufferWriter: insertBindStatementForCurrentIndex: done";
//}

void BufferWriter::insertBindStatementUpdateForCurrentIndex (std::shared_ptr<Buffer> buffer, unsigned int row)
{
    assert (buffer);
    logdbg  << "BufferWriter: insertBindStatementUpdateForCurrentIndex: start";
    const PropertyList &list =buffer->properties();
    unsigned int size = list.size();
    logdbg  << "BufferWriter: insertBindStatementUpdateForCurrentIndex: creating bind for " << size << " elements";

    std::string connection_type = db_connection_->type();

    assert (connection_type == MYSQL_IDENTIFIER || connection_type == SQLITE_IDENTIFIER);

    unsigned int index_cnt=0;

    logdbg << "BufferWriter: insertBindStatementUpdateForCurrentIndex: starting for loop";
    for (unsigned int cnt=0; cnt < size; cnt++)
    {
        const Property &property = list.at(cnt);
        PropertyDataType data_type = property.dataType();

        logdbg  << "BufferWriter: insertBindStatementUpdateForCurrentIndex: for at cnt " << cnt << " id " << property.name();

        if (connection_type == SQLITE_IDENTIFIER)
            index_cnt=cnt+2;
        else if (connection_type == MYSQL_IDENTIFIER)
            index_cnt=cnt+1;
        else
            throw std::runtime_error ("BufferWriter: insertBindStatementForCurrentIndex: unknown db type");

        if (buffer->isNone(property, row))
        {
            db_connection_->bindVariableNull (index_cnt);
            continue;
        }

        switch (data_type)
        {
        case PropertyDataType::BOOL:
            db_connection_->bindVariable (index_cnt, static_cast<int> (buffer->getBool(property.name()).get(row)));
            break;
        case PropertyDataType::CHAR:
            db_connection_->bindVariable (index_cnt, static_cast<int> (buffer->getChar(property.name()).get(row)));
            break;
        case PropertyDataType::UCHAR:
            db_connection_->bindVariable (index_cnt, static_cast<int> (buffer->getUChar(property.name()).get(row)));
            break;
        case PropertyDataType::INT:
            db_connection_->bindVariable (index_cnt, static_cast<int> (buffer->getInt(property.name()).get(row)));
            break;
        case PropertyDataType::UINT:
            assert (false);
            break;
        case PropertyDataType::LONGINT:
            assert (false);
            break;
        case PropertyDataType::ULONGINT:
            assert (false);
            break;
        case PropertyDataType::FLOAT:
            db_connection_->bindVariable (index_cnt, static_cast<double> (buffer->getFloat(property.name()).get(row)));
            break;
        case PropertyDataType::DOUBLE:
            db_connection_->bindVariable (index_cnt, buffer->getDouble(property.name()).get(row));
            break;
        case PropertyDataType::STRING:
            if (connection_type == SQLITE_IDENTIFIER)
                db_connection_->bindVariable (index_cnt, buffer->getString(property.name()).get(row));
            else //MYSQL assumed
                db_connection_->bindVariable (index_cnt, "'"+buffer->getString(property.name()).get(row)+"'");
            break;
        default:
            logerr  <<  "Buffer: insertBindStatementUpdateForCurrentIndex: unknown property type " << Property::asString(data_type);
            throw std::runtime_error ("Buffer: insertBindStatementUpdateForCurrentIndex: unknown property type "+Property::asString(data_type));
        }
    }

    db_connection_->stepAndClearBindings();

    logdbg  << "BufferWriter: insertBindStatementUpdateForCurrentIndex: done";
}

//void BufferWriter::createTableForBuffer (Buffer *data, std::string tablename)
//{
//    std::string create_statement=sql_generator_->createDBCreateString (data, tablename);
//    db_connection_->executeSQL(create_statement);

//    created_tables_.push_back (tablename);
//}
//bool BufferWriter::existsTableForBuffer (std::string tablename)
//{
//    std::vector<std::string>::iterator it;

//    it = find(created_tables_.begin(), created_tables_.end(), tablename);
//    return (it != created_tables_.end());
//}

