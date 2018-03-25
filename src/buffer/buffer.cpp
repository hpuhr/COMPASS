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
    //, first_write_(true),
    //search_active_(false), search_key_pos_(-1), search_key_min_ (-1), search_key_max_ (-1)
{
    logdbg  << "Buffer: constructor: start";

    id_ = ids_;
    ++ids_;

    for (unsigned int cnt=0; cnt < properties.size(); cnt++)
        addProperty(properties.at(cnt));

    //init();

    logdbg  << "Buffer: constructor: end";
}

/**
 * Calls clear.
 */
Buffer::~Buffer()
{
    logdbg  << "Buffer: destructor: start";

    properties_.clear();
    arrays_bool_.clear();
    arrays_char_.clear();
    arrays_uchar_.clear();
    arrays_int_.clear();
    arrays_uint_.clear();
    arrays_long_int_.clear();
    arrays_ulong_int_.clear();
    arrays_float_.clear();
    arrays_double_.clear();
    arrays_string_.clear();

    //    last_one_=false;

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
        assert (arrays_bool_.count(id) == 0);
        arrays_bool_ [id] = std::shared_ptr<ArrayListTemplate<bool>> (new ArrayListTemplate<bool>());
        break;
    case PropertyDataType::CHAR:
        assert (arrays_char_.count(id) == 0);
        arrays_char_ [id] = std::shared_ptr<ArrayListTemplate<char>> (new ArrayListTemplate<char>());
        break;
    case PropertyDataType::UCHAR:
        assert (arrays_uchar_.count(id) == 0);
        arrays_uchar_ [id] = std::shared_ptr<ArrayListTemplate<unsigned char>> (new ArrayListTemplate<unsigned char>());
        break;
    case PropertyDataType::INT:
        assert (arrays_int_.count(id) == 0);
        arrays_int_ [id] = std::shared_ptr<ArrayListTemplate<int>> (new ArrayListTemplate<int>());
        break;
    case PropertyDataType::UINT:
        assert (arrays_uint_.count(id) == 0);
        arrays_uint_ [id] = std::shared_ptr<ArrayListTemplate<unsigned int>> (new ArrayListTemplate<unsigned int>());
        break;
    case PropertyDataType::LONGINT:
        assert (arrays_long_int_.count(id) == 0);
        arrays_long_int_ [id] = std::shared_ptr<ArrayListTemplate<long>> (new ArrayListTemplate<long>());
        break;
    case PropertyDataType::ULONGINT:
        assert (arrays_ulong_int_.count(id) == 0);
        arrays_ulong_int_ [id] = std::shared_ptr<ArrayListTemplate<unsigned long>> (new ArrayListTemplate<unsigned long>());
        break;
    case PropertyDataType::FLOAT:
        assert (arrays_float_.count(id) == 0);
        arrays_float_ [id] = std::shared_ptr<ArrayListTemplate<float>> (new ArrayListTemplate<float>());
        break;
    case PropertyDataType::DOUBLE:
        assert (arrays_double_.count(id) == 0);
        arrays_double_ [id] = std::shared_ptr<ArrayListTemplate<double>> (new ArrayListTemplate<double>());
        break;
    case PropertyDataType::STRING:
        assert (arrays_string_.count(id) == 0);
        arrays_string_ [id] = std::shared_ptr<ArrayListTemplate<std::string>> (new ArrayListTemplate<std::string>());
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

bool Buffer::hasBool (const std::string &id)
{
    return arrays_bool_.count(id) != 0;
}

bool Buffer::hasChar (const std::string id)
{
    return arrays_char_.count(id) != 0;
}

bool Buffer::hasUChar (const std::string &id)
{
    return arrays_uchar_.count(id) != 0;
}

bool Buffer::hasInt (const std::string &id)
{
    return arrays_int_.count(id) != 0;
}

bool Buffer::hasUInt (const std::string &id)
{
    return arrays_uint_.count(id) != 0;
}

bool Buffer::hasLongInt (const std::string &id)
{
    return arrays_long_int_.count(id) != 0;
}

bool Buffer::hasULongInt (const std::string &id)
{
    return arrays_ulong_int_.count(id) != 0;
}

bool Buffer::hasFloat (const std::string &id)
{
    return arrays_float_.count(id) != 0;
}

bool Buffer::hasDouble (const std::string &id)
{
    return arrays_double_.count(id) != 0;
}

bool Buffer::hasString (const std::string &id)
{
    return arrays_string_.count(id) != 0;
}

ArrayListTemplate<bool> &Buffer::getBool (const std::string &id)
{
    if (arrays_bool_.count(id) == 0)
        logerr << "Buffer: getBool: unknown id " << id;

    return *arrays_bool_.at(id);
}

ArrayListTemplate<char> &Buffer::getChar (const std::string id)
{
    if (arrays_char_.count(id) == 0)
        logerr << "Buffer: getChar: unknown id " << id;

    return *arrays_char_.at(id);
}

ArrayListTemplate<unsigned char> &Buffer::getUChar (const std::string &id)
{
    if (arrays_uchar_.count(id) == 0)
        logerr << "Buffer: getUChar: unknown id " << id;

    return *arrays_uchar_.at(id);
}

ArrayListTemplate<int> &Buffer::getInt (const std::string &id)
{
    if (arrays_int_.count(id) == 0)
        logerr << "Buffer: getInt: unknown id " << id;

    return *arrays_int_.at(id);
}

ArrayListTemplate<unsigned int> &Buffer::getUInt (const std::string &id)
{
    if (arrays_uint_.count(id) == 0)
        logerr << "Buffer: getUInt: unknown id " << id;

    return *arrays_uint_.at(id);
}

ArrayListTemplate<long int> &Buffer::getLongInt (const std::string &id)
{
    if (arrays_long_int_.count(id) == 0)
        logerr << "Buffer: getLongInt: unknown id " << id;

    return *arrays_long_int_.at(id);
}

ArrayListTemplate<unsigned long int> &Buffer::getULongInt (const std::string &id)
{
     if (arrays_ulong_int_.count(id) == 0)
         logerr << "Buffer: getULongInt: unknown id " << id;

    return *arrays_ulong_int_.at(id);
}

ArrayListTemplate<float> &Buffer::getFloat (const std::string &id)
{
    //assert (arrays_float_.count(id) != 0);

    if (arrays_float_.count(id) == 0)
        logerr << "Buffer: getBool: unknown id " << id;

    return *arrays_float_.at(id);
}

ArrayListTemplate<double> &Buffer::getDouble (const std::string &id)
{
    if (arrays_double_.count(id) == 0)
        logerr << "Buffer: getDouble: unknown id " << id;

    return *arrays_double_.at(id);
}

ArrayListTemplate<std::string> &Buffer::getString (const std::string &id)
{
    if (arrays_string_.count(id) == 0)
        logerr << "Buffer: getBool: unknown id " << id;

    return *arrays_string_.at(id);
}

void Buffer::renameBool (const std::string &id, const std::string &id_new)
{
    logdbg << "Buffer: renameBool: current " << id << " new " << id_new;

    assert (arrays_bool_.count(id) == 1);
    assert (arrays_bool_.count(id_new) == 0);
    std::shared_ptr<ArrayListTemplate<bool>> array_list = arrays_bool_.at(id);
    arrays_bool_.erase(id);
    arrays_bool_[id_new]=array_list;

    assert (properties_.hasProperty(id));
    Property old_property = properties_.get(id);
    properties_.removeProperty(id);
    properties_.addProperty(id_new, old_property.dataType());
}

void Buffer::renameChar (const std::string &id, const std::string &id_new)
{
    logdbg << "Buffer: renameChar: current " << id << " new " << id_new;

    assert (arrays_char_.count(id) == 1);
    assert (arrays_char_.count(id_new) == 0);
    std::shared_ptr<ArrayListTemplate<char>> array_list = arrays_char_.at(id);
    arrays_char_.erase(id);
    arrays_char_[id_new]=array_list;

    assert (properties_.hasProperty(id));
    Property old_property = properties_.get(id);
    properties_.removeProperty(id);
    properties_.addProperty(id_new, old_property.dataType());
}

void Buffer::renameUChar (const std::string &id, const std::string &id_new)
{
    logdbg << "Buffer: renameUChar: current " << id << " new " << id_new;

    assert (arrays_uchar_.count(id) == 1);
    assert (arrays_uchar_.count(id_new) == 0);
    std::shared_ptr<ArrayListTemplate<unsigned char>> array_list = arrays_uchar_.at(id);
    arrays_uchar_.erase(id);
    arrays_uchar_[id_new]=array_list;

    assert (properties_.hasProperty(id));
    Property old_property = properties_.get(id);
    properties_.removeProperty(id);
    properties_.addProperty(id_new, old_property.dataType());
}

void Buffer::renameInt (const std::string &id, const std::string &id_new)
{
    logdbg << "Buffer: renameInt: current " << id << " new " << id_new;

    assert (arrays_int_.count(id) == 1);
    assert (arrays_int_.count(id_new) == 0);
    std::shared_ptr<ArrayListTemplate<int>> array_list = arrays_int_.at(id);
    arrays_int_.erase(id);
    arrays_int_[id_new]=array_list;

    assert (properties_.hasProperty(id));
    Property old_property = properties_.get(id);
    properties_.removeProperty(id);
    properties_.addProperty(id_new, old_property.dataType());
}

void Buffer::renameUInt (const std::string &id, const std::string &id_new)
{
    logdbg << "Buffer: renameUInt: current " << id << " new " << id_new;

    assert (arrays_uint_.count(id) == 1);
    assert (arrays_uint_.count(id_new) == 0);
    std::shared_ptr<ArrayListTemplate<unsigned int>> array_list = arrays_uint_.at(id);
    arrays_uint_.erase(id);
    arrays_uint_[id_new]=array_list;

    assert (properties_.hasProperty(id));
    Property old_property = properties_.get(id);
    properties_.removeProperty(id);
    properties_.addProperty(id_new, old_property.dataType());
}

void Buffer::renameLongInt (const std::string &id, const std::string &id_new)
{
    logdbg << "Buffer: renameLongInt: current " << id << " new " << id_new;

    assert (arrays_long_int_.count(id) == 1);
    assert (arrays_long_int_.count(id_new) == 0);
    std::shared_ptr<ArrayListTemplate<long int>> array_list = arrays_long_int_.at(id);
    arrays_long_int_.erase(id);
    arrays_long_int_[id_new]=array_list;

    assert (properties_.hasProperty(id));
    Property old_property = properties_.get(id);
    properties_.removeProperty(id);
    properties_.addProperty(id_new, old_property.dataType());
}

void Buffer::renameULongInt (const std::string &id, const std::string &id_new)
{
    logdbg << "Buffer: renameULongInt: current " << id << " new " << id_new;

    assert (arrays_ulong_int_.count(id) == 1);
    assert (arrays_ulong_int_.count(id_new) == 0);
    std::shared_ptr<ArrayListTemplate<unsigned long>> array_list = arrays_ulong_int_.at(id);
    arrays_ulong_int_.erase(id);
    arrays_ulong_int_[id_new]=array_list;

    assert (properties_.hasProperty(id));
    Property old_property = properties_.get(id);
    properties_.removeProperty(id);
    properties_.addProperty(id_new, old_property.dataType());
}

void Buffer::renameFloat (const std::string &id, const std::string &id_new)
{
    logdbg << "Buffer: renameFloat: current " << id << " new " << id_new;

    assert (arrays_float_.count(id) == 1);
    assert (arrays_float_.count(id_new) == 0);
    std::shared_ptr<ArrayListTemplate<float>> array_list = arrays_float_.at(id);
    arrays_float_.erase(id);
    arrays_float_[id_new]=array_list;

    assert (properties_.hasProperty(id));
    Property old_property = properties_.get(id);
    properties_.removeProperty(id);
    properties_.addProperty(id_new, old_property.dataType());
}

void Buffer::renameDouble (const std::string &id, const std::string &id_new)
{
    logdbg << "Buffer: renameDouble: current " << id << " new " << id_new;

    assert (arrays_double_.count(id) == 1);
    assert (arrays_double_.count(id_new) == 0);
    std::shared_ptr<ArrayListTemplate<double>> array_list = arrays_double_.at(id);
    arrays_double_.erase(id);
    arrays_double_[id_new]=array_list;

    assert (properties_.hasProperty(id));
    Property old_property = properties_.get(id);
    properties_.removeProperty(id);
    properties_.addProperty(id_new, old_property.dataType());
}

void Buffer::renameString (const std::string &id, const std::string &id_new)
{
    logdbg << "Buffer: renameString: current " << id << " new " << id_new;

    assert (arrays_string_.count(id) == 1);
    assert (arrays_string_.count(id_new) == 0);
    std::shared_ptr<ArrayListTemplate<std::string>> array_list = arrays_string_.at(id);
    arrays_string_.erase(id);
    arrays_string_[id_new]=array_list;

    assert (properties_.hasProperty(id));
    Property old_property = properties_.get(id);
    properties_.removeProperty(id);
    properties_.addProperty(id_new, old_property.dataType());
}

void Buffer::seizeBuffer (Buffer &org_buffer)
{
    logdbg  << "Buffer: seizeBuffer: start";

    logdbg  << "Buffer: seizeBuffer: full " << full() << " size " << size() << " first write " << firstWrite();

    assert (full() || firstWrite()); //|| first_write_

    //logdbg  << "Buffer: seizeBuffer: containers";
    //std::vector <PropertyContainer *> org_containers = org_buffer->containers_;

    assert (arrays_bool_.size() == org_buffer.arrays_bool_.size());
    assert (arrays_char_.size() == org_buffer.arrays_char_.size());
    assert (arrays_uchar_.size() == org_buffer.arrays_uchar_.size());
    assert (arrays_int_.size() == org_buffer.arrays_int_.size());
    assert (arrays_uint_.size() == org_buffer.arrays_uint_.size());
    assert (arrays_long_int_.size() == org_buffer.arrays_long_int_.size());
    assert (arrays_ulong_int_.size() == org_buffer.arrays_ulong_int_.size());
    assert (arrays_float_.size() == org_buffer.arrays_float_.size());
    assert (arrays_double_.size() == org_buffer.arrays_double_.size());
    assert (arrays_string_.size() == org_buffer.arrays_string_.size());

    logdbg  << "Buffer: seizeBuffer: inserting ";
    org_buffer.properties_.clear();

    for (auto it : arrays_bool_)
        it.second->addData(*org_buffer.arrays_bool_.at(it.first));
    org_buffer.arrays_bool_.clear();
    for (auto it : arrays_char_)
        it.second->addData(*org_buffer.arrays_char_.at(it.first));
    org_buffer.arrays_char_.clear();
    for (auto it : arrays_uchar_)
        it.second->addData(*org_buffer.arrays_uchar_.at(it.first));
    org_buffer.arrays_uchar_.clear();
    for (auto it : arrays_int_)
        it.second->addData(*org_buffer.arrays_int_.at(it.first));
    org_buffer.arrays_int_.clear();
    for (auto it : arrays_uint_)
        it.second->addData(*org_buffer.arrays_uint_.at(it.first));
    org_buffer.arrays_uint_.clear();
    for (auto it : arrays_long_int_)
        it.second->addData(*org_buffer.arrays_long_int_.at(it.first));
    org_buffer.arrays_long_int_.clear();
    for (auto it : arrays_ulong_int_)
        it.second->addData(*org_buffer.arrays_ulong_int_.at(it.first));
    org_buffer.arrays_ulong_int_.clear();
    for (auto it : arrays_float_)
        it.second->addData(*org_buffer.arrays_float_.at(it.first));
    org_buffer.arrays_float_.clear();
    for (auto it : arrays_double_)
        it.second->addData(*org_buffer.arrays_double_.at(it.first));
    org_buffer.arrays_double_.clear();
    for (auto it : arrays_string_)
        it.second->addData(*org_buffer.arrays_string_.at(it.first));
    org_buffer.arrays_string_.clear();

    if (org_buffer.lastOne())
        last_one_ = true;

    logdbg  << "Buffer: seizeBuffer: end size " << size();
}

bool Buffer::full ()
{
    return size()%BUFFER_ARRAY_SIZE == 0;
}

const size_t Buffer::size ()
{
    size_t size = 0;

    for (auto it : arrays_bool_)
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : arrays_char_)
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : arrays_uchar_)
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : arrays_int_)
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : arrays_uint_)
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : arrays_long_int_)
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : arrays_ulong_int_)
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : arrays_float_)
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : arrays_double_)
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : arrays_string_)
        if (it.second->size() > size)
            size = it.second->size();

    //loginf << "Buffer: size: " << size;
    return size;
}

