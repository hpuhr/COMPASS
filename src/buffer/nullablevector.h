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

#ifndef ARRAYLIST_H_
#define ARRAYLIST_H_

#include <tbb/tbb.h>

#include <QDateTime>
#include <array>
#include <bitset>
#include <iomanip>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <vector>

#include "buffer.h"
#include "property.h"
#include "stringconv.h"

//#include "boost/lexical_cast.hpp"

const bool BUFFER_PEDANTIC_CHECKING = true;

/**
 * @brief Template List of fixed-size arrays to be used in Buffer classes.
 *
 * Was written for easy management of arrays of different data types.
 */
template <class T>
class NullableVector
{
    friend class Buffer;

public:
    /// @brief Destructor
    virtual ~NullableVector() {}

    /// @brief Sets all elements to false
    void clear();

    /// @brief Returns const reference to a specific value
    const T get(unsigned int index);

    /// @brief Returns string of a specific value
    const std::string getAsString(unsigned int index);

    /// @brief Sets specific value
    void set(unsigned int index, T value);
    void setFromFormat(unsigned int index, const std::string& format, const std::string& value_str, bool debug=false);
    void setAll(T value);

    /// @brief Appends specific value
    void append(unsigned int index, T value);
    void appendFromFormat(unsigned int index, const std::string& format,
                          const std::string& value_str);

    /// @brief Sets specific element to Null value
    void setNull(unsigned int index);
    void setAllNull();

    NullableVector<T>& operator*=(double factor);

    std::set<T> distinctValues(unsigned int index = 0);
    std::map<T, unsigned int> distinctValuesWithCounts(unsigned int index = 0);
    std::tuple<bool,T,T> minMaxValues(unsigned int index = 0); // set, min, max

    std::map<T, std::vector<unsigned int>> distinctValuesWithIndexes(unsigned int from_index,
                                                                     unsigned int to_index);
    std::map<T, std::vector<unsigned int>> distinctValuesWithIndexes(
            const std::vector<unsigned int>& indexes);
    std::vector<unsigned int> nullValueIndexes(unsigned int from_index, unsigned int to_index);
    std::vector<unsigned int> nullValueIndexes(const std::vector<unsigned int>& indexes);

    void convertToStandardFormat(const std::string& from_format);

    unsigned int size();

    /// @brief Checks if specific element is Null
    bool isNull(unsigned int index);

    bool isNeverNull();

    void swapData (unsigned int index1, unsigned int index2);

    std::string propertyName() const
    {
        return property_.name();
    }

    std::string propertyID() const
    {
        return property_.name() + "(" + property_.dataTypeString() + ")";
    }

    std::vector<std::size_t> sortPermutation();
    void sortByPermutation(const std::vector<std::size_t>& perm);

private:
    Property property_;
    Buffer& buffer_;
    /// Data container
    std::vector<T> data_;
    // Null flags container
    std::vector<bool> null_flags_;

    /// @brief Sets specific element to not Null value
    void unsetNull(unsigned int index);

    void resizeDataTo(unsigned int size);
    void resizeNullTo(unsigned int size);
    void addData(NullableVector<T>& other);
    void copyData(NullableVector<T>& other);
    void cutToSize(unsigned int size);

    /// @brief Constructor, only for friend Buffer
    NullableVector(Property& property, Buffer& buffer);
};

template <class T>
NullableVector<T>::NullableVector(Property& property, Buffer& buffer)
    : property_(property), buffer_(buffer)
{
}

template <class T>
void NullableVector<T>::clear()
{
    logdbg << "NullableVector " << property_.name() << ": clear";
    std::fill(data_.begin(), data_.end(), T());
    std::fill(null_flags_.begin(), null_flags_.end(), true);
}

template <class T>
const T NullableVector<T>::get(unsigned int index)
{
    logdbg << "NullableVector " << property_.name() << ": get: index " << index;
    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert(data_.size() <= buffer_.data_size_);
        assert(null_flags_.size() <= buffer_.data_size_);
        assert(index < data_.size());
        assert(index < data_.size());
    }

    if (isNull(index))
    {
        if (BUFFER_PEDANTIC_CHECKING)
        {
            logerr << "NullableVector " << property_.name() << ": get: index " << index
                   << " is null";
            assert(false);
        }

        throw std::runtime_error("NullableVector: get of Null value " + std::to_string(index));
    }

    return data_.at(index);
}

