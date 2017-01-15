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
 * BufferSet.cpp
 *
 *  Created on: Mar 21, 2012
 *      Author: sk
 */

#include <algorithm>

#include "Buffer.h"
#include "BufferSetObserver.h"
#include "BufferSet.h"
#include "DBObjectManager.h"
#include "Logger.h"

BufferSet::BufferSet()
{
}

BufferSet::~BufferSet()
{
}

void BufferSet::addBuffer (Buffer *buffer)
{
    assert (buffer);
    boost::mutex::scoped_lock l(mutex_);
    data_.push_back(buffer);
    notifyObserversAdd (buffer);
}
Buffer *BufferSet::popBuffer ()
{
    boost::mutex::scoped_lock l(mutex_);
    assert (hasData());
    Buffer *buffer = data_.front();
    data_.pop_front();
    return buffer;
}

void BufferSet::addBuffers (std::list<Buffer *> buffers)
{
    //boost::mutex::scoped_lock l(mutex_);
    //data_.insert(data_.end(), buffers.begin(), buffers.end());

    std::list<Buffer*>::iterator it;

    for (it = buffers.begin(); it != buffers.end(); it++)
        addBuffer (*it);
}
std::list<Buffer *>BufferSet::popBuffers ()
{
    boost::mutex::scoped_lock l(mutex_);
    std::list<Buffer *> buffers = data_;
    data_.clear();
    return buffers;
}

std::list<Buffer *>BufferSet::getBuffers ()
{
    boost::mutex::scoped_lock l(mutex_);
    std::list<Buffer *> buffers = data_;
    return buffers;
}

void BufferSet::clear ()
{
    boost::mutex::scoped_lock l(mutex_);
    data_.clear();
    notifyObserversClear();
}


bool BufferSet::hasData ()
{
    return (data_.size()>0);
}

void BufferSet::clearAndDelete ()
{
    boost::mutex::scoped_lock l(mutex_);

    std::list<Buffer *>::iterator it;

    for (it = data_.begin(); it != data_.end(); it++)
    {
        delete *it;
    }

    data_.clear();
    notifyObserversClear();
}

unsigned int BufferSet::getSize ()
{
    return data_.size();
}

void BufferSet::addObserver (BufferSetObserver *observer)
{
    boost::mutex::scoped_lock l(observer_mutex_);
    assert (find (observers_.begin(), observers_.end(), observer) == observers_.end());
    observers_.push_back (observer);
}

void BufferSet::removeObserver (BufferSetObserver *observer)
{
    boost::mutex::scoped_lock l(observer_mutex_);
    assert (find (observers_.begin(), observers_.end(), observer) != observers_.end());
    observers_.erase (find (observers_.begin(), observers_.end(), observer));
}


void BufferSet::notifyObserversAdd (Buffer *buffer)
{
    assert (buffer);
    boost::mutex::scoped_lock l(observer_mutex_);
    std::vector <BufferSetObserver *>::iterator it;
    for (it=observers_.begin(); it != observers_.end(); it++)
        (*it)->notifyAdd (buffer);
}

void BufferSet::notifyObserversClear ()
{
    boost::mutex::scoped_lock l(observer_mutex_);
    std::vector <BufferSetObserver *>::iterator it;
    for (it=observers_.begin(); it != observers_.end(); it++)
        (*it)->clear();
}

