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

#include <type_traits>

#include <QDateTime>

#include "logger.h"
#include "property.h"
#include "stringconv.h"

static const unsigned int BUFFER_ARRAY_SIZE=10000;

const bool ARRAYLIST_PEDANTIC_CHECKING=true;

/**
 * @brief List interface of fixed-size arrays to be used in Buffer classes.
 *
 * Was written for easy management of arrays of different data types.
 */
class ArrayListBase
{
public:
    /// @brief Constructor
    ArrayListBase () {}
    /// @brief Destructor
    virtual ~ArrayListBase () {}

    /// @brief Returns size of the list
    size_t size () { return none_flags_.size(); }

    /// @brief Sets specific element to None value
    virtual void setNone(size_t index)
    {
        if (index >= none_flags_.size()) // allocate new stuff, fill all new with not none
            none_flags_.resize(index+1, true);

        none_flags_.at(index) = true;
    }

    /// @brief Checks if specific element is None
    bool isNone(size_t index)
    {
        if (ARRAYLIST_PEDANTIC_CHECKING)
            assert (index < none_flags_.size());

        return none_flags_.at(index);
    }

    /// @brief Sets all elements to initial value and None information to true
    virtual void clear()=0;

    virtual const std::string getAsString (size_t index)=0;

private:
    //std::vector < std::shared_ptr< std::array<bool,BUFFER_ARRAY_SIZE> > > none_flags_;
    std::vector <bool> none_flags_;

protected:
    /// @brief Sets all elements to None value
    void setAllNone() { std::fill (none_flags_.begin(), none_flags_.end(), true); }

    /// @brief Sets specific element to not None value
    void unsetNone (size_t index)
    {
        if (index >= none_flags_.size()) // allocate new stuff, fill all new with not none
            none_flags_.resize(index+1, true);

        none_flags_.at(index) = false;
    }

    void addNone (ArrayListBase& other)
    {
        none_flags_.insert(none_flags_.end(), other.none_flags_.begin(), other.none_flags_.end());
    }
};


/**
 * @brief Template List of fixed-size arrays to be used in Buffer classes.
 *
 * Was written for easy management of arrays of different data types.
 */
template <class T>
class ArrayListTemplate : public ArrayListBase
{
public:
    /// @brief Constructor
    ArrayListTemplate ()
        : ArrayListBase () {}

    /// @brief Destructor
    virtual ~ArrayListTemplate () {}

    /// @brief Sets all elements to false
    virtual void clear() override
    {
        std::fill (data_.begin(),data_.end(), T());
        setAllNone();
    }

    /// @brief Returns const reference to a specific value
    const T get (size_t index)
    {
        if (isNone(index))
            throw std::runtime_error ("ArrayListTemplate: get of None value "+std::to_string(index));

        return data_.at(index);
    }

    /// @brief Returns string of a specific value
    const std::string getAsString (size_t index) override
    {
        if (isNone(index))
            throw std::runtime_error ("ArrayListTemplate: getAsString of None value "+std::to_string(index));

        return Utils::String::getValueString (data_.at(index));
    }

    /// @brief Sets specific value
    void set (size_t index, T value)
    {
        if (index >= data_.size()) // allocate new stuff, fill all new with not none
        {
            data_.resize(index+1, T());
        }

        data_.at(index) = value;

        //logdbg << "ArrayListTemplate: set: size " << size_ << " max_size " << max_size_;

        unsetNone(index);

        if (ARRAYLIST_PEDANTIC_CHECKING)
            assert (data_.size() == ArrayListBase::size());
    }

    void setFromFormat (size_t index, const std::string& format, const std::string& value_str)
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

    /// @brief Sets specific element to None value
    virtual void setNone(size_t index) override
    {
        if (index >= data_.size()) // allocate new stuff, fill all new with not none
        {
            data_.resize(index+1, T());
        }

        ArrayListBase::setNone(index);

        if (ARRAYLIST_PEDANTIC_CHECKING)
            assert (data_.size() == ArrayListBase::size());
    }

    void addData (ArrayListTemplate<T> &other)
    {
        logdbg << "ArrayListTemplate: addData: data size " << data_.size();

        data_.insert(data_.end(), other.data_.begin(), other.data_.end());
        addNone(other);

        other.data_.clear();
        other.setAllNone();

        if (ARRAYLIST_PEDANTIC_CHECKING)
            assert (data_.size() == ArrayListBase::size());

        logdbg << "ArrayListTemplate: addData: end data size " << data_.size();
    }

    ArrayListTemplate<T>& operator*=(double factor)
    {
        for (auto &data_it : data_)
            data_it *= factor;

        return *this;
    }

    std::set<T> distinctValues (size_t index=0)
    {
        std::set<T> values;

        T value;

        for (; index < data_.size(); ++index)
        {
            if (!none_flags_.at(index)) // not for none
            {
                value = data_.at(index);
                if (values.count(value) == 0)
                    values.insert(value);
            }
        }
    }

    std::map<T, std::vector<size_t>> distinctValuesWithIndexes (size_t from_index, size_t to_index)
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
                if (ARRAYLIST_PEDANTIC_CHECKING)
                {
                    assert (index < data_.size());
                    assert (index < ArrayListBase::size());
                }

                values[data_.at(index)].push_back(index);
            }
        }

        logdbg << "ArrayList: distinctValuesWithIndexes: done with " << values.size();
        return values;
    }

    void convertToStandardFormat(const std::string& from_format)
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

protected:
    /// Data container
    std::vector<T> data_;
};


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