template <class T>
const std::string NullableVector<T>::getAsString(unsigned int index)
{
    logdbg << "NullableVector " << property_.name() << ": getAsString";
    return Utils::String::getValueString(get(index));
}

template <class T>
void NullableVector<T>::set(unsigned int index, T value)
{
    logdbg << "NullableVector " << property_.name() << ": set: index " << index << " value '"
           << value << "'";

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert(data_.size() <= buffer_.data_size_);
        assert(null_flags_.size() <= buffer_.data_size_);
    }

    if (index >= data_.size())  // allocate new stuff, fill all new with not null
    {
        if (index != data_.size())  // some where left out
            resizeNullTo(index + 1);

        resizeDataTo(index + 1);
    }

    if (BUFFER_PEDANTIC_CHECKING)
        assert(index < data_.size());

    data_.at(index) = value;
    unsetNull(index);

    // logdbg << "NullableVector: set: size " << size_ << " max_size " << max_size_;
}

template <class T>
void NullableVector<T>::setFromFormat(unsigned int index, const std::string& format,
                                      const std::string& value_str, bool debug)
{
    logdbg << "NullableVector " << property_.name() << ": setFromFormat";
    T value;

    if (format == "octal")
    {
        value = std::stoi(value_str, 0, 8);
    }
    else if (format == "hexadecimal")
    {
        value = std::stoi(value_str, 0, 16);
    }
    else if (format == "epoch_tod_ms")
    {
        QDateTime date_time;
        date_time.setMSecsSinceEpoch(std::stoul(value_str));
        value = Utils::String::timeFromString(date_time.toString("hh:mm:ss.zzz").toStdString());
    }
    else if (format == "epoch_tod_s")
    {
        QDateTime date_time;
        date_time.setMSecsSinceEpoch(1000 * std::stoul(value_str));
        value = Utils::String::timeFromString(date_time.toString("hh:mm:ss.zzz").toStdString());
    }
    else if (format == "bool")
    {
        if (value_str == "0" || value_str == "false")
            value = 'N';
        else if (value_str == "1"  || value_str == "true")
            value = 'Y';
    }
    else if (format == "bool_invert")
    {
        if (value_str == "1"  || value_str == "true")
            value = 'N';
        else if (value_str == "0" || value_str == "false")
            value = 'Y';
    }
    else
    {
        logerr << "NullableVector: setFromFormat: unknown format '" << format << "'";
        assert(false);
    }

    if (debug)
        loginf << "NullableVector: setFromFormat: index " << index << " value_str '" << value_str
               << "' value '" << value << "'";

    set(index, value);
}

template <class T>
void NullableVector<T>::setAll(T value)
{
    unsigned int data_size = data_.size();

    for (unsigned int cnt=0; cnt < data_size; ++cnt)
    {
        data_.at(index) = value;
        unsetNull(index);
    }
}

template <class T>
void NullableVector<T>::append(unsigned int index, T value)
{
    logdbg << "NullableVector " << property_.name() << ": append: index " << index << " value '"
           << value << "'";

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert(data_.size() <= buffer_.data_size_);
        assert(null_flags_.size() <= buffer_.data_size_);
    }

    if (index >= data_.size())  // allocate new stuff, fill all new with not null
    {
        if (index != data_.size())  // some where left out
            resizeNullTo(index + 1);

        resizeDataTo(index + 1);
    }

    if (BUFFER_PEDANTIC_CHECKING)
        assert(index < data_.size());

    data_.at(index) += value;
    unsetNull(index);

    // logdbg << "NullableVector: set: size " << size_ << " max_size " << max_size_;
}

template <class T>
void NullableVector<T>::appendFromFormat(unsigned int index, const std::string& format,
                                         const std::string& value_str)
{
    logdbg << "NullableVector " << property_.name() << ": appendFromFormat";
    T value;

    if (format == "octal")
    {
        value = std::stoi(value_str, 0, 8);
    }
    else if (format == "hexadecimal")
    {
        value = std::stoi(value_str, 0, 16);
    }
    else if (format == "epoch_tod_ms")
    {
        QDateTime date_time;
        date_time.setMSecsSinceEpoch(std::stoul(value_str));
        value = Utils::String::timeFromString(date_time.toString("hh:mm:ss.zzz").toStdString());
    }
    else if (format == "epoch_tod_s")
    {
        QDateTime date_time;
        date_time.setMSecsSinceEpoch(1000 * std::stoul(value_str));
        value = Utils::String::timeFromString(date_time.toString("hh:mm:ss.zzz").toStdString());
    }
    else
    {
        logerr << "NullableVector: appendFromFormat: unknown format '" << format << "'";
        assert(false);
    }

    append(index, value);
}

