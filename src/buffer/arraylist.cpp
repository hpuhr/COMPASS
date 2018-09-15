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

typedef std::numeric_limits<double> double_limit;
typedef std::numeric_limits<float> float_limit;

#include "arraylist.h"

ArrayListBase::ArrayListBase ()
{

}

ArrayListBase::~ArrayListBase ()
{

}

size_t ArrayListBase::size ()
{
    return size_;
}

size_t ArrayListBase::maximumSize ()
{
    return max_size_;
}

void ArrayListBase::setAllNone()
{
//    for (auto it : none_flags_)
//        it->fill(true);
    std::fill (none_flags_.begin(),none_flags_.end(), true);
}

void ArrayListBase::setNone(size_t index)
{
    //loginf << "UGA setnone";

    if (index >= size_)
        logerr << "ArrayListBase: setNone: index " << index << " too large for size " << size_;
    assert (index < size_);

    //(*none_flags_[index/BUFFER_ARRAY_SIZE])[index%BUFFER_ARRAY_SIZE] = true;

    if (index >= none_flags_.size()) // allocate new stuff, fill all new with not none
    {
//        size_t old_size = none_flags_.size();
//        none_flags_.reserve(index+1);
//        std::fill(none_flags_.begin()+old_size, none_flags_.end(), false);
        none_flags_.resize(index+1, true);
    }

    none_flags_.at(index) = true;

    //loginf << "UGA setnone done";
}

void ArrayListBase::unsetNone (size_t index)
{
    //loginf << "UGA unsetnone";
    if (index >= size_)
        logerr << "ArrayListBase: unsetNone: index " << index << " too large for size " << size_;

    assert (index < size_);

    if (index >= none_flags_.size()) // allocate new stuff, fill all new with not none
    {
//        //size_t old_size = none_flags_.size();
//        //none_flags_.reserve(index+1);
//        //loginf << "UGA index " << index << " current size " << none_flags_.size() << " old_size " << old_size;
//        //std::fill(none_flags_.begin()+old_size, none_flags_.end(), false);
//        //loginf << "UGA index " << index << " current size " << none_flags_.size() << " done";
        none_flags_.resize(index+1, true);
    }

//    if (index <= none_flags_.size()) // not allocated counts as not none
//        return;

    //(*none_flags_[index/BUFFER_ARRAY_SIZE])[index%BUFFER_ARRAY_SIZE] = false;
    none_flags_.at(index) = false;

   // loginf << "UGA unsetnone done";
}

bool ArrayListBase::isNone(size_t index)
{
    //loginf << "UGA isNone";

    if (index >= size_)
        return true;

    //assert (index < size_);
    //return (*none_flags_[index/BUFFER_ARRAY_SIZE])[index%BUFFER_ARRAY_SIZE];

    if (index >= none_flags_.size()) // not yet allocated, can't be none
        return false;

    bool ret = none_flags_.at(index);

    //loginf << "UGA isNone done";

    return ret;
}

//void ArrayListBase::allocatedNewNoneArray ()
//{
//    std::shared_ptr< std::array<bool,BUFFER_ARRAY_SIZE> > new_array_ptr =
//            std::make_shared<std::array<bool,BUFFER_ARRAY_SIZE>>();

//    new_array_ptr->fill(true);

//    none_flags_.push_back(new_array_ptr);

    //size_t old_size = ;
//    for (size_t cnt = none_flags_.size(); cnt < none_flags_.size()+BUFFER_ARRAY_SIZE; ++cnt)
//        none_flags_[cnt] = true;
    //none_flags_[old_size+BUFFER_ARRAY_SIZE] = true;
    //std::fill (none_flags_.begin()+old_size,none_flags_.begin()+old_size+BUFFER_ARRAY_SIZE, true);

//    none_flags_.resize(max_size_,true);
//}

void ArrayListBase::addNone (ArrayListBase& other)
{
    none_flags_.insert(none_flags_.end(), other.none_flags_.begin(), other.none_flags_.end());
}

//template <class T>
//ArrayListTemplate<T>& ArrayListTemplate<T>::operator*=(double factor)
//{
//    //DO STUFF
//    return *this;
//}

//template <>
//const std::string ArrayListTemplate<std::string>::getAsString (size_t index)
//{
//    if (index > size_)
//        throw std::out_of_range ("ArrayListTemplate: getAsString out of index "+std::to_string(index));

//    if (isNone(index))
//        throw std::out_of_range ("ArrayListTemplate: getAsString of None value "+std::to_string(index));

//    return data_[index/BUFFER_ARRAY_SIZE]->at (index%BUFFER_ARRAY_SIZE);
//}

//template <>
//const std::string ArrayListTemplate<float>::getAsString (size_t index)
//{
//    if (index > size_)
//        throw std::out_of_range ("ArrayListTemplate: getAsString out of index "+std::to_string(index));

//    if (isNone(index))
//        throw std::out_of_range ("ArrayListTemplate: getAsString of None value "+std::to_string(index));

//    std::ostringstream out;
//    out << std::setprecision (float_limit::max_digits10) << data_[index/BUFFER_ARRAY_SIZE]->at (index%BUFFER_ARRAY_SIZE);
//    return out.str();
//}

//template <>
//const std::string ArrayListTemplate<double>::getAsString (size_t index)
//{
//    if (index > size_)
//        throw std::out_of_range ("ArrayListTemplate: getAsString out of index "+std::to_string(index));

//    if (isNone(index))
//        throw std::out_of_range ("ArrayListTemplate: getAsString of None value "+std::to_string(index));

//    std::ostringstream out;
//    out << std::setprecision (double_limit::max_digits10) << data_[index/BUFFER_ARRAY_SIZE]->at (index%BUFFER_ARRAY_SIZE);
//    return out.str();
//}

template <>
ArrayListTemplate<bool>& ArrayListTemplate<bool>::operator*=(double factor)
{
    size_t list_size = data_.size();
    bool tmp_factor = static_cast<bool> (factor);

    size_t index=0;

    for (size_t list_cnt=0; list_cnt < list_size; list_cnt++)
    {
        for (size_t cnt=0; cnt < BUFFER_ARRAY_SIZE; cnt++)
        {
            if (!isNone(index)) // not for none
                data_.at(list_cnt)->at(cnt) = data_.at(list_cnt)->at(cnt) && tmp_factor;
            ++index;
        }
    }

    return *this;
}

