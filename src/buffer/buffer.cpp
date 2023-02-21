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

#include "buffer.h"

#include "boost/date_time/posix_time/posix_time.hpp"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/variableset.h"
#include "logger.h"
#include "nullablevector.h"
#include "string.h"
#include "stringconv.h"
#include "unit.h"
#include "unitmanager.h"
#include "util/timeconv.h"

using namespace nlohmann;
using namespace std;

Buffer::Buffer(PropertyList properties, const string& dbcontent_name)
    : dbcontent_name_(dbcontent_name) //, last_one_(false)
{
    logdbg << "Buffer: constructor: start";

    for (unsigned int cnt = 0; cnt < properties.size(); cnt++)
        addProperty(properties.at(cnt));

    logdbg << "Buffer: constructor: end";
}

Buffer::~Buffer()
{
    logdbg << "Buffer: destructor: dbo " << dbcontent_name_;

    properties_.clear();

    getArrayListMap<bool>().clear();
    getArrayListMap<char>().clear();
    getArrayListMap<unsigned char>().clear();
    getArrayListMap<int>().clear();
    getArrayListMap<unsigned int>().clear();
    getArrayListMap<long int>().clear();
    getArrayListMap<unsigned long int>().clear();
    getArrayListMap<float>().clear();
    getArrayListMap<double>().clear();
    getArrayListMap<string>().clear();
    getArrayListMap<json>().clear();
    getArrayListMap<boost::posix_time::ptime>().clear();

    data_size_ = 0;

    logdbg << "Buffer: destructor: end";
}

bool Buffer::hasProperty(const Property& property)
{
    if (properties_.hasProperty(property.name()))
    {
        if (properties_.get(property.name()).dataType() != property.dataType())
            logwrn << "Buffer: hasProperty: property '" << property.name()
                   << " has same name but different data types (" << properties_.get(property.name()).dataTypeString()
                   << ", " << property.dataTypeString() << ")";

        return true;
    }

    return false;
}

void Buffer::addProperty(string id, PropertyDataType type)
{
    logdbg << "Buffer: addProperty:  id '" << id << "' type " << Property::asString(type);

    assert(!id.empty());

    if (properties_.hasProperty(id))
        throw runtime_error("Buffer: addProperty: property " + id + " already exists");

    Property property = Property(id, type);

    switch (type)
    {
        case PropertyDataType::BOOL:
            assert(getArrayListMap<bool>().count(id) == 0);
            getArrayListMap<bool>()[id] =
                shared_ptr<NullableVector<bool>>(new NullableVector<bool>(property, *this));
            break;
        case PropertyDataType::CHAR:
            assert(getArrayListMap<char>().count(id) == 0);
            getArrayListMap<char>()[id] =
                shared_ptr<NullableVector<char>>(new NullableVector<char>(property, *this));
            break;
        case PropertyDataType::UCHAR:
            assert(getArrayListMap<unsigned char>().count(id) == 0);
            getArrayListMap<unsigned char>()[id] = shared_ptr<NullableVector<unsigned char>>(
                new NullableVector<unsigned char>(property, *this));
            break;
        case PropertyDataType::INT:
            assert(getArrayListMap<int>().count(id) == 0);
            getArrayListMap<int>()[id] =
                shared_ptr<NullableVector<int>>(new NullableVector<int>(property, *this));
            break;
        case PropertyDataType::UINT:
            assert(getArrayListMap<unsigned int>().count(id) == 0);
            getArrayListMap<unsigned int>()[id] = shared_ptr<NullableVector<unsigned int>>(
                new NullableVector<unsigned int>(property, *this));
            break;
        case PropertyDataType::LONGINT:
            assert(getArrayListMap<long int>().count(id) == 0);
            getArrayListMap<long int>()[id] =
                shared_ptr<NullableVector<long>>(new NullableVector<long>(property, *this));
            break;
        case PropertyDataType::ULONGINT:
            assert(getArrayListMap<unsigned long int>().count(id) == 0);
            getArrayListMap<unsigned long int>()[id] =
                shared_ptr<NullableVector<unsigned long>>(
                    new NullableVector<unsigned long>(property, *this));
            break;
        case PropertyDataType::FLOAT:
            assert(getArrayListMap<float>().count(id) == 0);
            getArrayListMap<float>()[id] =
                shared_ptr<NullableVector<float>>(new NullableVector<float>(property, *this));
            break;
        case PropertyDataType::DOUBLE:
            assert(getArrayListMap<double>().count(id) == 0);
            getArrayListMap<double>()[id] = shared_ptr<NullableVector<double>>(
                new NullableVector<double>(property, *this));
            break;
        case PropertyDataType::STRING:
            assert(getArrayListMap<string>().count(id) == 0);
            getArrayListMap<string>()[id] = shared_ptr<NullableVector<string>>(
                new NullableVector<string>(property, *this));
            break;
        case PropertyDataType::JSON:
            assert(getArrayListMap<json>().count(id) == 0);
            getArrayListMap<json>()[id] = shared_ptr<NullableVector<json>>(
                new NullableVector<json>(property, *this));
            break;
    case PropertyDataType::TIMESTAMP:
        assert(getArrayListMap<boost::posix_time::ptime>().count(id) == 0);
        getArrayListMap<boost::posix_time::ptime>()[id] = shared_ptr<NullableVector<boost::posix_time::ptime>>(
            new NullableVector<boost::posix_time::ptime>(property, *this));
        break;
        default:
            logerr << "Buffer: addProperty: unknown property type " << Property::asString(type);
            throw runtime_error("Buffer: addProperty: unknown property type " +
                                     Property::asString(type));
    }

    properties_.addProperty(id, type);

    logdbg << "Buffer: addProperty: end";
}