template <class T>
void NullableVector<T>::setNull(unsigned int index)
{
    logdbg << "NullableVector " << property_.name() << ": setNull: index " << index;

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert(data_.size() <= buffer_.data_size_);
        assert(null_flags_.size() <= buffer_.data_size_);
    }

    if (index >= null_flags_.size())  // null flags to small
        resizeNullTo(index + 1);

    if (BUFFER_PEDANTIC_CHECKING)
        assert(index < null_flags_.size());

    null_flags_.at(index) = true;
}

template <class T>
void NullableVector<T>::setAllNull()
{
    unsigned int data_size = data_.size();

    for (unsigned int cnt=0; cnt < data_size; ++cnt)
        setNull(index);
}


/// @brief Checks if specific element is Null
template <class T>
bool NullableVector<T>::isNull(unsigned int index)
{
    logdbg << "NullableVector " << property_.name() << ": isNull: index " << index;

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert(data_.size() <= buffer_.data_size_);
        assert(null_flags_.size() <= buffer_.data_size_);
        assert(index < buffer_.data_size_);
    }

    if (index < null_flags_.size())  // if stored, return value
        return null_flags_.at(index);

    // null not stored, so all set are not null

    if (index >= data_.size())  // not yet set
        return true;

    // must be set
    return false;
}

template <class T>
void NullableVector<T>::resizeDataTo(unsigned int size)
{
    logdbg << "NullableVector " << property_.name() << ": resizeDataTo: size " << size;

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert(data_.size() <= buffer_.data_size_);
        assert(data_.size() < size);  // only to be called if needed
    }

    data_.resize(size, T());

    if (buffer_.data_size_ < data_.size())  // set new data size
        buffer_.data_size_ = data_.size();
}

template <class T>
void NullableVector<T>::resizeNullTo(unsigned int size)
{
    logdbg << "NullableVector " << property_.name() << ": resizeNullTo: size " << size;

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (size >= null_flags_.size());
        assert(null_flags_.size() <= buffer_.data_size_);
    }

    if (data_.size() > null_flags_.size())  // data was set w/o null, adjust & fill with set values
        null_flags_.resize(data_.size(), false);

    if (null_flags_.size() < size)  // adjust to new size, fill with null values
        null_flags_.resize(size, true);

    if (buffer_.data_size_ < null_flags_.size())  // set new data size
        buffer_.data_size_ = null_flags_.size();

    if (BUFFER_PEDANTIC_CHECKING)
        assert(null_flags_.size() >= size); // could be larger since increase to data.size()
}

template <class T>
void NullableVector<T>::addData(NullableVector<T>& other)
{
    logdbg << "NullableVector " << property_.name() << ": addData";

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert(data_.size() <= buffer_.data_size_);
        assert(null_flags_.size() <= buffer_.data_size_);
    }

    if (!other.data_.size() &&
            other.null_flags_.size())  // if other has null flags set, need to fill my nulls
    {
        logdbg << "NullableVector " << property_.name()
               << ": addData: 1: other no data resizing null";
        resizeNullTo(buffer_.data_size_);
        logdbg << "NullableVector " << property_.name() << ": addData: 1: inserting null";
        null_flags_.insert(null_flags_.end(), other.null_flags_.begin(), other.null_flags_.end());
        goto DONE;
    }

    if (other.data_.size() && !other.null_flags_.size())  // if other has everything set
    {
        logdbg << "NullableVector " << property_.name()
               << ": addData: 2: other has everything set";

        if (data_.size() < buffer_.data_size_)  // need to size data up
        {
            logdbg << "NullableVector " << property_.name()
                   << ": addData: 2: data not full, setting null";
            resizeNullTo(buffer_.data_size_);

            logdbg << "NullableVector " << property_.name() << ": addData: 2: resizing data";
            resizeDataTo(buffer_.data_size_);
        }

        logdbg << "NullableVector " << property_.name() << ": addData: 2: inserting data";
        data_.insert(data_.end(), other.data_.begin(), other.data_.end());
        goto DONE;
    }

    logdbg << "NullableVector " << property_.name()
           << ": addData: 3: mixture, both have data & nulls";

    logdbg << "NullableVector " << property_.name() << ": addData: 3: resizing null to "
           << buffer_.data_size_;
    resizeNullTo(buffer_.data_size_);
    logdbg << "NullableVector " << property_.name() << ": addData: 3: inserting nulls";
    null_flags_.insert(null_flags_.end(), other.null_flags_.begin(), other.null_flags_.end());

    if (data_.size() < buffer_.data_size_)  // need to size data up
    {
        logdbg << "NullableVector " << property_.name() << ": addData: 3: resizing data";
        resizeDataTo(buffer_.data_size_);
    }

    logdbg << "NullableVector " << property_.name() << ": addData: 3: inserting data";
    data_.insert(data_.end(), other.data_.begin(), other.data_.end());