bool Buffer::firstWrite ()
{
    std::vector <ArrayListBase *>::const_iterator it;

    for (auto it : arrays_bool_)
        if (it.second->size() > 0)
            return false;
    for (auto it : arrays_char_)
        if (it.second->size() > 0)
            return false;
    for (auto it : arrays_uchar_)
        if (it.second->size() > 0)
            return false;
    for (auto it : arrays_int_)
        if (it.second->size() > 0)
            return false;
    for (auto it : arrays_uint_)
        if (it.second->size() > 0)
            return false;
    for (auto it : arrays_long_int_)
        if (it.second->size() > 0)
            return false;
    for (auto it : arrays_ulong_int_)
        if (it.second->size() > 0)
            return false;
    for (auto it : arrays_float_)
        if (it.second->size() > 0)
            return false;
    for (auto it : arrays_double_)
        if (it.second->size() > 0)
            return false;
    for (auto it : arrays_string_)
        if (it.second->size() > 0)
            return false;

    return true;
}

bool Buffer::isNone (const Property& property, unsigned int row_cnt)
{
    switch (property.dataType())
    {
    case PropertyDataType::BOOL:
        assert (arrays_bool_.count(property.name()));
        return arrays_bool_.at(property.name())->isNone(row_cnt);
    case PropertyDataType::CHAR:
        assert (arrays_char_.count(property.name()));
        return arrays_char_.at(property.name())->isNone(row_cnt);
    case PropertyDataType::UCHAR:
        assert (arrays_uchar_.count(property.name()));
        return arrays_uchar_.at(property.name())->isNone(row_cnt);
    case PropertyDataType::INT:
        assert (arrays_int_.count(property.name()));
        return arrays_int_.at(property.name())->isNone(row_cnt);
    case PropertyDataType::UINT:
        assert (arrays_uint_.count(property.name()));
        return arrays_uint_.at(property.name())->isNone(row_cnt);
    case PropertyDataType::LONGINT:
        assert (arrays_long_int_.count(property.name()));
        return arrays_long_int_.at(property.name())->isNone(row_cnt);
    case PropertyDataType::ULONGINT:
        assert (arrays_ulong_int_.count(property.name()));
        return arrays_ulong_int_.at(property.name())->isNone(row_cnt);
    case PropertyDataType::FLOAT:
        assert (arrays_float_.count(property.name()));
        return arrays_float_.at(property.name())->isNone(row_cnt);
    case PropertyDataType::DOUBLE:
        assert (arrays_double_.count(property.name()));
        return arrays_double_.at(property.name())->isNone(row_cnt);
    case PropertyDataType::STRING:
        assert (arrays_string_.count(property.name()));
        return arrays_string_.at(property.name())->isNone(row_cnt);
    default:
        logerr  <<  "Buffer: isNone: unknown property type " << Property::asString(property.dataType());
        throw std::runtime_error ("Buffer: isNone: unknown property type "+Property::asString(property.dataType()));
    }

}

