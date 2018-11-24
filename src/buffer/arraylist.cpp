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

#include <cassert>
#include <limits>

#include <QDateTime>

#include "stringconv.h"
#include "arraylist.h"
#include "buffer.h"

typedef std::numeric_limits<double> double_limit;
typedef std::numeric_limits<float> float_limit;

//#include "arraylist.h"

template <class T> ArrayListTemplate<T>::ArrayListTemplate (Buffer& buffer) : buffer_(buffer) {}

template <class T> void ArrayListTemplate<T>::clear()
{
    std::fill (data_.begin(),data_.end(), T());
    setAllNone();
}

template <class T> const T ArrayListTemplate<T>::get (size_t index)
{
    if (isNone(index))
        throw std::runtime_error ("ArrayListTemplate: get of None value "+std::to_string(index));

    return data_.at(index);
}

template <class T> const std::string ArrayListTemplate<T>::getAsString (size_t index)
{
    if (isNone(index))
        throw std::runtime_error ("ArrayListTemplate: getAsString of None value "+std::to_string(index));

    return Utils::String::getValueString (data_.at(index));
}

template <class T> void ArrayListTemplate<T>::set (size_t index, T value)
{
    if (buffer_.data_size_ < index+1)
        buffer_.data_size_ = index+1;

    if (index >= data_.size()) // allocate new stuff, fill all new with not none
    {
        data_.resize(index+1, T());
    }

    data_.at(index) = value;

    //logdbg << "ArrayListTemplate: set: size " << size_ << " max_size " << max_size_;

    unsetNone(index);

    if (BUFFER_PEDANTIC_CHECKING)
        assert (data_.size() == noneSize());
}

template <class T> void ArrayListTemplate<T>::setFromFormat (size_t index, const std::string& format, const std::string& value_str)
{
    T value;

    if (format == "octal")
    {
        value = std::stoi(value_str, 0, 8);
    }
    else if (format == "hexadecimal")
    {
        value = std::stoi(value_str, 0, 16);
    }
    else if (format == "epoch_tod")
    {
        QDateTime date_time;
        date_time.setMSecsSinceEpoch(std::stoul(value_str));
        value = Utils::String::timeFromString(date_time.toString("hh:mm:ss.zzz").toStdString());
    }
    else
    {
        logerr << "ArrayListTemplate: setFromFormat: unknown format '" << format << "'";
        assert (false);
    }

    set (index, value);
}

template <class T> void ArrayListTemplate<T>::setNone(size_t index)
{
    if (index >= data_.size()) // allocate new stuff, fill all new with not none
    {
        data_.resize(index+1, T());
    }

    setNoneFlag(index);

    if (BUFFER_PEDANTIC_CHECKING)
        assert (data_.size() == noneSize());
}

template <class T> void ArrayListTemplate<T>::addData (ArrayListTemplate<T>& other)
{
    logdbg << "ArrayListTemplate: addData: data size " << data_.size();

    assert (size() == noneSize());
    assert (other.size() == other.noneSize());

    data_.insert(data_.end(), other.data_.begin(), other.data_.end());
    addNone(other);

    other.data_.clear();
    other.setAllNone();

    if (BUFFER_PEDANTIC_CHECKING)
        assert (data_.size() == noneSize());

    logdbg << "ArrayListTemplate: addData: end data size " << data_.size();
}

template <class T> void ArrayListTemplate<T>::copyData (ArrayListTemplate<T>& other)
{
    data_ = other.data_;
    none_flags_ = other.none_flags_;
}

template <class T> ArrayListTemplate<T>& ArrayListTemplate<T>::operator*=(double factor)
{
    for (auto &data_it : data_)
        data_it *= factor;

    return *this;
}

template <class T> std::set<T> ArrayListTemplate<T>::distinctValues (size_t index)
{
    std::set<T> values;

    T value;

    for (; index < data_.size(); ++index)
    {
        if (!isNone(index)) // not for none
        {
            value = data_.at(index);
            if (values.count(value) == 0)
                values.insert(value);
        }
    }

    return values;
}