void Buffer::addProperty(const Property& property)
{
    assert (!hasProperty(property));

    addProperty(property.name(), property.dataType());

    assert (hasProperty(property));
}

void Buffer::deleteProperty(const Property& property)
{
    switch (property.dataType())
    {
    case PropertyDataType::BOOL:
        assert (has<bool>(property.name()));
        remove<bool> (property.name());
        assert (!has<bool>(property.name()));
        break;
    case PropertyDataType::CHAR:
        assert (has<char>(property.name()));
        remove<char> (property.name());
        assert (!has<char>(property.name()));
        break;
    case PropertyDataType::UCHAR:
        assert (has<unsigned char>(property.name()));
        remove<unsigned char> (property.name());
        assert (!has<unsigned char>(property.name()));
        break;
    case PropertyDataType::INT:
        assert (has<int>(property.name()));
        remove<int> (property.name());
        assert (!has<int>(property.name()));
        break;
    case PropertyDataType::UINT:
        assert (has<unsigned int>(property.name()));
        remove<unsigned int> (property.name());
        assert (!has<unsigned int>(property.name()));
        break;
    case PropertyDataType::LONGINT:
        assert (has<long int>(property.name()));
        remove<long int> (property.name());
        assert (!has<long int>(property.name()));
        break;
    case PropertyDataType::ULONGINT:
        assert (has<unsigned long int>(property.name()));
        remove<unsigned long int> (property.name());
        assert (!has<unsigned long int>(property.name()));
        break;
    case PropertyDataType::FLOAT:
        assert (has<float>(property.name()));
        remove<float> (property.name());
        assert (!has<float>(property.name()));
        break;
    case PropertyDataType::DOUBLE:
        assert (has<double>(property.name()));
        remove<double> (property.name());
        assert (!has<double>(property.name()));
        break;
    case PropertyDataType::STRING:
        assert (has<string>(property.name()));
        remove<string> (property.name());
        assert (!has<string>(property.name()));
        break;
    case PropertyDataType::JSON:
        assert (has<json>(property.name()));
        remove<json> (property.name());
        assert (!has<json>(property.name()));
        break;
    case PropertyDataType::TIMESTAMP:
        assert (has<boost::posix_time::ptime>(property.name()));
        remove<boost::posix_time::ptime> (property.name());
        assert (!has<boost::posix_time::ptime>(property.name()));
        break;
    default:
        logerr << "Buffer: deleteProperty: unknown property type "
                   << Property::asString(property.dataType());
        throw runtime_error(
                    "Buffer: deleteProperty: unknown property type " +
                    Property::asString(property.dataType()));
    }
}

