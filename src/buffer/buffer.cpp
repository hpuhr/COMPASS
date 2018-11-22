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
#include "logger.h"
#include "string.h"

#include "dbovariableset.h"
#include "dbovariable.h"
#include "dbtablecolumn.h"
#include "stringconv.h"
#include "unitmanager.h"
#include "unit.h"

unsigned int Buffer::ids_ = 0;

/**
 * Creates an empty buffer withput an DBO type
 *
 */
Buffer::Buffer()
{
    logdbg  << "Buffer: constructor: start";

    id_ = ids_;
    ++ids_;

    logdbg  << "Buffer: constructor: end";
}


/**
 * Creates a buffer from a PropertyList and a DBO type. Sets member to initial values.
 *
 * \param member_list PropertyList defining all properties
 * \param type DBO type
 */
Buffer::Buffer(PropertyList properties, const std::string &dbo_name)
    : dbo_name_(dbo_name), last_one_(false)
    //search_active_(false), search_key_pos_(-1), search_key_min_ (-1), search_key_max_ (-1)
{
    logdbg  << "Buffer: constructor: start";

    id_ = ids_;
    ++ids_;

    for (unsigned int cnt=0; cnt < properties.size(); cnt++)
        addProperty(properties.at(cnt));

    logdbg  << "Buffer: constructor: end";
}

/**
 * Calls clear.
 */
Buffer::~Buffer()
{
    logdbg  << "Buffer: destructor: start";

    properties_.clear();

    logdbg  << "Buffer: destructor: end";
}

/**
 * \param id Unique property identifier
 * \param type Property data type
 *
 * \exception std::runtime_error if property id already in use
 */
void Buffer::addProperty (std::string id, PropertyDataType type)
{
    logdbg  << "Buffer: addProperty:  id '" << id << "' type " << Property::asString(type);

    assert (!id.empty());

    if (properties_.hasProperty(id))
        throw std::runtime_error ("Buffer: addProperty: property "+id+" already exists");

    switch (type)
    {
    case PropertyDataType::BOOL:
        assert (getArrayListMap<bool>().count(id) == 0);
        getArrayListMap<bool>()[id] =
                std::shared_ptr<ArrayListTemplate<bool>> (new ArrayListTemplate<bool>());
        break;
    case PropertyDataType::CHAR:
        assert (getArrayListMap<char>().count(id) == 0);
        getArrayListMap<char>() [id] =
                std::shared_ptr<ArrayListTemplate<char>> (new ArrayListTemplate<char>());
        break;
    case PropertyDataType::UCHAR:
        assert (getArrayListMap<unsigned char>().count(id) == 0);
        getArrayListMap<unsigned char>() [id] =
                std::shared_ptr<ArrayListTemplate<unsigned char>> (new ArrayListTemplate<unsigned char>());
        break;
    case PropertyDataType::INT:
        assert (getArrayListMap<int>().count(id) == 0);
        getArrayListMap<int>() [id] =
                std::shared_ptr<ArrayListTemplate<int>> (new ArrayListTemplate<int>());
        break;
    case PropertyDataType::UINT:
        assert (getArrayListMap<unsigned int>().count(id) == 0);
        getArrayListMap<unsigned int>() [id] =
                std::shared_ptr<ArrayListTemplate<unsigned int>> (new ArrayListTemplate<unsigned int>());
        break;
    case PropertyDataType::LONGINT:
        assert (getArrayListMap<long int>().count(id) == 0);
        getArrayListMap<long int>() [id] =
                std::shared_ptr<ArrayListTemplate<long>> (new ArrayListTemplate<long>());
        break;
    case PropertyDataType::ULONGINT:
        assert (getArrayListMap<unsigned long int>().count(id) == 0);
        getArrayListMap<unsigned long int>() [id] =
                std::shared_ptr<ArrayListTemplate<unsigned long>> (new ArrayListTemplate<unsigned long>());
        break;
    case PropertyDataType::FLOAT:
        assert (getArrayListMap<float>().count(id) == 0);
        getArrayListMap<float>() [id] =
                std::shared_ptr<ArrayListTemplate<float>> (new ArrayListTemplate<float>());
        break;
    case PropertyDataType::DOUBLE:
        assert (getArrayListMap<double>().count(id) == 0);
        getArrayListMap<double>() [id] =
                std::shared_ptr<ArrayListTemplate<double>> (new ArrayListTemplate<double>());
        break;
    case PropertyDataType::STRING:
        assert (getArrayListMap<std::string>().count(id) == 0);
        getArrayListMap<std::string>() [id] =
                std::shared_ptr<ArrayListTemplate<std::string>> (new ArrayListTemplate<std::string>());
        break;
    default:
        logerr  <<  "Buffer: addProperty: unknown property type " << Property::asString(type);
        throw std::runtime_error ("Buffer: addProperty: unknown property type "+Property::asString(type));
    }

    properties_.addProperty(id,type);

    logdbg  << "Buffer: addProperty: end";
}

