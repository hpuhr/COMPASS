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
 * ArrayList.cpp
 *
 *      Author: sk
 */

#include <cassert>

#include "ArrayList.h"

ArrayListBase::ArrayListBase ()
    : size_(0), max_size_(0)
{

}

ArrayListBase::~ArrayListBase ()
{

}

size_t ArrayListBase::getSize ()
{
    return size_;
}

size_t ArrayListBase::getMaximumSize ()
{
    return max_size_;
}

void ArrayListBase::setAllNone()
{
    std::vector < std::shared_ptr< std::array<bool,BUFFER_ARRAY_SIZE> > >::iterator it;

    for (it = none_flags_.begin(); it != none_flags_.end(); it++)
        for (unsigned int cnt=0; cnt < BUFFER_ARRAY_SIZE; cnt++)
            it->get()->at(cnt)=true;
}

void ArrayListBase::setNone(size_t index)
{
    assert (index < size_);
    none_flags_[index/BUFFER_ARRAY_SIZE]->at (index%BUFFER_ARRAY_SIZE) = true;
}

bool ArrayListBase::isNone(size_t index)
{
    assert (index < size_);
    return none_flags_[index/BUFFER_ARRAY_SIZE]->at (index%BUFFER_ARRAY_SIZE);

}

void ArrayListBase::allocatedNewNoneArray ()
{
    std::shared_ptr< std::array<bool,BUFFER_ARRAY_SIZE> > new_array_ptr = std::make_shared<std::array<bool,BUFFER_ARRAY_SIZE>>();

    for (unsigned int cnt=0; cnt < BUFFER_ARRAY_SIZE; cnt++) //initialize to all none
        new_array_ptr->at(cnt) = true;

    none_flags_.push_back(new_array_ptr);
}

void ArrayListBase::unsetNone (size_t index)
{
    assert (index < size_);
    none_flags_[index/BUFFER_ARRAY_SIZE]->at (index%BUFFER_ARRAY_SIZE) = false;
}
