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
 * ArrayList.h
 *
 *      Author: sk
 */

#ifndef ARRAYLIST_H_
#define ARRAYLIST_H_

#include <memory>
#include <vector>

#include "Logger.h"

static const unsigned int BUFFER_ARRAY_SIZE=10000;

/**
 * @brief List interface of fixed-size arrays to be used in Buffer classes.
 *
 * Was written for easy management of arrays of different data types.
 */
class ArrayListBase
{
public:
    /// @brief Constructor
    ArrayListBase ();
    /// @brief Destructor
    virtual ~ArrayListBase ();

    /// @brief Returns size of the list
    size_t size ();

    /// @brief Returns current maximum size of the list
    size_t maximumSize ();

    /// @brief Sets specific element to None value
    virtual void setNone(size_t size_t);

    /// @brief Checks if specific element is None
    bool isNone(size_t size_t);

    /// @brief Sets all elements to initial value and None information to true
    virtual void clear()=0;

protected:
    /// Identifier of contained data
    std::string id_;
    /// Size of the data contents, maximum index of set+1
    size_t size_;
    /// Size of data arrays
    size_t max_size_;

    std::vector < std::shared_ptr< std::array<bool,BUFFER_ARRAY_SIZE> > > none_flags_;

    /// @brief Allocates a new none array
    void allocatedNewNoneArray ();
    /// @brief Sets all elements to None value
    void setAllNone();
    /// @brief Sets specific element to not None value
    void unsetNone (size_t index);
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
    virtual ~ArrayListTemplate () {};

    /// @brief Sets all elements to false
    virtual void clear()
    {
        typename std::vector < std::shared_ptr< std::array<T,BUFFER_ARRAY_SIZE> > >::iterator it;

        for (it = data_.begin(); it != data_.end(); it++)
            for (unsigned int cnt=0; cnt < BUFFER_ARRAY_SIZE; cnt++)
                it->get()->at(cnt)=T();

        setAllNone();
    };

    /// @brief Returns const reference to a specific value
    const T &get (size_t index)
    {
        if (index > size_)
            throw std::out_of_range ("ArrayListTemplate: get out of index");

        if (isNone(index))
            throw std::out_of_range ("ArrayListTemplate: get of None value");

        return data_[index/BUFFER_ARRAY_SIZE]->at (index%BUFFER_ARRAY_SIZE);
    };

    /// @brief Returns string representation of a specific value
    const std::string &getAsString (size_t index)
    {
        if (index > size_)
            throw std::out_of_range ("ArrayListTemplate: get out of index");

        if (isNone(index))
            throw std::out_of_range ("ArrayListTemplate: get of None value");

        return std::string (data_[index/BUFFER_ARRAY_SIZE]->at (index%BUFFER_ARRAY_SIZE));
    };

    /// @brief Sets specific value
    void set (size_t index, T value)
    {
        //loginf << "ArrayListBool:set: index " << index << " current size-1 " << size_-1;

        if (index >= max_size_)
        {
            logdbg << "ArrayListTemplate:set: adding new arrays for index " << index << " current max size " << max_size_;
            while (index >= max_size_)
                allocateNewArray ();
        }

        logdbg << "ArrayListTemplate: set: setting index " << index << " to value " << value << " using array " << index/BUFFER_ARRAY_SIZE << " array_index " << index%BUFFER_ARRAY_SIZE;

        data_[index/BUFFER_ARRAY_SIZE]->at (index%BUFFER_ARRAY_SIZE) = value;

        if (index >= size_)
            size_= index+1;

        logdbg << "ArrayListTemplate: set: size " << size_ << " max_size " << max_size_;

        unsetNone(index);
    };

    /// @brief Sets specific element to None value
    virtual void setNone(size_t index)
    {
        if (index >= max_size_)
        {
            logdbg << "ArrayListTemplate:setNone: adding new arrays for index " << index << " current max size " << max_size_;
            while (index >= max_size_)
                allocateNewArray ();
        }

        ArrayListBase::setNone(index);
    };

protected:
    /// Data containers
    std::vector < std::shared_ptr< std::array<T,BUFFER_ARRAY_SIZE> > > data_;

    /// @brief Adds a new data container
    void allocateNewArray ()
    {
        std::shared_ptr< std::array<T,BUFFER_ARRAY_SIZE> > new_array_ptr = std::make_shared<std::array<T,BUFFER_ARRAY_SIZE>>();
        data_.push_back(new_array_ptr);
        max_size_ += BUFFER_ARRAY_SIZE;

        allocatedNewNoneArray();

        assert (data_.size() == none_flags_.size());

        logdbg << "ArrayListTemplate: allocateNewArray: added new array current max size " << max_size_;
    };
};

#endif /* ARRAYLIST_H_ */