void Buffer::addProperty (const Property &property)
{
    addProperty (property.name(), property.dataType());
}

void Buffer::seizeBuffer (Buffer &org_buffer)
{
    logdbg  << "Buffer: seizeBuffer: start";

    logdbg  << "Buffer: seizeBuffer: size " << size() << " other size " << org_buffer.size();

    //assert (full() || firstWrite()); //|| first_write_

    org_buffer.properties_.clear();

    seizeArrayListMap<bool>(org_buffer);
    seizeArrayListMap<char>(org_buffer);
    seizeArrayListMap<unsigned char>(org_buffer);
    seizeArrayListMap<int>(org_buffer);
    seizeArrayListMap<unsigned int>(org_buffer);
    seizeArrayListMap<long int>(org_buffer);
    seizeArrayListMap<unsigned long int>(org_buffer);
    seizeArrayListMap<float>(org_buffer);
    seizeArrayListMap<double>(org_buffer);
    seizeArrayListMap<std::string>(org_buffer);

    if (org_buffer.lastOne())
        last_one_ = true;

    logdbg  << "Buffer: seizeBuffer: end size " << size();
}

//bool Buffer::full ()
//{
//    return size()%BUFFER_ARRAY_SIZE == 0;
//}

const size_t Buffer::size ()
{
    size_t size = 0;

    for (auto it : getArrayListMap<bool>())
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : getArrayListMap<char>())
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : getArrayListMap<unsigned char>())
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : getArrayListMap<int>())
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : getArrayListMap<unsigned int>())
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : getArrayListMap<long int>())
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : getArrayListMap<unsigned long int>())
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : getArrayListMap<float>())
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : getArrayListMap<double>())
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : getArrayListMap<std::string>())
        if (it.second->size() > size)
            size = it.second->size();

    //loginf << "Buffer: size: " << size;
    return size;
}

bool Buffer::firstWrite ()
{
    std::vector <ArrayListBase *>::const_iterator it;

    for (auto it : getArrayListMap<bool>())
        if (it.second->size() > 0)
            return false;
    for (auto it : getArrayListMap<char>())
        if (it.second->size() > 0)
            return false;
    for (auto it : getArrayListMap<unsigned char>())
        if (it.second->size() > 0)
            return false;
    for (auto it : getArrayListMap<int>())
        if (it.second->size() > 0)
            return false;
    for (auto it : getArrayListMap<unsigned int>())
        if (it.second->size() > 0)
            return false;
    for (auto it : getArrayListMap<long int>())
        if (it.second->size() > 0)
            return false;
    for (auto it : getArrayListMap<unsigned long int>())
        if (it.second->size() > 0)
            return false;
    for (auto it : getArrayListMap<float>())
        if (it.second->size() > 0)
            return false;
    for (auto it : getArrayListMap<double>())
        if (it.second->size() > 0)
            return false;
    for (auto it : getArrayListMap<std::string>())
        if (it.second->size() > 0)
            return false;

    return true;
}

bool Buffer::isNone (const Property& property, unsigned int row_cnt)
{
    switch (property.dataType())
    {
    case PropertyDataType::BOOL:
        assert (getArrayListMap<bool>().count(property.name()));
        return getArrayListMap<bool>().at(property.name())->isNone(row_cnt);
    case PropertyDataType::CHAR:
        assert (getArrayListMap<char>().count(property.name()));
        return getArrayListMap<char>().at(property.name())->isNone(row_cnt);
    case PropertyDataType::UCHAR:
        assert (getArrayListMap<unsigned char>().count(property.name()));
        return getArrayListMap<unsigned char>().at(property.name())->isNone(row_cnt);
    case PropertyDataType::INT:
        assert (getArrayListMap<int>().count(property.name()));
        return getArrayListMap<int>().at(property.name())->isNone(row_cnt);
    case PropertyDataType::UINT:
        assert (getArrayListMap<unsigned int>().count(property.name()));
        return getArrayListMap<unsigned int>().at(property.name())->isNone(row_cnt);
    case PropertyDataType::LONGINT:
        assert (getArrayListMap<long int>().count(property.name()));
        return getArrayListMap<long int>().at(property.name())->isNone(row_cnt);
    case PropertyDataType::ULONGINT:
        assert (getArrayListMap<unsigned long int>().count(property.name()));
        return getArrayListMap<unsigned long int>().at(property.name())->isNone(row_cnt);
    case PropertyDataType::FLOAT:
        assert (getArrayListMap<float>().count(property.name()));
        return getArrayListMap<float>().at(property.name())->isNone(row_cnt);
    case PropertyDataType::DOUBLE:
        assert (getArrayListMap<double>().count(property.name()));
        return getArrayListMap<double>().at(property.name())->isNone(row_cnt);
    case PropertyDataType::STRING:
        assert (getArrayListMap<std::string>().count(property.name()));
        return getArrayListMap<std::string>().at(property.name())->isNone(row_cnt);
    default:
        logerr  <<  "Buffer: isNone: unknown property type " << Property::asString(property.dataType());
        throw std::runtime_error ("Buffer: isNone: unknown property type "+Property::asString(property.dataType()));
    }

}