template <class T> std::map<T, std::vector<size_t>> ArrayListTemplate<T>::distinctValuesWithIndexes (size_t from_index, size_t to_index)
{
    std::map<T, std::vector<size_t>> values;

    assert (to_index);
    assert (from_index < to_index);
    assert (from_index < data_.size());
    assert (to_index < data_.size());

    for (size_t index = from_index; index <= to_index; ++index)
    {
        if (!isNone(index)) // not for none
        {
            if (BUFFER_PEDANTIC_CHECKING)
            {
                assert (index < data_.size());
                assert (index < noneSize());
            }

            values[data_.at(index)].push_back(index);
        }
    }

    logdbg << "ArrayList: distinctValuesWithIndexes: done with " << values.size();
    return values;
}

template <class T> void ArrayListTemplate<T>::convertToStandardFormat(const std::string& from_format)
{
    static_assert (std::is_integral<T>::value, "only defined for integer types");

    std::string value_str;
    //T value;

    size_t data_size = data_.size();
    for (size_t cnt=0; cnt < data_size; cnt++)
    {
        if (isNone(cnt))
            continue;

        value_str = std::to_string(data_[cnt]);

        if (from_format == "octal")
        {
            data_[cnt] = std::stoi(value_str, 0, 8);
        }
        else
        {
            logerr << "ArrayListTemplate: convertToStandardFormat: unknown format '" << from_format << "'";
            assert (false);
        }
    }
}

template <class T> size_t ArrayListTemplate<T>::size() { return data_.size(); }

template <class T> void ArrayListTemplate<T>::cutToSize (size_t size)
{
    cutNoneToSize(size);
    assert (size <= data_.size());
    while (data_.size() > size)
        data_.pop_back();
}

/// @brief Returns size of the list
template <class T> size_t ArrayListTemplate<T>::noneSize () { return none_flags_.size(); }

template <class T> void ArrayListTemplate<T>::cutNoneToSize (size_t size)
{
    assert (size <= none_flags_.size());
    while (none_flags_.size() > size)
        none_flags_.pop_back();
}

/// @brief Sets specific element to None value
template <class T> void ArrayListTemplate<T>::setNoneFlag(size_t index)
{
    if (index >= none_flags_.size()) // allocate new stuff, fill all new with not none
        none_flags_.resize(index+1, true);

    none_flags_.at(index) = true;
}

/// @brief Checks if specific element is None
template <class T> bool ArrayListTemplate<T>::isNone(size_t index)
{
    if (BUFFER_PEDANTIC_CHECKING)
        assert (index < none_flags_.size());

    return none_flags_.at(index);
}

template <class T> void ArrayListTemplate<T>::checkNotNone ()
{
    for (size_t cnt=0; cnt < none_flags_.size(); cnt++)
    {
       if (none_flags_.at(cnt))
       {
           logerr << "cnt " << cnt << " none";
           assert (false);
       }
    }
}

// private stuff

template <class T> void ArrayListTemplate<T>::setAllNone() { std::fill (none_flags_.begin(), none_flags_.end(), true); }

/// @brief Sets specific element to not None value
template <class T> void ArrayListTemplate<T>::unsetNone (size_t index)
{
    if (index >= none_flags_.size()) // allocate new stuff, fill all new with not none
        none_flags_.resize(index+1, true);

    none_flags_.at(index) = false;
}

template <class T> void ArrayListTemplate<T>::addNone (ArrayListTemplate<T>& other)
{
    none_flags_.insert(none_flags_.end(), other.none_flags_.begin(), other.none_flags_.end());
}

//template <>
//ArrayListTemplate<bool>& ArrayListTemplate<bool>::operator*=(double factor)
//{
//    bool tmp_factor = static_cast<bool> (factor);

//    for (auto data_it : data_)
//        data_it = data_it && tmp_factor;

//    return *this;
//}

