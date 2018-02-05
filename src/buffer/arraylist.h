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

#include "logger.h"
#include "property.h"
#include "stringconv.h"

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

    bool hasSpecialRepresentation () { return representation_ != Utils::String::Representation::STANDARD; }
    Utils::String::Representation representation() const;
    void representation(const Utils::String::Representation &representation);

    virtual const std::string getAsString (size_t index)=0;

protected:
    /// Identifier of contained data
    std::string id_;
    /// Size of the data contents, maximum index of set+1
    size_t size_;
    /// Size of data arrays
    size_t max_size_;

    Utils::String::Representation representation_;

    std::vector < std::shared_ptr< std::array<bool,BUFFER_ARRAY_SIZE> > > none_flags_;
    //std::vector <std::shared_ptr<std::bitset<BUFFER_ARRAY_SIZE>>> none_flags_;

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
    virtual ~ArrayListTemplate () {}

    /// @brief Sets all elements to false
    virtual void clear() override
    {
        for (auto& data_it : data_)
            data_it->fill(T());

        setAllNone();
    }

    /// @brief Returns const reference to a specific value
    const T &get (size_t index)
    {
        assert (index <= size_);
        assert (!isNone(index));

        if (index > size_)
            throw std::out_of_range ("ArrayListTemplate: get out of index "+std::to_string(index));

        if (isNone(index))
            throw std::out_of_range ("ArrayListTemplate: get of None value "+std::to_string(index));

        return data_[index/BUFFER_ARRAY_SIZE]->at (index%BUFFER_ARRAY_SIZE);
    }

    /// @brief Returns string of a specific value
    const std::string getAsString (size_t index) override
    {
        if (index > size_)
            throw std::out_of_range ("ArrayListTemplate: getAsString out of index "+std::to_string(index));

        if (isNone(index))
            throw std::out_of_range ("ArrayListTemplate: getAsString of None value "+std::to_string(index));

        return Utils::String::getValueString (data_[index/BUFFER_ARRAY_SIZE]->at (index%BUFFER_ARRAY_SIZE));
    }

    /// @brief Returns representation string of a specific value
    const std::string getAsRepresentationString (size_t index)
    {
        if (index > size_)
            throw std::out_of_range ("ArrayListTemplate: getAsRepresentationString out of index "+std::to_string(index));

        if (isNone(index))
            throw std::out_of_range ("ArrayListTemplate: getAsRepresentationString of None value "+std::to_string(index));

        if (representation_ == Utils::String::Representation::STANDARD)
            return Utils::String::getValueString (data_[index/BUFFER_ARRAY_SIZE]->at (index%BUFFER_ARRAY_SIZE));

        return Utils::String::getAsSpecialRepresentationString (data_[index/BUFFER_ARRAY_SIZE]->at (index%BUFFER_ARRAY_SIZE), representation_);
    }

    /// @brief Sets specific value
    void set (size_t index, T value)
    {
        //loginf << "ArrayListBool:set: index " << index << " current size-1 " << size_-1;

        if (index >= max_size_)
        {
            //logdbg << "ArrayListTemplate:set: adding new arrays for index " << index << " current max size " << max_size_;
            while (index >= max_size_)
                allocateNewArray ();
        }

        //logdbg << "ArrayListTemplate: set: setting index " << index << " to value " << value << " using array " << index/BUFFER_ARRAY_SIZE << " array_index " << index%BUFFER_ARRAY_SIZE;

        data_[index/BUFFER_ARRAY_SIZE]->at (index%BUFFER_ARRAY_SIZE) = value;

        if (index >= size_)
            size_= index+1;

        //logdbg << "ArrayListTemplate: set: size " << size_ << " max_size " << max_size_;

        unsetNone(index);
    }

    /// @brief Sets specific element to None value
    virtual void setNone(size_t index) override
    {
        if (index >= max_size_)
        {
            //logdbg << "ArrayListTemplate:setNone: adding new arrays for index " << index << " current max size " << max_size_;
            while (index >= max_size_)
                allocateNewArray ();
        }

        ArrayListBase::setNone(index);
    }

    void addData (ArrayListTemplate<T> &other)
    {
        logdbg << "ArrayListTemplate: addData: data size " << data_.size() << " none flags size " << none_flags_.size() << " size " << size_ << " max " << max_size_;
        data_.insert(data_.end(), other.data_.begin(), other.data_.end());
        none_flags_.insert(none_flags_.end(), other.none_flags_.begin(), other.none_flags_.end());
        assert (data_.size() == none_flags_.size());
        size_ = max_size_ + other.size_;
        max_size_ += other.max_size_;

        other.data_.clear();
        other.none_flags_.clear();
        other.size_=0;
        other.max_size_=0;

        logdbg << "ArrayListTemplate: addData: end data size " << data_.size() << " none flags size " << none_flags_.size() << " size " << size_ << " max " << max_size_;
    }

    ArrayListTemplate<T>& operator*=(double factor)
    {
        for (auto &data_it : data_)
            for (unsigned int cnt=0; cnt < BUFFER_ARRAY_SIZE; cnt++)
                data_it->at(cnt) *= factor;

        return *this;
    }

    std::set<T> distinctValues (size_t index=0)
    {
        std::set<T> values;

        T value;

        size_t first_list_cnt=0;
        unsigned list_cnt=0;

        size_t first_list_row=0;
        size_t list_row_cnt;

        if (index)
        {
            first_list_cnt = index/BUFFER_ARRAY_SIZE;
            first_list_row = index%BUFFER_ARRAY_SIZE;
        }

        for (; list_cnt < data_.size(); list_cnt++)
        {
            std::shared_ptr< std::array<T,BUFFER_ARRAY_SIZE> > array_list = data_.at(list_cnt);

            if (index && list_cnt == first_list_cnt) // there is a start index
                list_row_cnt=first_list_row;
            else
                list_row_cnt=0;

            for (; list_row_cnt < BUFFER_ARRAY_SIZE; list_row_cnt++)
            {
                if (!none_flags_.at(list_cnt)->at(list_row_cnt)) // not for none
                {
                    value = array_list->at(list_row_cnt);
                    if (values.count(value) == 0)
                        values.insert(value);
                }
            }
        }
        return values;
    }

    std::map<T, std::vector<size_t>> distinctValuesWithIndexes (size_t from_index, size_t to_index)
    {
        std::map<T, std::vector<size_t>> values;

        assert (to_index);
        assert (from_index < to_index);

        size_t first_list_cnt = from_index/BUFFER_ARRAY_SIZE;
        unsigned list_cnt = first_list_cnt;
        unsigned list_size = to_index/BUFFER_ARRAY_SIZE;
        size_t first_list_row = from_index%BUFFER_ARRAY_SIZE;
        size_t list_row_cnt = 0;

        logdbg << "ArrayList: distinctValuesWithIndexes: from_index " << from_index << " to_index " << to_index
               << " first_list_cnt " << first_list_cnt << " list_cnt " << list_cnt << " list size " << list_size
               << " first_list_row " << first_list_row;

//        T none_index = std::numeric_limits<T>::max();

        for (; list_cnt < list_size; list_cnt++)
        {
            std::shared_ptr< std::array<T,BUFFER_ARRAY_SIZE> > array_list = data_.at(list_cnt);

            if (list_cnt == first_list_cnt) // there is a start index
                list_row_cnt=first_list_row;
            else
                list_row_cnt=0;

            for (; list_row_cnt < BUFFER_ARRAY_SIZE; list_row_cnt++)
            {
                if (!none_flags_.at(list_cnt)->at(list_row_cnt)) // not for none
                {
                    values[array_list->at(list_row_cnt)].push_back(list_cnt*BUFFER_ARRAY_SIZE+list_row_cnt);
                }
//                else // add to unknown sensor
//                {
//                    values[none_index].push_back(list_cnt*BUFFER_ARRAY_SIZE+list_row_cnt);
//                }
            }
        }

        logdbg << "ArrayList: distinctValuesWithIndexes: done with " << values.size();
        return values;
    }

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

        //logdbg << "ArrayListTemplate: allocateNewArray: added new array current max size " << max_size_;
    }
};


//template <>
//const std::string ArrayListTemplate<std::string>::getAsString (size_t index);

//template <>
//const std::string ArrayListTemplate<float>::getAsString (size_t index);

//template <>
//const std::string ArrayListTemplate<double>::getAsString (size_t index);

template <>
ArrayListTemplate<bool>& ArrayListTemplate<bool>::operator*=(double factor);



#endif /* ARRAYLIST_H_ */