void Buffer::sortByProperty(const Property& property)
{
    logdbg << "Buffer: sortByProperty: name " << property.name();

    std::vector<std::size_t> perm;

    switch (property.dataType())
    {
    case PropertyDataType::BOOL:
        perm = get<bool> (property.name()).sortPermutation();
        break;
    case PropertyDataType::CHAR:
        perm = get<char> (property.name()).sortPermutation();
        break;
    case PropertyDataType::UCHAR:
        perm = get<unsigned char> (property.name()).sortPermutation();
        break;
    case PropertyDataType::INT:
        perm = get<int> (property.name()).sortPermutation();
        break;
    case PropertyDataType::UINT:
        perm = get<unsigned int> (property.name()).sortPermutation();
        break;
    case PropertyDataType::LONGINT:
        perm = get<long int> (property.name()).sortPermutation();
        break;
    case PropertyDataType::ULONGINT:
        perm = get<unsigned long int> (property.name()).sortPermutation();
        break;
    case PropertyDataType::FLOAT:
        perm = get<float> (property.name()).sortPermutation();
        break;
    case PropertyDataType::DOUBLE:
        perm = get<double> (property.name()).sortPermutation();
        break;
    case PropertyDataType::STRING:
        perm = get<string> (property.name()).sortPermutation();
        break;
    case PropertyDataType::JSON:
        perm = get<json> (property.name()).sortPermutation();
        break;
    case PropertyDataType::TIMESTAMP:
        perm = get<boost::posix_time::ptime> (property.name()).sortPermutation();
        break;
    default:
        logerr << "Buffer: sortByProperty: unknown property type "
                   << Property::asString(property.dataType());
        throw runtime_error(
                    "Buffer: sortByProperty: unknown property type " +
                    Property::asString(property.dataType()));
    }

    assert (perm.size() == data_size_);

    for (auto& prop_it : properties_.properties())
    {
        logdbg << "Buffer: sortByProperty: sorting name " << prop_it.name();

        switch (prop_it.dataType())
        {
        case PropertyDataType::BOOL:
            get<bool> (prop_it.name()).sortByPermutation(perm);
            break;
        case PropertyDataType::CHAR:
            get<char> (prop_it.name()).sortByPermutation(perm);
            break;
        case PropertyDataType::UCHAR:
            get<unsigned char> (prop_it.name()).sortByPermutation(perm);
            break;
        case PropertyDataType::INT:
            get<int> (prop_it.name()).sortByPermutation(perm);
            break;
        case PropertyDataType::UINT:
            get<unsigned int> (prop_it.name()).sortByPermutation(perm);
            break;
        case PropertyDataType::LONGINT:
            get<long int> (prop_it.name()).sortByPermutation(perm);
            break;
        case PropertyDataType::ULONGINT:
            get<unsigned long int> (prop_it.name()).sortByPermutation(perm);
            break;
        case PropertyDataType::FLOAT:
            get<float> (prop_it.name()).sortByPermutation(perm);
            break;
        case PropertyDataType::DOUBLE:
            get<double> (prop_it.name()).sortByPermutation(perm);
            break;
        case PropertyDataType::STRING:
            get<string> (prop_it.name()).sortByPermutation(perm);
            break;
        case PropertyDataType::JSON:
            get<json> (prop_it.name()).sortByPermutation(perm);
            break;
        case PropertyDataType::TIMESTAMP:
            get<boost::posix_time::ptime> (prop_it.name()).sortByPermutation(perm);
            break;
        default:
            logerr << "Buffer: sortByProperty: unknown property type "
                       << Property::asString(property.dataType());
            throw runtime_error(
                        "Buffer: sortByProperty: unknown property type " +
                        Property::asString(property.dataType()));
        }
    }

    logdbg << "Buffer: sortByProperty: name " << property.name() << " done";
}

