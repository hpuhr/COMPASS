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

#ifndef ARRAYLIST_H_
#define ARRAYLIST_H_

#include <memory>
#include <vector>
#include <bitset>
#include <sstream>
#include <iomanip>
#include <array>
#include <set>
#include <map>

#include <QDateTime>

#include "stringconv.h"
#include "arraylist.h"
#include "buffer.h"
#include "property.h"

const bool BUFFER_PEDANTIC_CHECKING=false;

/**
 * @brief Template List of fixed-size arrays to be used in Buffer classes.
 *
 * Was written for easy management of arrays of different data types.
 */
template <class T>
class ArrayListTemplate
{
    friend class Buffer;

public:
    /// @brief Destructor
    virtual ~ArrayListTemplate () {}

    /// @brief Sets all elements to false
    void clear();

    /// @brief Returns const reference to a specific value
    const T get (size_t index);

    /// @brief Returns string of a specific value
    const std::string getAsString (size_t index);

    /// @brief Sets specific value
    void set (size_t index, T value);

    void setFromFormat (size_t index, const std::string& format, const std::string& value_str);

    /// @brief Sets specific element to None value
    void setNone(size_t index);

    ArrayListTemplate<T>& operator*=(double factor);

    std::set<T> distinctValues (size_t index=0);

    std::map<T, std::vector<size_t>> distinctValuesWithIndexes (size_t from_index, size_t to_index);

    void convertToStandardFormat(const std::string& from_format);

    size_t size();


    /// @brief Returns size of the list
    //size_t noneSize ();

    //void cutNoneToSize (size_t size);

    /// @brief Checks if specific element is None
    bool isNone(size_t index);

    void checkNotNone ();


private:
    Property property_;
    Buffer& buffer_;
    /// Data container
    std::vector<T> data_;
    // None flags container
    std::vector <bool> none_flags_;

    /// @brief Sets specific element to not None value
    void unsetNone (size_t index);

    void resizeDataTo (size_t size);
    void resizeNoneTo (size_t size);
    void addData (ArrayListTemplate<T>& other);
    void copyData (ArrayListTemplate<T>& other);
    void cutToSize (size_t size);

    /// @brief Constructor, only for friend Buffer
    ArrayListTemplate (Property& property, Buffer& buffer);


    //void addNone (ArrayListTemplate<T>& other);

    /// @brief Sets specific element to None value
    //void setNoneFlag(size_t index);
};


template <class T> ArrayListTemplate<T>::ArrayListTemplate (Property& property, Buffer& buffer)
    : property_(property), buffer_(buffer) {}

template <class T> void ArrayListTemplate<T>::clear()
{
    logdbg << "ArrayListTemplate " << property_.name() << ": clear";
    std::fill (data_.begin(),data_.end(), T());
    std::fill (none_flags_.begin(), none_flags_.end(), true);
}

template <class T> const T ArrayListTemplate<T>::get (size_t index)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": get: index " << index;
    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (data_.size() <= buffer_.data_size_);
        assert (none_flags_.size() <= buffer_.data_size_);
        assert (index < data_.size());
        assert (index < data_.size());
    }

    if (isNone(index))
    {
        if (BUFFER_PEDANTIC_CHECKING)
        {
            logerr << "ArrayListTemplate " << property_.name() << ": get: index " << index << " is none";
            assert (false);
        }

        throw std::runtime_error ("ArrayListTemplate: get of None value "+std::to_string(index));
    }

    return data_.at(index);
}

template <class T> const std::string ArrayListTemplate<T>::getAsString (size_t index)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": getAsString";
    return Utils::String::getValueString (get(index));
}

template <class T> void ArrayListTemplate<T>::set (size_t index, T value)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": set: index " << index << " value '" << value << "'";

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (data_.size() <= buffer_.data_size_);
        assert (none_flags_.size() <= buffer_.data_size_);
    }

    if (index >= data_.size()) // allocate new stuff, fill all new with not none
    {
        if (index != data_.size()) // some where left out
            resizeNoneTo(index+1);

        resizeDataTo (index+1);
    }

    if (BUFFER_PEDANTIC_CHECKING)
        assert (index < data_.size());

    data_.at(index) = value;
    unsetNone(index);

    //logdbg << "ArrayListTemplate: set: size " << size_ << " max_size " << max_size_;
}

template <class T> void ArrayListTemplate<T>::setFromFormat (size_t index, const std::string& format,
                                                             const std::string& value_str)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": setFromFormat";
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
    logdbg << "ArrayListTemplate " << property_.name() << ": setNone: index " << index;

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (data_.size() <= buffer_.data_size_);
        assert (none_flags_.size() <= buffer_.data_size_);
    }

    if (index >= none_flags_.size()) // none flags to small
        resizeNoneTo (index+1);

    if (BUFFER_PEDANTIC_CHECKING)
        assert (index < none_flags_.size());

    none_flags_.at(index) = true;
}