void Buffer::transformVariables (DBOVariableSet& list, bool tc2dbovar)
{
    std::vector <DBOVariable*> &variables = list.getSet ();

    for (auto var_it : variables)
    {
        logdbg << "Buffer: transformVariables: variable " << var_it->name() << " has representation "
               << var_it->representationString();
        assert (var_it->hasCurrentDBColumn());
        const DBTableColumn &column = var_it->currentDBColumn ();

        PropertyDataType data_type = var_it->dataType();

        std::string current_var_name;
        std::string transformed_var_name;

        if (tc2dbovar)
        {
            assert (properties_.hasProperty(column.name()));
            assert (properties_.get(column.name()).dataType() == var_it->dataType());
            current_var_name = column.name();
            transformed_var_name = var_it->name();
        }
        else
        {
            assert (properties_.hasProperty(var_it->name()));
            assert (properties_.get(var_it->name()).dataType() == column.propertyType());
            current_var_name = var_it->name();
            transformed_var_name = column.name();
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
                assert (hasBool(current_var_name));
                ArrayListTemplate<bool> &array_list = getBool(current_var_name);
                logwrn << "Buffer: transformVariables: double multiplication of boolean variable "
                       << var_it->name();
                array_list *= factor;
                break;
            }
            case PropertyDataType::CHAR:
            {
                assert (hasChar(current_var_name));
                ArrayListTemplate<char> &array_list = getChar (current_var_name);
                logwrn << "Buffer: transformVariables: double multiplication of char variable "
                       << var_it->name();
                array_list *= factor;
                break;
            }
            case PropertyDataType::UCHAR:
            {
                assert (hasUChar(current_var_name));
                ArrayListTemplate<unsigned char> &array_list = getUChar (current_var_name);
                logwrn << "Buffer: transformVariables: double multiplication of unsigned char variable "
                       << var_it->name();
                array_list *= factor;
                break;
            }
            case PropertyDataType::INT:
            {
                assert (hasInt(current_var_name));
                ArrayListTemplate<int> &array_list = getInt (current_var_name);
                array_list *= factor;
                break;
            }
            case PropertyDataType::UINT:
            {
                assert (hasUInt(current_var_name));
                ArrayListTemplate<unsigned int> &array_list = getUInt (current_var_name);
                array_list *= factor;
                break;
            }
            case PropertyDataType::LONGINT:
            {
                assert (hasLongInt(current_var_name));
                ArrayListTemplate<long> &array_list = getLongInt(current_var_name);
                array_list *= factor;
                break;
            }
            case PropertyDataType::ULONGINT:
            {
                assert (hasULongInt(current_var_name));
                ArrayListTemplate<unsigned long> &array_list = getULongInt(current_var_name);
                array_list *= factor;
                break;
            }
            case PropertyDataType::FLOAT:
            {
                assert (hasFloat(current_var_name));
                ArrayListTemplate<float> &array_list = getFloat(current_var_name);
                array_list *= factor;
                break;
            }
            case PropertyDataType::DOUBLE:
            {
                assert (hasDouble(current_var_name));
                ArrayListTemplate<double> &array_list = getDouble(current_var_name);
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
            logdbg << "Buffer: transformVariables: renaming variable " << current_var_name
                   << " to variable name " << transformed_var_name;

            switch (data_type)
            {
            case PropertyDataType::BOOL:
            {
                renameBool (current_var_name, transformed_var_name);
                break;
            }
            case PropertyDataType::CHAR:
            {
                renameChar (current_var_name, transformed_var_name);
                break;
            }
            case PropertyDataType::UCHAR:
            {
                renameUChar (current_var_name, transformed_var_name);
                break;
            }
            case PropertyDataType::INT:
            {
                renameInt (current_var_name, transformed_var_name);
                break;
            }
            case PropertyDataType::UINT:
            {
                renameUInt (current_var_name, transformed_var_name);
                break;
            }
            case PropertyDataType::LONGINT:
            {
                renameLongInt (current_var_name, transformed_var_name);
                break;
            }
            case PropertyDataType::ULONGINT:
            {
                renameULongInt (current_var_name, transformed_var_name);
                break;
            }
            case PropertyDataType::FLOAT:
            {
                renameFloat (current_var_name, transformed_var_name);
                break;
            }
            case PropertyDataType::DOUBLE:
            {
                renameDouble (current_var_name, transformed_var_name);
                break;
            }
            case PropertyDataType::STRING:
            {
                renameString (current_var_name, transformed_var_name);
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