DONE:
    // size is adjusted in Buffer::seizeBuffer

    logdbg << "NullableVector " << property_.name() << ": addData: end";
}

template <class T>
void NullableVector<T>::copyData(NullableVector<T>& other)
{
    logdbg << "NullableVector " << property_.name() << ": copyData";

    data_ = other.data_;
    null_flags_ = other.null_flags_;

    // is only done for new buffers in Buffer::getPartialCopy, so no size-too-big isse

    if (buffer_.data_size_ < data_.size())
        buffer_.data_size_ = data_.size();

    if (buffer_.data_size_ < null_flags_.size())
        buffer_.data_size_ = null_flags_.size();

    logdbg << "NullableVector " << property_.name() << ": copyData: end";
}

template <class T>
NullableVector<T>& NullableVector<T>::operator*=(double factor)
{
    logdbg << "NullableVector " << property_.name() << ": operator*=";

    unsigned int data_size = data_.size();

    tbb::parallel_for(uint(0), data_size, [&](unsigned int cnt) {
        if (!isNull(cnt))
        {
            data_.at(cnt) *= factor;
        }
    });

    //    for (auto &data_it : data_)
    //        data_it *= factor;

    return *this;
}

template <class T>
std::set<T> NullableVector<T>::distinctValues(unsigned int index)
{
    logdbg << "NullableVector " << property_.name() << ": distinctValues";

    std::set<T> values;

    T value;

    for (; index < data_.size(); ++index)
    {
        if (!isNull(index))  // not for null
        {
            value = data_.at(index);
            if (values.count(value) == 0)
                values.insert(value);
        }
    }

    return values;
}

template <class T>
std::map<T, unsigned int> NullableVector<T>::distinctValuesWithCounts(unsigned int index)
{
    logdbg << "NullableVector " << property_.name() << ": distinctValuesWithCounts";

    std::map<T, unsigned int> values;

    T value;

    for (; index < data_.size(); ++index)
    {
        if (!isNull(index))  // not for null
        {
            value = data_.at(index);
            values[value] += 1;
        }
    }

    return values;
}

template <class T>
std::tuple<bool,T,T> NullableVector<T>::minMaxValues(unsigned int index)
{
    bool set = false;
    T min, max;

    for (; index < data_.size(); ++index)
    {
        if (!isNull(index))  // not for null
        {
            if (!set)
            {
                min = data_.at(index);
                max = data_.at(index);
                set = true;
            }
            else
            {
                min = std::min(min, data_.at(index));
                max = std::max(max, data_.at(index));
            }
        }
    }

    return std::tuple<bool,T,T> {set, min, max};
}

template <class T>
std::map<T, std::vector<unsigned int>> NullableVector<T>::distinctValuesWithIndexes(
        unsigned int from_index, unsigned int to_index)
{
    logdbg << "NullableVector " << property_.name() << ": distinctValuesWithIndexes";

    std::map<T, std::vector<unsigned int>> values;

    assert(from_index <= to_index);

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert(to_index);
        assert(from_index < buffer_.data_size_);
        assert(to_index < buffer_.data_size_);
        assert(data_.size() <= buffer_.data_size_);
        assert(null_flags_.size() <= buffer_.data_size_);
    }

    if (from_index + 1 > data_.size())  // no data
        return values;

    for (unsigned int index = from_index; index <= to_index; ++index)
    {
        if (!isNull(index))  // not for null
        {
            if (BUFFER_PEDANTIC_CHECKING)
                assert(index < data_.size());

            values[data_.at(index)].push_back(index);
        }
    }

    logdbg << "NullableVector " << property_.name() << ": distinctValuesWithIndexes: done with "
           << values.size();
    return values;
}