void Buffer::seizeBuffer(Buffer& org_buffer)
{
    logdbg << "Buffer: seizeBuffer: start";

    logdbg << "Buffer: seizeBuffer: size " << size() << " other size " << org_buffer.size();

    seizeArrayListMap<bool>(org_buffer);
    seizeArrayListMap<char>(org_buffer);
    seizeArrayListMap<unsigned char>(org_buffer);
    seizeArrayListMap<int>(org_buffer);
    seizeArrayListMap<unsigned int>(org_buffer);
    seizeArrayListMap<long int>(org_buffer);
    seizeArrayListMap<unsigned long int>(org_buffer);
    seizeArrayListMap<float>(org_buffer);
    seizeArrayListMap<double>(org_buffer);
    seizeArrayListMap<string>(org_buffer);
    seizeArrayListMap<json>(org_buffer);
    seizeArrayListMap<boost::posix_time::ptime>(org_buffer);

    org_buffer.properties_.clear();

    data_size_ += org_buffer.data_size_;

    logdbg << "Buffer: seizeBuffer: end size " << size();
}

size_t Buffer::size() { return data_size_; }

void Buffer::cutToSize(size_t size)
{
    for (auto& it : getArrayListMap<bool>())
        it.second->cutToSize(size);
    for (auto& it : getArrayListMap<char>())
        it.second->cutToSize(size);
    for (auto& it : getArrayListMap<unsigned char>())
        it.second->cutToSize(size);
    for (auto& it : getArrayListMap<int>())
        it.second->cutToSize(size);
    for (auto& it : getArrayListMap<unsigned int>())
        it.second->cutToSize(size);
    for (auto& it : getArrayListMap<long int>())
        it.second->cutToSize(size);
    for (auto& it : getArrayListMap<unsigned long int>())
        it.second->cutToSize(size);
    for (auto& it : getArrayListMap<float>())
        it.second->cutToSize(size);
    for (auto& it : getArrayListMap<double>())
        it.second->cutToSize(size);
    for (auto& it : getArrayListMap<string>())
        it.second->cutToSize(size);
    for (auto& it : getArrayListMap<json>())
        it.second->cutToSize(size);
    for (auto& it : getArrayListMap<boost::posix_time::ptime>())
        it.second->cutToSize(size);

    data_size_ = size;
}

void Buffer::cutUpToIndex(size_t index) // everything up to index is removed
{
    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (index < data_size_);

        for (auto& it : getArrayListMap<bool>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<char>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<unsigned char>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<int>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<unsigned int>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<long int>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<unsigned long int>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<float>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<double>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<string>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<json>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<boost::posix_time::ptime>())
            assert (it.second->size() <= data_size_);

        loginf << "Buffer: cutUpToIndex: index " << index << " data_size_ " << data_size_;
    }

    for (auto& it : getArrayListMap<bool>())
        it.second->cutUpToIndex(index);
    for (auto& it : getArrayListMap<char>())
        it.second->cutUpToIndex(index);
    for (auto& it : getArrayListMap<unsigned char>())
        it.second->cutUpToIndex(index);
    for (auto& it : getArrayListMap<int>())
        it.second->cutUpToIndex(index);
    for (auto& it : getArrayListMap<unsigned int>())
        it.second->cutUpToIndex(index);
    for (auto& it : getArrayListMap<long int>())
        it.second->cutUpToIndex(index);
    for (auto& it : getArrayListMap<unsigned long int>())
        it.second->cutUpToIndex(index);
    for (auto& it : getArrayListMap<float>())
        it.second->cutUpToIndex(index);
    for (auto& it : getArrayListMap<double>())
        it.second->cutUpToIndex(index);
    for (auto& it : getArrayListMap<string>())
        it.second->cutUpToIndex(index);
    for (auto& it : getArrayListMap<json>())
        it.second->cutUpToIndex(index);
    for (auto& it : getArrayListMap<boost::posix_time::ptime>())
        it.second->cutUpToIndex(index);

    data_size_ -= index+1;

    if (BUFFER_PEDANTIC_CHECKING)
    {
        loginf << "Buffer: cutUpToIndex: after cut index " << index << " data_size_ " << data_size_;

        for (auto& it : getArrayListMap<bool>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<char>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<unsigned char>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<int>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<unsigned int>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<long int>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<unsigned long int>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<float>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<double>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<string>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<json>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<boost::posix_time::ptime>())
            assert (it.second->size() <= data_size_);

    }
}

