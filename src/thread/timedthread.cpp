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


#include "timedthread.h"


/**
Constructor.
@param interval_ms Interval time in milliseconds.
@param id Thread id.
  */
TimedThread::TimedThread( unsigned int interval_ms, const std::string& id )
:   Thread( id, false ),
    interval_( interval_ms )
{
}

/**
Destructor.
  */
TimedThread::~TimedThread()
{
}

/**
Reimplemented main thread working function.
  */
void TimedThread::do_work()
{
    while( !stop_requested_ )
    {
        boost::this_thread::sleep( boost::posix_time::milliseconds( interval_ ) );

        boost::mutex::scoped_lock statelock( state_mutex_ );
        state_= THREAD_STATE_WORKING;

        workFun();

        state_= THREAD_STATE_IDLE;
    }
}