template <class T>
std::map<T, std::vector<unsigned int>> NullableVector<T>::distinctValuesWithIndexes(
        const std::vector<unsigned int>& indexes)
{
    logdbg << "NullableVector " << property_.name() << ": distinctValuesWithIndexes";

    std::map<T, std::vector<unsigned int>> values;

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert(data_.size() <= buffer_.data_size_);
        assert(null_flags_.size() <= buffer_.data_size_);
    }

    for (auto index : indexes)
    {
        if (!isNull(index))  // not for null
        {
            if (BUFFER_PEDANTIC_CHECKING)
                assert(index < data_.size());

            values[data_.at(index)].push_back(index);
        }
    }

    logdbg << "NullableVector " << property_.name() << ": distinctValuesWithIndexes: done with "
           << values.size();
    return values;
}

template <class T>
std::vector<unsigned int> NullableVector<T>::nullValueIndexes(unsigned int from_index,
                                                              unsigned int to_index)
{
    logdbg << "NullableVector " << property_.name() << ": nullValueIndexes";

    std::vector<unsigned int> indexes;

    assert(from_index <= to_index);

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert(to_index);
        assert(from_index < buffer_.data_size_);
        assert(to_index < buffer_.data_size_);
        assert(data_.size() <= buffer_.data_size_);
        assert(null_flags_.size() <= buffer_.data_size_);
    }

    //    if (from_index+1 >= data_.size()) // no data
    //        return indexes;

    for (unsigned int index = from_index; index <= to_index; ++index)
    {
        if (isNull(index))  // not for null
        {
            if (BUFFER_PEDANTIC_CHECKING)
                assert(index < data_.size());

            indexes.push_back(index);
        }
    }

    logdbg << "NullableVector " << property_.name() << ": nullValueIndexes: done with "
           << indexes.size();
    return indexes;
}

template <class T>
std::vector<unsigned int> NullableVector<T>::nullValueIndexes(
        const std::vector<unsigned int>& indexes)
{
    logdbg << "NullableVector " << property_.name() << ": nullValueIndexes";

    std::vector<unsigned int> ret_indexes;

    for (auto index : indexes)
    {
        if (isNull(index))  // not for null
        {
            if (BUFFER_PEDANTIC_CHECKING)
                assert(index < data_.size());

            ret_indexes.push_back(index);
        }
    }

    logdbg << "NullableVector " << property_.name() << ": nullValueIndexes: done with "
           << ret_indexes.size();
    return ret_indexes;
}

template <class T>
void NullableVector<T>::convertToStandardFormat(const std::string& from_format)
{
    logdbg << "NullableVector " << property_.name() << ": convertToStandardFormat";

    static_assert(std::is_integral<T>::value, "only defined for integer types");

    // std::string value_str;
    // T value;

    if (from_format != "octal")
    {
        logerr << "NullableVector: convertToStandardFormat: unknown format '" << from_format
               << "'";
        assert(false);
    }

    unsigned int data_size = data_.size();

    tbb::parallel_for(uint(0), data_size, [&](unsigned int cnt) {
        if (!isNull(cnt))
        {
            // value_str = std::to_string(data_.at(cnt));
            data_.at(cnt) = std::stoi(std::to_string(data_.at(cnt)), 0, 8);
        }
    });

    //    for (unsigned int cnt=0; cnt < data_size; cnt++)
    //    {
    //        if (isNull(cnt))
    //            continue;

    //        value_str = std::to_string(data_.at(cnt));

    //        if (from_format == "octal")
    //        {
    //            data_.at(cnt) = std::stoi(value_str, 0, 8);
    //        }
    //        else
    //        {
    //            logerr << "NullableVector: convertToStandardFormat: unknown format '" <<
    //            from_format << "'"; assert (false);
    //        }
    //    }
}

template <class T>
unsigned int NullableVector<T>::size()
{
    return data_.size();
}

template <class T>
void NullableVector<T>::cutToSize(unsigned int size)
{
    logdbg << "NullableVector " << property_.name() << ": cutToSize: size " << size;

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert(data_.size() <= buffer_.data_size_);
        assert(null_flags_.size() <= buffer_.data_size_);
    }

    while (null_flags_.size() > size)
        null_flags_.pop_back();

    while (data_.size() > size)
        data_.pop_back();

    // size set in Buffer::cutToSize
}

