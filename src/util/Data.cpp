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
 * DataManipulation.cpp
 *
 *  Created on: Mar 3, 2014
 *      Author: sk
 */

#include "Data.h"
#include "Logger.h"
#include "Buffer.h"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <cstring>
#include "String.h"
#include "DBOVariable.h"
#include "DBSchemaManager.h"
#include "DBSchema.h"
#include "MetaDBTable.h"
#include "DBTableColumn.h"
#include "UnitManager.h"

using namespace Utils::String;

namespace Utils
{
namespace Data
{

void setNan (unsigned int data_type, void *ptr)
{
    switch (data_type)
    {
    case P_TYPE_BOOL:
    {
        *((bool*)ptr) = false;
    }
    break;
    case P_TYPE_UCHAR:
    {
        *((unsigned char*)ptr) = 0;
    }
    break;
    case P_TYPE_CHAR:
    {
        *((char*)ptr) = 0;
    }
    break;
    case P_TYPE_INT:
    {
        *((int*)ptr) = std::numeric_limits<int>::max();
        //*((int*)ptr) = std::numeric_limits<int>::quiet_NaN();
    }
    break;
    case P_TYPE_UINT:
    {
        *((unsigned int*)ptr) = std::numeric_limits<unsigned int>::max();
    }
    break;
    case P_TYPE_STRING:
    {
        *((std::string*)ptr) = "NULL";
    }
    break;
    case P_TYPE_FLOAT:
    {
        *((float*)ptr) = std::numeric_limits<float>::quiet_NaN();
    }
    break;
    case P_TYPE_DOUBLE:
    {
        *((double*)ptr) = std::numeric_limits<double>::quiet_NaN();
    }
    break;
    case P_TYPE_POINTER:
    {
        *((void**)ptr) = NULL;
    }
    break;
    default:
        //logerr  <<  "Property: setNan: unknown property type";
        throw std::runtime_error ("Util: setNan: unknown property type");
    }
}
bool isNan (unsigned int data_type, void *ptr)
{
    switch (data_type)
    {
    case P_TYPE_BOOL:
    {
        return false;
    }
    break;
    case P_TYPE_UCHAR:
    {
        return false;
    }
    break;
    case P_TYPE_CHAR:
    {
        return false;
    }
    break;
    case P_TYPE_INT:
    {
        return *((int*)ptr) == std::numeric_limits<int>::max();
        //return *((int*)ptr) != *((int*)ptr);    //TODO: Too compiler/platform dependent!
    }
    break;
    case P_TYPE_UINT:
    {
        return *((unsigned int*)ptr) == std::numeric_limits<unsigned int>::max();
    }
    break;
    case P_TYPE_STRING:
    {
        return (((std::string*)ptr)->size() == 0) || ((std::string*)ptr)->compare("NULL") == 0;
        //return false;
    }
    break;
    case P_TYPE_FLOAT:
    {
        return *((float*)ptr) != *((float*)ptr);    //TODO: Too compiler/platform dependent!
    }
    break;
    case P_TYPE_DOUBLE:
    {
        return *((double*)ptr) != *((double*)ptr);    //TODO: Too compiler/platform dependent!
    }
    break;
    case P_TYPE_POINTER:
    {
        return *((void**)ptr) != NULL;
    }
    break;
    default:
        //logerr  <<  "Property: isNan: unknown property type";
        throw std::runtime_error ("Util: isNan: unknown property type "+intToString (data_type));
        //return false;
    }

}
void clear (unsigned int data_type, void *ptr)
{
    switch (data_type)
    {
    case P_TYPE_BOOL:
        *(bool *)ptr = false;
        break;
    case P_TYPE_UCHAR:
        *(unsigned char *)ptr = 0;
        break;
    case P_TYPE_CHAR:
        *(char *)ptr = 0;
        break;
    case P_TYPE_INT:
        *(int *)ptr = 0;
        break;
    case P_TYPE_UINT:
        *(unsigned int *)ptr = 0;
        break;
    case P_TYPE_STRING:
        *(std::string *)ptr = "";
        break;
    case P_TYPE_FLOAT:
        *(float *)ptr = 0.0;
        break;
    case P_TYPE_DOUBLE:
        *(double *)ptr = 0.0;
        break;
    case P_TYPE_POINTER:
        *((void**)ptr) = 0;
        break;
    case P_TYPE_LONGINT:
        *(long int *)ptr=0;
        break;
    case P_TYPE_ULONGINT:
        *(long unsigned int *)ptr=0;
        break;
    default:
        logerr  <<  "Util: clear: unknown property type";
        throw std::runtime_error ("Util: Util: unknown property type");
    }
}
void copy (void *src, void *dest, unsigned int data_type, unsigned int num_bytes, bool reverse, bool verbose)
{
    clear(data_type, dest);

    if (!(num_bytes > 0 || data_type == P_TYPE_STRING || data_type == P_TYPE_FLOAT ||
            data_type == P_TYPE_DOUBLE || data_type == P_TYPE_POINTER))
        throw std::runtime_error ("Util: copy: num_bytes "+intToString(num_bytes)+" or wrong data type "+PROPERTY_DATA_TYPE_STRINGS[(PROPERTY_DATA_TYPE)data_type]);

    //logdbg << "Util: copy: value after clear " << getPropertyValueString(dest, (PROPERTY_DATA_TYPE) data_type);

    if (verbose)
    {
        std::stringstream ss;
        if (reverse)
        {
            for (unsigned int cnt=0; cnt < num_bytes; cnt++)
            {
                ss << std::hex << (unsigned int) *((unsigned char*)src+num_bytes-cnt-1) << " ";
            }
        }
        else
        {
            for (unsigned int cnt=0; cnt < num_bytes; cnt++)
            {
                ss << std::hex << (unsigned int) *((unsigned char*)src+cnt) << " ";
            }
        }
        logdbg << "Util: copy: address " << std::hex << src << " bytes " << num_bytes << " data " << ss.str() << " rev " << reverse;
    }

    switch (data_type)
    {
    case P_TYPE_BOOL:
    case P_TYPE_UCHAR:
    case P_TYPE_CHAR:
    {
        assert (num_bytes == 1);
        memcpy(dest, src, num_bytes);
    }
    break;
    case P_TYPE_INT:
    case P_TYPE_UINT:
    {
        assert (num_bytes <= 4);
        if (reverse)
            memcpy (dest, src, num_bytes);
        else
            memcpy_reverse((char*)dest, (char*)src, num_bytes,verbose);
    }
    break;
    case P_TYPE_STRING:
    {
        if (reverse)
            throw std::runtime_error ("Util: copy: cannot reverse string");
        if (num_bytes == 0)
            *(std::string *)dest = *(std::string *)src;
        else
        {
            char tmp[num_bytes+1];
            memcpy(tmp, src, num_bytes);
            tmp [num_bytes] = '\0';
            *(std::string *)dest = tmp;
        }
    }
    break;
    case P_TYPE_FLOAT:
    {
        if (reverse)
            throw std::runtime_error ("Util: copy: cannot reverse float");

        *(float *)dest = *(float *)src;
    }
    break;
    case P_TYPE_DOUBLE:
    {
        if (reverse)
            throw std::runtime_error ("Util: copy: cannot reverse double");

        *(double *)dest = *(double *)src;
    }
    break;
    case P_TYPE_POINTER:
    {
        if (reverse)
            throw std::runtime_error ("Util: copy: cannot reverse pointer");

        *(void **)dest = *(void **)src;
    }
    break;
    case P_TYPE_LONGINT:
    case P_TYPE_ULONGINT:
    {
        assert (num_bytes <= 8);
        if (reverse)
            memcpy (dest, src, num_bytes);
        else
            memcpy_reverse((char*)dest, (char*)src, num_bytes,verbose);
    }
    break;
    default:
        logerr  <<  "Util: copy: unknown property type";
        throw std::runtime_error ("Util: copy: unknown property type");
    }

    if (verbose)
        logdbg << "Util: copy: value after copy " << getPropertyValueString(dest, (PROPERTY_DATA_TYPE) data_type);
}

void add (void *data, unsigned int data_type, int constant)
{
    if (data_type == P_TYPE_STRING || data_type == P_TYPE_POINTER || data_type == P_TYPE_BOOL)
        throw std::runtime_error ("Util: add: wrong data type "+PROPERTY_DATA_TYPE_STRINGS[(PROPERTY_DATA_TYPE)data_type]);

    //TODO over/underflow problem
//    if (constant < 0 && (data_type == P_TYPE_UCHAR || data_type == P_TYPE_UINT || data_type == P_TYPE_ULONGINT))
//        throw std::runtime_error ("Util: add: negative constant "+intToString(constant)+" not allowed for data type "
//                +PROPERTY_DATA_TYPE_STRINGS[(PROPERTY_DATA_TYPE)data_type]);

    switch (data_type)
    {
    case P_TYPE_UCHAR:
        *(unsigned char*) data += constant;
        break;
    case P_TYPE_CHAR:
        *(char*) data += constant;
        break;
    case P_TYPE_INT:
        *(int*) data += constant;
        break;
    case P_TYPE_UINT:
        *(unsigned int*) data += constant;
        break;
    case P_TYPE_FLOAT:
        *(float*) data += constant;
        break;
    case P_TYPE_DOUBLE:
        *(double*) data += constant;
        break;
    case P_TYPE_LONGINT:
        *(long int *) data +=constant;
        break;
    case P_TYPE_ULONGINT:
        *(long unsigned int *) data *=constant;
        break;
    default:
        logerr  <<  "Util: add: unknown property type";
        throw std::runtime_error ("Util: add: unknown property type");
    }
}

void memcpy_reverse (char *dest, char* src, unsigned int num_bytes, bool verbose)
{
    for (unsigned int cnt=0; cnt < num_bytes; cnt++)
    {
        if (verbose)
            logdbg << "Util: memcpy_reverse: byte " << cnt << " num " << num_bytes << " val " << std::hex << (int)src[cnt];
        dest[num_bytes-cnt-1]=src[cnt];
    }
}

void check_reverse (char *data, char*check_value, unsigned int num_bytes)
{
    for (unsigned int cnt=0; cnt < num_bytes; cnt++)
    {
        if (data[num_bytes-cnt-1] != check_value[cnt])
        {
            unsigned char value = data[num_bytes-cnt-1];
            unsigned char check = check_value[cnt];
            throw std::runtime_error ("Util: check_reverse: byte "+intToString(cnt)+" data "+uIntToString(value)+" check "+uIntToString(check));
        }
    }
    //loginf << "check_reverse approves";
}

void setSpecialNullsNan (Buffer *buffer, unsigned int column, unsigned int data_type, std::string special_null)
{
    assert (buffer);
    assert (special_null.size() > 0);

    buffer->setIndex( 0 );

    std::vector<void*>* output_adresses;
    unsigned int cnt;
    unsigned n = buffer->getSize();


    switch (data_type)
    {
    case P_TYPE_BOOL:
    {
        bool special_null_value = intFromString(special_null);
        for( cnt=0; cnt<n; ++cnt )
        {
            if( cnt != 0 )
            {
                buffer->incrementIndex();
            }

            output_adresses = buffer->getAdresses();

            if (isNan(data_type, output_adresses->at( column )))
                continue;

            if (special_null_value == *((bool*)output_adresses->at( column )))
                setNan(data_type, output_adresses->at( column ));
        }
    }
    break;
    case P_TYPE_UCHAR:
    {
        unsigned char special_null_value = intFromString(special_null);
        for( cnt=0; cnt<n; ++cnt )
        {
            if( cnt != 0 )
            {
                buffer->incrementIndex();
            }

            output_adresses = buffer->getAdresses();

            if (isNan(data_type, output_adresses->at( column )))
                continue;

            if (special_null_value == *((unsigned char*)output_adresses->at( column )))
                setNan(data_type, output_adresses->at( column ));
        }
    }
    break;
    case P_TYPE_CHAR:
    {
        char special_null_value = intFromString(special_null);
        for( cnt=0; cnt<n; ++cnt )
        {
            if( cnt != 0 )
            {
                buffer->incrementIndex();
            }

            output_adresses = buffer->getAdresses();

            if (isNan(data_type, output_adresses->at( column )))
                continue;

            if (special_null_value == *((char*)output_adresses->at( column )))
                setNan(data_type, output_adresses->at( column ));
        }

    }
    break;
    case P_TYPE_INT:
    {
        int special_null_value = intFromString(special_null);
        for( cnt=0; cnt<n; ++cnt )
        {
            if( cnt != 0 )
            {
                buffer->incrementIndex();
            }

            output_adresses = buffer->getAdresses();

            if (isNan(data_type, output_adresses->at( column )))
                continue;

            if (special_null_value == *((int*)output_adresses->at( column )))
                setNan(data_type, output_adresses->at( column ));
        }

    }
    break;
    case P_TYPE_UINT:
    {
        unsigned int special_null_value = intFromString(special_null);
        for( cnt=0; cnt<n; ++cnt )
        {
            if( cnt != 0 )
            {
                buffer->incrementIndex();
            }

            output_adresses = buffer->getAdresses();

            if (isNan(data_type, output_adresses->at( column )))
                continue;

            if (special_null_value == *((unsigned int*)output_adresses->at( column )))
                setNan(data_type, output_adresses->at( column ));
        }

    }
    break;
    case P_TYPE_STRING:
    {
        for( cnt=0; cnt<n; ++cnt )
        {
            if( cnt != 0 )
            {
                buffer->incrementIndex();
            }

            output_adresses = buffer->getAdresses();

            if (isNan(data_type, output_adresses->at( column )))
                continue;

            if (special_null.compare( *((std::string*)output_adresses->at( column ))) == 0)
                setNan(data_type, output_adresses->at( column ));
        }

    }
    break;
    case P_TYPE_FLOAT:
    {
        float special_null_value = doubleFromString(special_null);
        for( cnt=0; cnt<n; ++cnt )
        {
            if( cnt != 0 )
            {
                buffer->incrementIndex();
            }

            output_adresses = buffer->getAdresses();

            if (isNan(data_type, output_adresses->at( column )))
                continue;

            if (special_null_value == *((float*)output_adresses->at( column )))
                setNan(data_type, output_adresses->at( column ));
        }

    }
    break;
    case P_TYPE_DOUBLE:
    {
        double special_null_value = doubleFromString(special_null);
        for( cnt=0; cnt<n; ++cnt )
        {
            if( cnt != 0 )
            {
                buffer->incrementIndex();
            }

            output_adresses = buffer->getAdresses();

            if (isNan(data_type, output_adresses->at( column )))
                continue;

            if (special_null_value == *((double*)output_adresses->at( column )))
                setNan(data_type, output_adresses->at( column ));
        }

    }
    break;
    case P_TYPE_POINTER:
    {
        throw std::runtime_error ("Util: setSpecialNullsNan: unknown for property type pointer");
    }
    break;
    default:
        //logerr  <<  "Property: isNan: unknown property type";
        throw std::runtime_error ("Util: setSpecialNullsNan: unknown property type "+intToString (data_type));
        //return false;
    }

}


void multiplyData (void *ptr, PROPERTY_DATA_TYPE data_type, double factor)
{
    if (data_type == P_TYPE_CHAR)
        *(char*)ptr *= factor;
    else if (data_type == P_TYPE_INT)
        *(int*)ptr *= factor;
    else if (data_type == P_TYPE_UCHAR)
        *(unsigned char*)ptr *= factor;
    else if (data_type == P_TYPE_UINT)
        *(unsigned int*)ptr *= factor;
    else if (data_type == P_TYPE_FLOAT)
    {
        //loginf << "pre" << *(float*)ptr;
        *(float*)ptr *= factor;
        //loginf << "post"  << *(float*)ptr;
    }
    else if (data_type == P_TYPE_DOUBLE)
    {
        //loginf << "pre" << *(double*)ptr;
        *(double*)ptr *= factor;
        //loginf << "post"  << *(double*)ptr;

    }
    else
        throw std::runtime_error ("Util: multiplyData: incorrect data type "+intToString (data_type));
}

void copyPropertyData (void *src, void *target, Property *property)
{
    if (property->data_type_int_ == P_TYPE_STRING)
    {
        ((std::string *) target)->assign (*(std::string*) src);
    }
    else
        memcpy (src,target, property->size_);
}

void finalizeDBData (DB_OBJECT_TYPE type, Buffer *buffer, DBOVariableSet read_list)
{
    logdbg << "Util: finalizeDBData";

    assert (type != DBO_UNDEFINED);

    DBOVariableSet *type_set = read_list.getFor (type);
    std::vector <DBOVariable*> &variables =type_set->getSet();
    std::vector <DBOVariable*>::iterator it;

    PropertyList *buffer_list = buffer->getPropertyList ();

    logdbg << "Util: finalizeDBData: vars " << buffer_list->getNumProperties();

    for (it = variables.begin(); it != variables.end(); it++)
    {
        DBOVariable *variable = *it;

        assert (!variable->isMetaVariable());

        std::string meta_tablename = variable->getCurrentMetaTable ();
        std::string table_varname = variable->getCurrentVariableName ();

        DBTableColumn *table_column = DBSchemaManager::getInstance().getCurrentSchema ()->getMetaTable(meta_tablename)->getTableColumn(table_varname);

        logdbg  << "Util: finalizeDBData: type " << type << " variable " << table_column->getName();
        if (!buffer_list->hasProperty(table_column->getName()))
        {
            logwrn  << "Util: finalizeDBData: variable information not synchronized";
            continue;
        }

        unsigned col = buffer_list->getPropertyIndex(table_column->getName());
        PROPERTY_DATA_TYPE data_type = (PROPERTY_DATA_TYPE) buffer->getPropertyList()->getProperty(col)->data_type_int_;

        if (table_column->hasSpecialNull())
        {
            logdbg << "Util: finalizeDBData: setting special null '" << table_column->getSpecialNull()
                                                            << "' col " << col;
            setSpecialNullsNan (buffer, col, data_type, table_column->getSpecialNull());
        }

        if (variable->hasUnit () || table_column->hasUnit())
        {
            logdbg  << "Util: finalizeDBData: unit var type " << variable->getDBOType() << " dim '" << variable->getUnitDimension() << "'";
            if (variable->hasUnit () != table_column->hasUnit())
            {
                logerr << "Util: finalizeDBData: unit transformation inconsistent: var " << variable->getName () << " has unit " << variable->hasUnit ()
                                                                                  << " table column " << table_column->getName() << " has unit " << table_column->hasUnit();
                continue;
            }

            if (variable->getUnitDimension().compare(table_column->getUnitDimension()) != 0)
            {
                logerr << "Util: finalizeDBData: unit transformation inconsistent: var " << variable->getName () << " has dimension " << variable->getUnitDimension ()
                                                                                  << " table column " << table_column->getName() << " has dimension " << table_column->getUnitDimension();
                continue;
            }

            Unit *unit = UnitManager::getInstance().getUnit (variable->getUnitDimension());
            double factor = unit->getFactor (table_column->getUnitUnit(), variable->getUnitUnit());
            logdbg  << "Util: finalizeDBData: correct unit transformation with factor " << factor;

            buffer->setIndex( 0 );

            std::vector<void*>* output_adresses;
            unsigned int cnt;
            unsigned n = buffer->getSize();

            for( cnt=0; cnt<n; ++cnt )
            {
                if( cnt != 0 )
                {
                    assert (buffer);
                    buffer->incrementIndex();
                }

                output_adresses = buffer->getAdresses();

                if (isNan(data_type, output_adresses->at( col )))
                    continue;

                multiplyData(output_adresses->at( col ), data_type, factor);
            }
        }
    }
    delete type_set;

    logdbg << "Util: finalizeDBData: done";
}
}
}


