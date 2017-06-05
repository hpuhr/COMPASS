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
 * StructureReader.cpp
 *
 *  Created on: Jan 24, 2012
 *      Author: sk
 */

#include "buffer.h"
#include "config.h"
#include "dbinterface.h"
#include "bufferset.h"
#include "dbobjectmanager.h"
#include "logger.h"
#include "structurereader.h"
#include "structurevariable.h"
#include "structuredescriptionmanager.h"
#include "structuredescription.h"
#include "propertylist.h"
//#include "WriteBufferDBJob.h"

//#include "Data.h"

//using namespace Utils::Data;

StructureReader::StructureReader(DBInterface *db_interface)
    : db_interface_(db_interface)
{
    logdbg  << "StructureReader: constructor";

    std::map <DB_OBJECT_TYPE, StructureDescription *> &descriptions = StructureDescriptionManager::getInstance().getStructureDescriptions ();
    std::map <DB_OBJECT_TYPE, StructureDescription *>::iterator it;

    for (it = descriptions.begin(); it != descriptions.end(); it++)
    {
        mutexes_[it->first] = new boost::mutex ();
        descriptions_[it->first] = it->second;
        property_lists_[it->first] = it->second->convert();
        //data_[it->first] = getNewBuffer(it->first);
    }

    //data_set_ = new BufferSet ();
}

StructureReader::~StructureReader()
{
    logdbg  << "StructureReader: destructor";

    std::map <DB_OBJECT_TYPE, boost::mutex *>::iterator it;

    for (it=mutexes_.begin(); it != mutexes_.end(); it++)
    {
        delete it->second;
    }
    mutexes_.clear();

    std::map <DB_OBJECT_TYPE, Buffer *>::iterator it2;
    for (it2=data_.begin(); it2 != data_.end(); it2++)
    {
        delete it2->second;
    }
    data_.clear();

    std::map <DB_OBJECT_TYPE, PropertyList*>::iterator it3;
    for (it3=property_lists_.begin(); it3 != property_lists_.end(); it3++)
    {
        delete it3->second;
    }
    property_lists_.clear();
}

void StructureReader::add (DB_OBJECT_TYPE type, void *data)
{
    logdbg  << "StructureReader: add";

    boost::mutex::scoped_lock (*mutexes_.at(type));

    if (data_.find(type) == data_.end())
        data_[type] = getNewBuffer(type);

    Buffer *target = data_.at(type);
    StructureDescription *desc = descriptions_.at(type);

    PropertyList *list = target->getPropertyList();
    std::vector <Property*> *properties = list->getProperties ();

    assert (desc->getSize() == list->getNumProperties());

    unsigned int size = desc->getSize();
    if (!target->getFirstWrite())
        target->incrementIndex();
    else
        target->unsetFirstWrite();

    std::vector <void *> *adresses = target->getAdresses();
    void *ptr;

    StructureVariable *var;
    StructureVariable *var_present;
    for (unsigned int cnt=0; cnt < size; cnt++)
    {
        var = desc->getVariableAt(cnt);
        logdbg  << "StructureReader: add: var " << var->getNumber() << " " << var->getId() << " type " << var->getType() << " data add " << data << " off " << var->getOffset();
        // assumes that memory is cleared, which it is (ArrayTemplate), and that the same order is in StructureDescription and PropertyList of PropertyContainer

        //if (var->getId().compare("radar_number") == 0)
        //  loginf  << "StructureReader: add: var " << var->getNumber() << " " << var->getId() << " type " << var->getType() << " data value " << *(short int*) ((unsigned char*)data + var->getOffset()) << " off " << var->getOffset();

        ptr = adresses->at(cnt);

        if (var->hasPresentVariable())
        {
            var_present = var->getPresentVariable();
            bool present = *(bool*)((unsigned char*)data + var_present->getOffset());
            if (!present)
            {
                setNan(properties->at(cnt)->data_type_int_, ptr);
                //loginf  << "StructureReader: add: got not present at type " << (int)type<< " var " << var->getId();
                continue;
            }
        }

        if (var->getType() == SE_TYPE_VARCHAR)
        {
            std::string *tmp = (std::string *)ptr;
            tmp->assign (*(char **) ((unsigned char*)data + var->getOffset()));
            logdbg  << "StructureReader: add: varchar src '" << *((char **) ((unsigned char*)data + var->getOffset())) << "' dst '" << *tmp  << "' size " << tmp->size();
        }
        else if (var->getType() == SE_TYPE_VARCHAR_ARRAY)
        {
            std::string *tmp = (std::string *)ptr;
            tmp->assign ((char *) data + var->getOffset());
            logdbg  << "StructureReader: add: varchararray src '" << (char *) data + var->getOffset() << "' dst '" << *tmp << "' size " << tmp->size();
        }
        else
            memcpy (ptr,(unsigned char*)data + var->getOffset(), var->getSize());

    }

    if (target->isFull())
    {
        logdbg  << "StructureReader: add: buffer is full: " << target->getSize();
        Buffer *buffer = target->transferData();
        writeBuffer (buffer);
        //data_set_->pushBuffer(buffer);
        //logdbg  << "StructureReader: add: old buffer: " << target->getSize();
        logdbg  << "StructureReader: add: new buffer is full: " << buffer->getSize();
    }
}

bool StructureReader::hasUnwrittenData ()
{
    std::map <DB_OBJECT_TYPE, Buffer *>::iterator it2;
    for (it2=data_.begin(); it2 != data_.end(); it2++)
    {
        if (!it2->second->getFirstWrite())
            return true;
    }
    return false;
}

void StructureReader::finalize ()
{
    std::map <DB_OBJECT_TYPE, Buffer *>::iterator it2;
    for (it2=data_.begin(); it2 != data_.end(); it2++)
    {
        if (!it2->second->getFirstWrite())
            writeBuffer(it2->second);
    }
    data_.clear();
}

Buffer *StructureReader::getNewBuffer (DB_OBJECT_TYPE type)
{
    //assert (DBObjectManager::getInstance().existsDBObject(type));

    if (type == DBO_UNDEFINED)
        throw std::runtime_error ("StructureReader: getNewBuffer: not defined for dbo undefined");

    Buffer * buffer = new Buffer (*property_lists_.at(type),type);
    return buffer;
}

void StructureReader::writeBuffer (Buffer *buffer)
{
    // TODO
    //    WriteBufferDBJob *job = new WriteBufferDBJob (this, boost::bind( &StructureReader::writeBufferDone, this, _1 ),
    //        boost::bind( &StructureReader::jobAborted, this, _1 ), db_interface_, buffer);
}

void StructureReader::writeBufferDone( Job *job )
{
    WriteBufferDBJob *write_job = (WriteBufferDBJob*)job;
    delete write_job->getBuffer();
    delete job;
}
void StructureReader::jobAborted( Job *job )
{
    WriteBufferDBJob *write_job = (WriteBufferDBJob*)job;
    delete write_job->getBuffer();
    delete job;
}