/// @brief Checks if specific element is None
template <class T> bool ArrayListTemplate<T>::isNone(size_t index)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": isNone: index " << index;

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (data_.size() <= buffer_.data_size_);
        assert (none_flags_.size() <= buffer_.data_size_);
        assert (index < buffer_.data_size_);
    }

    if (index < none_flags_.size()) // if stored, return value
        return none_flags_.at(index);

    // none not stored, so all set are not none

    if (index >= data_.size()) // not yet set
        return true;

    // must be set
    return false;
}

template <class T> void ArrayListTemplate<T>::resizeDataTo (size_t size)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": resizeDataTo: size " << size;

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (data_.size() <= buffer_.data_size_);
        assert (data_.size() < size); // only to be called if needed
    }

    data_.resize(size, T());

    if (buffer_.data_size_ < data_.size()) // set new data size
        buffer_.data_size_ = data_.size();
}

template <class T> void ArrayListTemplate<T>::resizeNoneTo (size_t size)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": resizeNoneTo: size " << size;

    if (BUFFER_PEDANTIC_CHECKING)
        assert (none_flags_.size() <= buffer_.data_size_);

    if (data_.size() > none_flags_.size()) // data was set w/o none, adjust & fill with set values
        none_flags_.resize(data_.size(), false);

    if (none_flags_.size() < size) // adjust to new size, fill with none values
        none_flags_.resize(size, true);

    if (buffer_.data_size_ < none_flags_.size()) // set new data size
        buffer_.data_size_ = none_flags_.size();

    if (BUFFER_PEDANTIC_CHECKING)
        assert (size == none_flags_.size());
}

template <class T> void ArrayListTemplate<T>::addData (ArrayListTemplate<T>& other)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": addData";

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (data_.size() <= buffer_.data_size_);
        assert (none_flags_.size() <= buffer_.data_size_);
    }

    if (!other.data_.size() && other.none_flags_.size()) // if other has none flags set, need to fill my nones
    {
        logdbg << "ArrayListTemplate " << property_.name() << ": addData: 1: other no data resizing none";
        resizeNoneTo (buffer_.data_size_);
        logdbg << "ArrayListTemplate " << property_.name() << ": addData: 1: inserting none";
        none_flags_.insert(none_flags_.end(), other.none_flags_.begin(), other.none_flags_.end());
        goto DONE;
    }

    if (other.data_.size() && !other.none_flags_.size()) // if other has everything set
    {
        logdbg << "ArrayListTemplate " << property_.name() << ": addData: 2: other has everything set";

        if (data_.size() < buffer_.data_size_) // need to size data up
        {
            logdbg << "ArrayListTemplate " << property_.name() << ": addData: 2: data not full, setting none";
            resizeNoneTo (buffer_.data_size_);

            logdbg << "ArrayListTemplate " << property_.name() << ": addData: 2: resizing data";
            resizeDataTo (buffer_.data_size_);
        }

        logdbg << "ArrayListTemplate " << property_.name() << ": addData: 2: inserting data";
        data_.insert(data_.end(), other.data_.begin(), other.data_.end());
        goto DONE;
    }

    logdbg << "ArrayListTemplate " << property_.name() << ": addData: 3: mixture, both have data & nones";

    logdbg << "ArrayListTemplate " << property_.name() << ": addData: 3: resizing none to " << buffer_.data_size_;
    resizeNoneTo (buffer_.data_size_);
    logdbg << "ArrayListTemplate " << property_.name() << ": addData: 3: inserting nones";
    none_flags_.insert(none_flags_.end(), other.none_flags_.begin(), other.none_flags_.end());

    if (data_.size() < buffer_.data_size_) // need to size data up
    {
        logdbg << "ArrayListTemplate " << property_.name() << ": addData: 3: resizing data";
        resizeDataTo (buffer_.data_size_);
    }

    logdbg << "ArrayListTemplate " << property_.name() << ": addData: 3: inserting data";
    data_.insert(data_.end(), other.data_.begin(), other.data_.end());

DONE:
    // size is adjusted in Buffer::seizeBuffer

    logdbg << "ArrayListTemplate " << property_.name() << ": addData: end";
}

template <class T> void ArrayListTemplate<T>::copyData (ArrayListTemplate<T>& other)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": copyData";

    data_ = other.data_;
    none_flags_ = other.none_flags_;

    // is only done for new buffers in Buffer::getPartialCopy, so no size-too-big isse

    if (buffer_.data_size_ < data_.size())
        buffer_.data_size_ = data_.size();

    if (buffer_.data_size_ < none_flags_.size())
        buffer_.data_size_ = none_flags_.size();

    logdbg << "ArrayListTemplate " << property_.name() << ": copyData: end";
}

template <class T> ArrayListTemplate<T>& ArrayListTemplate<T>::operator*=(double factor)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": operator*=";

    for (auto &data_it : data_)
        data_it *= factor;

    return *this;
}

template <class T> std::set<T> ArrayListTemplate<T>::distinctValues (size_t index)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": distinctValues";

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