void Buffer::removeIndexes(const std::vector<size_t>& indexes_to_remove)
{
    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (indexes_to_remove.size() <= data_size_);

        for (auto& it : getArrayListMap<bool>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<char>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<unsigned char>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<int>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<unsigned int>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<long int>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<unsigned long int>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<float>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<double>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<string>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<json>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<boost::posix_time::ptime>())
            assert (it.second->size() <= data_size_);

        loginf << "Buffer: removeIndexes: indexes " << indexes_to_remove.size() << " data_size_ " << data_size_;
    }

    if (indexes_to_remove.size() == data_size_)
    {
        for (auto& it : getArrayListMap<bool>())
            it.second->clearData();
        for (auto& it : getArrayListMap<char>())
            it.second->clearData();
        for (auto& it : getArrayListMap<unsigned char>())
            it.second->clearData();
        for (auto& it : getArrayListMap<int>())
            it.second->clearData();
        for (auto& it : getArrayListMap<unsigned int>())
            it.second->clearData();
        for (auto& it : getArrayListMap<long int>())
            it.second->clearData();
        for (auto& it : getArrayListMap<unsigned long int>())
            it.second->clearData();
        for (auto& it : getArrayListMap<float>())
            it.second->clearData();
        for (auto& it : getArrayListMap<double>())
            it.second->clearData();
        for (auto& it : getArrayListMap<string>())
            it.second->clearData();
        for (auto& it : getArrayListMap<json>())
            it.second->clearData();
        for (auto& it : getArrayListMap<boost::posix_time::ptime>())
            it.second->clearData();

    }
    else
    {
        for (auto& it : getArrayListMap<bool>())
            it.second->removeIndexes(indexes_to_remove);
        for (auto& it : getArrayListMap<char>())
            it.second->removeIndexes(indexes_to_remove);
        for (auto& it : getArrayListMap<unsigned char>())
            it.second->removeIndexes(indexes_to_remove);
        for (auto& it : getArrayListMap<int>())
            it.second->removeIndexes(indexes_to_remove);
        for (auto& it : getArrayListMap<unsigned int>())
            it.second->removeIndexes(indexes_to_remove);
        for (auto& it : getArrayListMap<long int>())
            it.second->removeIndexes(indexes_to_remove);
        for (auto& it : getArrayListMap<unsigned long int>())
            it.second->removeIndexes(indexes_to_remove);
        for (auto& it : getArrayListMap<float>())
            it.second->removeIndexes(indexes_to_remove);
        for (auto& it : getArrayListMap<double>())
            it.second->removeIndexes(indexes_to_remove);
        for (auto& it : getArrayListMap<string>())
            it.second->removeIndexes(indexes_to_remove);
        for (auto& it : getArrayListMap<json>())
            it.second->removeIndexes(indexes_to_remove);
        for (auto& it : getArrayListMap<boost::posix_time::ptime>())
            it.second->removeIndexes(indexes_to_remove);

    }

    data_size_ -= indexes_to_remove.size();

    if (BUFFER_PEDANTIC_CHECKING)
    {
        loginf << "Buffer: removeIndexes: after cut indexes " << indexes_to_remove.size() << " data_size_ " << data_size_;

        for (auto& it : getArrayListMap<bool>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<char>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<unsigned char>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<int>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<unsigned int>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<long int>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<unsigned long int>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<float>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<double>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<string>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<json>())
            assert (it.second->size() <= data_size_);
        for (auto& it : getArrayListMap<boost::posix_time::ptime>())
            assert (it.second->size() <= data_size_);

    }
}