void Buffer::transformVariables (DBOVariableSet& list, bool tc2dbovar)
{
    // TODO add proper data type conversion
    std::vector <DBOVariable*> &variables = list.getSet ();

    for (auto var_it : variables)
    {
        logdbg << "Buffer: transformVariables: variable " << var_it->name() << " has representation "
               << var_it->representationString();
        assert (var_it->hasCurrentDBColumn());
        const DBTableColumn &column = var_it->currentDBColumn ();

        logdbg << "Buffer: transformVariables: variable " << var_it->name() << " col "
               << column.name();

        PropertyDataType data_type = var_it->dataType();

        std::string current_var_name;
        std::string transformed_var_name;

        if (tc2dbovar)
        {
            assert (properties_.hasProperty(column.name()));
            // TODO HACK should be column data type
            assert (properties_.get(column.name()).dataType() == var_it->dataType());
            current_var_name = column.name();
            transformed_var_name = var_it->name();
        }
        else
        {
            assert (properties_.hasProperty(var_it->name()));
            logdbg << "Buffer: transformVariables: var " << var_it->name()
                   << " col " << column.name()
                   << " prop dt " << Property::asString(properties_.get(var_it->name()).dataType())
                   << " col dt " << Property::asString(column.propertyType());
            // TODO HACK
            //assert (properties_.get(var_it->name()).dataType() == column.propertyType());
            current_var_name = var_it->name();
            transformed_var_name = column.name();
        }

        if (column.dataFormat() != "") // do format conversion stuff
        {
            logdbg << "Buffer: transformVariables: column " << column.name()
                   << " has to-be-removed format " << column.dataFormat();

            switch (data_type)
            {
            case PropertyDataType::CHAR:
            {
                assert (has<char>(current_var_name));
                ArrayListTemplate<char> &array_list = get<char> (current_var_name);
                array_list.convertToStandardFormat(column.dataFormat());
                break;
            }
            case PropertyDataType::UCHAR:
            {
                assert (has<unsigned char>(current_var_name));
                ArrayListTemplate<unsigned char> &array_list = get<unsigned char> (current_var_name);
                array_list.convertToStandardFormat(column.dataFormat());
                break;
            }
            case PropertyDataType::INT:
            {
                assert (has<int>(current_var_name));
                ArrayListTemplate<int> &array_list = get<int> (current_var_name);
                array_list.convertToStandardFormat(column.dataFormat());
                break;
            }
            case PropertyDataType::UINT:
            {
                assert (has<unsigned int>(current_var_name));
                ArrayListTemplate<unsigned int> &array_list = get<unsigned int> (current_var_name);
                array_list.convertToStandardFormat(column.dataFormat());
                break;
            }
            case PropertyDataType::LONGINT:
            {
                assert (has<long int>(current_var_name));
                ArrayListTemplate<long int> &array_list = get<long int>(current_var_name);
                array_list.convertToStandardFormat(column.dataFormat());
                break;
            }
            case PropertyDataType::ULONGINT:
            {
                assert (has<unsigned long>(current_var_name));
                ArrayListTemplate<unsigned long> &array_list = get<unsigned long>(current_var_name);
                array_list.convertToStandardFormat(column.dataFormat());
                break;
            }
            default:
                logerr  <<  "Buffer: transformVariables: format conversion impossible for property type "
                         << Property::asString(data_type);
                throw std::runtime_error ("Buffer: transformVariables: impossible property type "
                                          + Property::asString(data_type));
            }
        }

        if (column.dimension() != var_it->dimension())
            logwrn << "Buffer: transformVariables:: variable " << var_it->name()
                   << " has differing dimensions " << column.dimension() << " " << var_it->dimension();
        else if (column.unit() != var_it->unit()) // do unit conversion stuff
        {
            logdbg << "Buffer: transformVariables: variable " << var_it->name()
                   << " of same dimension has different units " << column.unit() << " " << var_it->unit();

            const Dimension &dimension = UnitManager::instance().dimension (var_it->dimension());
            double factor;

            if (tc2dbovar)
                factor = dimension.getFactor (column.unit(), var_it->unit());
            else
                factor = dimension.getFactor (var_it->unit(), column.unit());

            logdbg  << "Buffer: transformVariables: correct unit transformation with factor " << factor;

            switch (data_type)
            {
            case PropertyDataType::BOOL:
            {
                assert (has<bool>(current_var_name));
                ArrayListTemplate<bool> &array_list = get<bool>(current_var_name);
                logwrn << "Buffer: transformVariables: double multiplication of boolean variable "
                       << var_it->name();
                array_list *= factor;
                break;
            }
            case PropertyDataType::CHAR:
            {
                assert (has<char>(current_var_name));
                ArrayListTemplate<char> &array_list = get<char> (current_var_name);
                logwrn << "Buffer: transformVariables: double multiplication of char variable "
                       << var_it->name();
                array_list *= factor;
                break;
            }
            case PropertyDataType::UCHAR:
            {
                assert (has<unsigned char>(current_var_name));
                ArrayListTemplate<unsigned char> &array_list = get<unsigned char> (current_var_name);
                logwrn << "Buffer: transformVariables: double multiplication of unsigned char variable "
                       << var_it->name();
                array_list *= factor;
                break;
            }
            case PropertyDataType::INT:
            {
                assert (has<int>(current_var_name));
                ArrayListTemplate<int> &array_list = get<int> (current_var_name);
                array_list *= factor;
                break;
            }
            case PropertyDataType::UINT:
            {
                assert (has<unsigned int>(current_var_name));
                ArrayListTemplate<unsigned int> &array_list = get<unsigned int> (current_var_name);
                array_list *= factor;
                break;
            }
            case PropertyDataType::LONGINT:
            {
                assert (has<long int>(current_var_name));
                ArrayListTemplate<long int> &array_list = get<long int>(current_var_name);
                array_list *= factor;
                break;
            }
            case PropertyDataType::ULONGINT:
            {
                assert (has<unsigned long>(current_var_name));
                ArrayListTemplate<unsigned long> &array_list = get<unsigned long>(current_var_name);
                array_list *= factor;
                break;
            }
            case PropertyDataType::FLOAT:
            {
                assert (has<float>(current_var_name));
                ArrayListTemplate<float> &array_list = get<float>(current_var_name);
                array_list *= factor;
                break;
            }
            case PropertyDataType::DOUBLE:
            {
                assert (has<double>(current_var_name));
                ArrayListTemplate<double> &array_list = get<double>(current_var_name);
                array_list *= factor;
                break;
            }
            case PropertyDataType::STRING:
                logerr << "Buffer: transformVariables: unit transformation for string variable "
                       << var_it->name() << " impossible";
                break;
            default:
                logerr  <<  "Buffer: transformVariables: unknown property type "
                         << Property::asString(data_type);
                throw std::runtime_error ("Buffer: transformVariables: unknown property type "
                                          + Property::asString(data_type));
            }
        }

        // rename to reflect dbo variable
        if (current_var_name != transformed_var_name)
        {
            loginf << "Buffer: transformVariables: renaming variable " << current_var_name
                   << " to variable name " << transformed_var_name;

            switch (data_type)
            {
            case PropertyDataType::BOOL:
            {
                rename<bool> (current_var_name, transformed_var_name);
                break;
            }
            case PropertyDataType::CHAR:
            {
                rename<char> (current_var_name, transformed_var_name);
                break;
            }
            case PropertyDataType::UCHAR:
            {
                rename<unsigned char> (current_var_name, transformed_var_name);
                break;
            }
            case PropertyDataType::INT:
            {
                rename<int> (current_var_name, transformed_var_name);
                break;
            }
            case PropertyDataType::UINT:
            {
                rename<unsigned int> (current_var_name, transformed_var_name);
                break;
            }
            case PropertyDataType::LONGINT:
            {
                rename<long int> (current_var_name, transformed_var_name);
                break;
            }
            case PropertyDataType::ULONGINT:
            {
                rename<unsigned long int> (current_var_name, transformed_var_name);
                break;
            }
            case PropertyDataType::FLOAT:
            {
                rename<float> (current_var_name, transformed_var_name);
                break;
            }
            case PropertyDataType::DOUBLE:
            {
                rename<double> (current_var_name, transformed_var_name);
                break;
            }
            case PropertyDataType::STRING:
            {
                rename<std::string> (current_var_name, transformed_var_name);
                break;
            }
            default:
                logerr  <<  "Buffer: transformVariables: unknown property type "
                         << Property::asString(data_type);
                throw std::runtime_error ("Buffer: transformVariables: unknown property type "
                                          + Property::asString(data_type));
            }
        }
    }
}