template <class T> std::map<T, std::vector<size_t>> ArrayListTemplate<T>::distinctValuesWithIndexes (size_t from_index,
                                                                                                     size_t to_index)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": distinctValuesWithIndexes";

    std::map<T, std::vector<size_t>> values;

    assert (from_index < to_index);

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (to_index);
        assert (from_index < buffer_.data_size_);
        assert (to_index < buffer_.data_size_);
        assert (data_.size() <= buffer_.data_size_);
        assert (none_flags_.size() <= buffer_.data_size_);
    }

    if (from_index+1 > data_.size()) // no data
        return values;

    for (size_t index = from_index; index <= to_index; ++index)
    {
        if (!isNone(index)) // not for none
        {
            if (BUFFER_PEDANTIC_CHECKING)
                assert (index < data_.size());

            values[data_.at(index)].push_back(index);
        }
    }

    logdbg << "ArrayListTemplate " << property_.name() << ": distinctValuesWithIndexes: done with " << values.size();
    return values;
}

template <class T> void ArrayListTemplate<T>::convertToStandardFormat(const std::string& from_format)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": convertToStandardFormat";

    static_assert (std::is_integral<T>::value, "only defined for integer types");

    std::string value_str;
    //T value;

    size_t data_size = data_.size();
    for (size_t cnt=0; cnt < data_size; cnt++)
    {
        if (isNone(cnt))
            continue;

        value_str = std::to_string(data_.at(cnt));

        if (from_format == "octal")
        {
            data_.at(cnt) = std::stoi(value_str, 0, 8);
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
    logdbg << "ArrayListTemplate " << property_.name() << ": cutToSize: size " << size;

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (data_.size() <= buffer_.data_size_);
        assert (none_flags_.size() <= buffer_.data_size_);
    }

//    if (size > data_.size())
//        return;

    //assert (size <= data_.size());

    while (none_flags_.size() > size)
        none_flags_.pop_back();

    while (data_.size() > size)
        data_.pop_back();

    // size set in Buffer::cutToSize
}

/// @brief Returns size of the list
//template <class T> size_t ArrayListTemplate<T>::noneSize () { return none_flags_.size(); }

//template <class T> void ArrayListTemplate<T>::cutNoneToSize (size_t size)
//{
//    while (none_flags_.size() > size)
//        none_flags_.pop_back();

//    // size set in Buffer::cutToSize
//}

/// @brief Sets specific element to None value
//template <class T> void ArrayListTemplate<T>::setNoneFlag(size_t index)
//{
//    if (BUFFER_PEDANTIC_CHECKING)
//    {
//        assert (data_.size() <= buffer_.data_size_);
//        assert (none_flags_.size() <= buffer_.data_size_);
//    }

//    if (index >= none_flags_.size()) // do need to allocate stuff for setting
//    {
//        if (none_flags_.size() < data_.size()) // for already set data, set not null
//            none_flags_.resize(data_.size(), false);

//        // allocate new stuff, fill all new with not none
//        none_flags_.resize(index+1, true);
//    }

//    if (BUFFER_PEDANTIC_CHECKING)
//    {
//        assert (index < buffer_.data_size_);
//        assert (index < data_.size());
//        assert (index < none_flags_.size());
//    }

//    none_flags_.at(index) = true;
//}



template <class T> void ArrayListTemplate<T>::checkNotNone ()
{
    logdbg << "ArrayListTemplate " << property_.name() << ": checkNotNone";

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

/// @brief Sets specific element to not None value
template <class T> void ArrayListTemplate<T>::unsetNone (size_t index)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": unsetNone";

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert (data_.size() <= buffer_.data_size_);
        assert (none_flags_.size() <= buffer_.data_size_);
        assert (index < buffer_.data_size_);
        assert (index < data_.size());
    }

    if (index < none_flags_.size()) // if was already set
        none_flags_.at(index) = false;
}

//template <class T> void ArrayListTemplate<T>::addNone (ArrayListTemplate<T>& other)
//{
//    if (other.none_flags_.size())
//    {
//        if (none_flags_.size() < data_.size()) // for already set data, set not null
//            none_flags_.resize(data_.size(), false);

//        none_flags_.insert(none_flags_.end(), other.none_flags_.begin(), other.none_flags_.end());
//    }
//}

//template <>
//ArrayListTemplate<bool>& ArrayListTemplate<bool>::operator*=(double factor)
//{
//    bool tmp_factor = static_cast<bool> (factor);

//    for (auto data_it : data_)
//        data_it = data_it && tmp_factor;

//    return *this;
//}


//template <class T> Base* Foo<T>::convert(ID) const {return new Bar<T>;}



// For integral types only:
//template<typename T>
//typename std::enable_if<std::is_integral<T>::value>::type f(T t)
//{
//    // ...
//}

//template <>
//const std::string ArrayListTemplate<std::string>::getAsString (size_t index);

//template <>
//const std::string ArrayListTemplate<float>::getAsString (size_t index);

//template <>
//const std::string ArrayListTemplate<double>::getAsString (size_t index);

template <>
ArrayListTemplate<bool>& ArrayListTemplate<bool>::operator*=(double factor);




#endif /* ARRAYLIST_H_ */