const PropertyList& Buffer::properties() { return properties_; }

void Buffer::printProperties()
{
    for (const auto& prop_it : properties_.properties())
        loginf << "'" << prop_it.name() << "' " << prop_it.dataTypeString();
}

bool Buffer::isNull(const Property& property, unsigned int index)
{
    if (BUFFER_PEDANTIC_CHECKING)
        assert(index < data_size_);

    switch (property.dataType())
    {
        case PropertyDataType::BOOL:
            assert(getArrayListMap<bool>().count(property.name()));
            return getArrayListMap<bool>().at(property.name())->isNull(index);
        case PropertyDataType::CHAR:
            assert(getArrayListMap<char>().count(property.name()));
            return getArrayListMap<char>().at(property.name())->isNull(index);
        case PropertyDataType::UCHAR:
            assert(getArrayListMap<unsigned char>().count(property.name()));
            return getArrayListMap<unsigned char>().at(property.name())->isNull(index);
        case PropertyDataType::INT:
            assert(getArrayListMap<int>().count(property.name()));
            return getArrayListMap<int>().at(property.name())->isNull(index);
        case PropertyDataType::UINT:
            assert(getArrayListMap<unsigned int>().count(property.name()));
            return getArrayListMap<unsigned int>().at(property.name())->isNull(index);
        case PropertyDataType::LONGINT:
            assert(getArrayListMap<long int>().count(property.name()));
            return getArrayListMap<long int>().at(property.name())->isNull(index);
        case PropertyDataType::ULONGINT:
            assert(getArrayListMap<unsigned long int>().count(property.name()));
            return getArrayListMap<unsigned long int>().at(property.name())->isNull(index);
        case PropertyDataType::FLOAT:
            assert(getArrayListMap<float>().count(property.name()));
            return getArrayListMap<float>().at(property.name())->isNull(index);
        case PropertyDataType::DOUBLE:
            assert(getArrayListMap<double>().count(property.name()));
            return getArrayListMap<double>().at(property.name())->isNull(index);
        case PropertyDataType::STRING:
            assert(getArrayListMap<string>().count(property.name()));
            return getArrayListMap<string>().at(property.name())->isNull(index);
        case PropertyDataType::JSON:
            assert(getArrayListMap<json>().count(property.name()));
            return getArrayListMap<json>().at(property.name())->isNull(index);
        case PropertyDataType::TIMESTAMP:
            assert(getArrayListMap<boost::posix_time::ptime>().count(property.name()));
            return getArrayListMap<boost::posix_time::ptime>().at(property.name())->isNull(index);
        default:
            logerr << "Buffer: isNone: unknown property type "
                   << Property::asString(property.dataType());
            throw runtime_error("Buffer: isNone: unknown property type " +
                                     Property::asString(property.dataType()));
    }
}

