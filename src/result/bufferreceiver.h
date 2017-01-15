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
 * BufferReceiver.h
 *
 *  Created on: Sep 17, 2013
 *      Author: sk
 */

#ifndef BUFFERRECEIVER_H_
#define BUFFERRECEIVER_H_

#include "BufferSet.h"
#include "Logger.h"

class Buffer;

/**
 * @brief Interface for receivers of buffers
 *
 * Sub-classes have to override receive().
 */
class BufferReceiver
{
public:
    BufferReceiver() {}
    virtual ~BufferReceiver() {}

    /// @brief Is called when a buffer is delivered, override but call base function
    virtual void receive (Buffer *buffer)
    {
        logdbg << "BufferReceiver: receive: adding buffer to buffer set";
        buffer_set_.addBuffer(buffer);
    }

    void deleteReceivedBuffers ()
    {
        buffer_set_.clearAndDelete();
    }

    virtual void loadingDone ()=0;

protected:
    BufferSet buffer_set_;

};

#endif /* BUFFERRECEIVER_H_ */