template <class T>
bool NullableVector<T>::isNeverNull()
{
    logdbg << "NullableVector " << property_.name() << ": isNeverNull";

    for (unsigned int cnt = 0; cnt < null_flags_.size(); cnt++)
    {
        if (null_flags_.at(cnt))
            return true;
    }

    return false;
}

template <class T>
void NullableVector<T>::swapData (unsigned int index1, unsigned int index2)
{
    bool index1_null = isNull(index1);
    bool index2_null = isNull(index2);

    if (index1_null && index2_null)
        return;
    else if (!index1_null && !index2_null)
    {
        assert (index1 < data_.size());
        assert (index2 < data_.size());

        T val = get(index1);
        set(index1, get(index2));
        set(index2, val);
    }
    else if (index1_null && !index2_null)
    {
        assert (index2 < data_.size());

        set(index1, get(index2));
        setNull(index2);
    }
    else if (!index1_null && index2_null)
    {
        assert (index1 < data_.size());

        set(index2, get(index1));
        setNull(index1);
    }
}

//from https://stackoverflow.com/questions/17074324/how-can-i-sort-two-vectors-in-the-same-way-with-criteria-that-uses-only-one-of

template <class T>
std::vector<std::size_t> NullableVector<T>::sortPermutation()
{
    //assert (isNeverNull());

    loginf << "UGA sortPermutation data size " << buffer_.size();

    if (data_.size() < buffer_.size())
        resizeDataTo(buffer_.size());

    assert (data_.size() == buffer_.size());
    std::vector<std::size_t> p (data_.size());

    std::iota(p.begin(), p.end(), 0);
    std::sort(p.begin(), p.end(),
              [&](std::size_t i, std::size_t j){

        bool is_i_null = isNull(i);
        bool is_j_null = isNull(j);

        if (is_i_null && is_j_null)
            return false; // same not smaller
        else if (!is_i_null && is_j_null)
            return false; // not null < null = false
        else if (is_i_null && !is_j_null)
            return true; // null < not null = true
        else
            return data_.at(i) < data_.at(j);
    });
    return p;
}

template <class T>
void NullableVector<T>::sortByPermutation(const std::vector<std::size_t>& perm)
{
//    std::vector<bool> done(data_.size());

    std::vector<bool> done(perm.size());

//    for (std::size_t i = 0; i < data_.size(); ++i)

    for (std::size_t i = 0; i < perm.size(); ++i)
    {
        if (done.at(i))
            continue;

        assert (i < done.size());
        done.at(i) = true;
        std::size_t prev_j = i;

        assert (i < perm.size());
        std::size_t j = perm.at(i);
        while (i != j)
        {
            //std::swap(data_[prev_j], data_[j]);
            swapData(prev_j, j);

            assert (j < done.size());
            done.at(j) = true;
            prev_j = j;
            assert (j < perm.size());
            j = perm.at(j);
        }
    }


//    for (std::size_t i = 0; i < data_.size(); ++i)
//    {
//        if (done[i])
//        {
//            continue;
//        }
//        done[i] = true;
//        std::size_t prev_j = i;
//        std::size_t j = perm[i];
//        while (i != j)
//        {
//            //std::swap(data_[prev_j], data_[j]);
//            swapData(prev_j, j);

//            done[j] = true;
//            prev_j = j;
//            j = perm[j];
//        }
//    }
}

// private stuff

/// @brief Sets specific element to not Null value
template <class T>
void NullableVector<T>::unsetNull(unsigned int index)
{
    logdbg << "NullableVector " << property_.name() << ": unsetNull";

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert(data_.size() <= buffer_.data_size_);
        assert(null_flags_.size() <= buffer_.data_size_);
        assert(index < buffer_.data_size_);
        assert(index < data_.size());
    }

    if (index < null_flags_.size())  // if was already set
        null_flags_.at(index) = false;
}

template <>
NullableVector<bool>& NullableVector<bool>::operator*=(double factor);

template <>
void NullableVector<bool>::append(unsigned int index, bool value);

template <>
void NullableVector<std::string>::append(unsigned int index, std::string value);

#endif /* ARRAYLIST_H_ */