void Buffer::transformVariables(dbContent::VariableSet& list, bool dbcol2dbovar)
{
    logdbg << "Buffer: transformVariables: dbo '" << dbcontent_name_ << "' dbcol2dbovar " << dbcol2dbovar;

    const vector<dbContent::Variable*>& variables = list.getSet();
    string variable_name;
    string db_column_name;

    string current_var_name;
    string transformed_var_name;

    for (auto var_it : variables)
    {
        logdbg << "Buffer: transformVariables: variable " << var_it->name() << " db column " << db_column_name;

        variable_name = var_it->name();
        db_column_name = var_it->dbColumnName();

        PropertyDataType data_type = var_it->dataType();

        if (dbcol2dbovar)
        {
            if (!properties_.hasProperty(db_column_name))
            {
                logerr << "Buffer: transformVariables: property '" << db_column_name << "' not found";
                continue;
            }

            assert(properties_.hasProperty(db_column_name));
            assert(properties_.get(db_column_name).dataType() == var_it->dataType());

            current_var_name = db_column_name;
            transformed_var_name = variable_name;
        }
        else
        {
            if (!properties_.hasProperty(var_it->name()))
            {
                logerr << "Buffer: transformVariables: variable '" << variable_name << "' not found";
                continue;
            }

            assert(properties_.hasProperty(variable_name));
            assert(properties_.get(variable_name).dataType() == var_it->dataType());

            current_var_name = variable_name;
            transformed_var_name = db_column_name;
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
                    rename<bool>(current_var_name, transformed_var_name);
                    break;
                }
                case PropertyDataType::CHAR:
                {
                    rename<char>(current_var_name, transformed_var_name);
                    break;
                }
                case PropertyDataType::UCHAR:
                {
                    rename<unsigned char>(current_var_name, transformed_var_name);
                    break;
                }
                case PropertyDataType::INT:
                {
                    rename<int>(current_var_name, transformed_var_name);
                    break;
                }
                case PropertyDataType::UINT:
                {
                    rename<unsigned int>(current_var_name, transformed_var_name);
                    break;
                }
                case PropertyDataType::LONGINT:
                {
                    rename<long int>(current_var_name, transformed_var_name);
                    break;
                }
                case PropertyDataType::ULONGINT:
                {
                    rename<unsigned long int>(current_var_name, transformed_var_name);
                    break;
                }
                case PropertyDataType::FLOAT:
                {
                    rename<float>(current_var_name, transformed_var_name);
                    break;
                }
                case PropertyDataType::DOUBLE:
                {
                    rename<double>(current_var_name, transformed_var_name);
                    break;
                }
                case PropertyDataType::STRING:
                {
                    rename<string>(current_var_name, transformed_var_name);
                    break;
                }
                case PropertyDataType::JSON:
                {
                    rename<json>(current_var_name, transformed_var_name);
                    break;
                }
                case PropertyDataType::TIMESTAMP:
                {
                    rename<boost::posix_time::ptime>(current_var_name, transformed_var_name);
                    break;
                }
                default:
                    logerr << "Buffer: transformVariables: unknown property type "
                           << Property::asString(data_type);
                    throw runtime_error("Buffer: transformVariables: unknown property type " +
                                             Property::asString(data_type));
            }
        }
    }
}

nlohmann::json Buffer::asJSON(unsigned int max_size)
{
    json j;

    if (max_size == 0)
        max_size = data_size_;

    for (unsigned int cnt=0; cnt < max_size; ++cnt)
    {
        j[cnt] = json::object();

        for (auto& it : getArrayListMap<bool>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = it.second->get(cnt);
        for (auto& it : getArrayListMap<char>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = it.second->get(cnt);
        for (auto& it : getArrayListMap<unsigned char>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = it.second->get(cnt);
        for (auto& it : getArrayListMap<int>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = it.second->get(cnt);
        for (auto& it : getArrayListMap<unsigned int>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = it.second->get(cnt);
        for (auto& it : getArrayListMap<long int>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = it.second->get(cnt);
        for (auto& it : getArrayListMap<unsigned long int>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = it.second->get(cnt);
        for (auto& it : getArrayListMap<float>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = it.second->get(cnt);
        for (auto& it : getArrayListMap<double>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = it.second->get(cnt);
        for (auto& it : getArrayListMap<string>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = it.second->get(cnt);
        for (auto& it : getArrayListMap<json>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = it.second->get(cnt);
        for (auto& it : getArrayListMap<boost::posix_time::ptime>())
            if (!it.second->isNull(cnt))
                j[cnt][it.second->propertyName()] = Utils::Time::toString(it.second->get(cnt));

    }

    return j;
}
