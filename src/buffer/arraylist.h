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

//static const unsigned int BUFFER_ARRAY_SIZE=10000;

const bool BUFFER_PEDANTIC_CHECKING=false;

class Buffer;

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
    /// @brief Constructor
    ArrayListTemplate (Buffer& buffer);

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

    void addData (ArrayListTemplate<T>& other);

    void copyData (ArrayListTemplate<T>& other);

    ArrayListTemplate<T>& operator*=(double factor);

    std::set<T> distinctValues (size_t index=0);

    std::map<T, std::vector<size_t>> distinctValuesWithIndexes (size_t from_index, size_t to_index);

    void convertToStandardFormat(const std::string& from_format);

    size_t size();

    void cutToSize (size_t size);

    /// @brief Returns size of the list
    size_t noneSize ();

    void cutNoneToSize (size_t size);

    /// @brief Sets specific element to None value
    void setNoneFlag(size_t index);

    /// @brief Checks if specific element is None
    bool isNone(size_t index);

    void checkNotNone ();


private:
    Buffer& buffer_;
    /// Data container
    std::vector<T> data_;
    // None flags container
    std::vector <bool> none_flags_;

    /// @brief Sets all elements to None value
    void setAllNone();

    /// @brief Sets specific element to not None value
    void unsetNone (size_t index);

    void addNone (ArrayListTemplate<T>& other);
};

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
