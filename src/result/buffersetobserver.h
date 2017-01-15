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
 * BufferSetObserver.h
 *
 *  Created on: Mar 1, 2013
 *      Author: sk
 */

#ifndef BUFFERSETOBSERVER_H_
#define BUFFERSETOBSERVER_H_

class Buffer;

/**
 * @brief Interface for observers of a BufferSet
 *
 * Sub-classes have to override notifyAdd() and clear().
 */
class BufferSetObserver
{
public:
	BufferSetObserver() {}
	virtual ~BufferSetObserver() {}

	/// @brief Is called when a buffer is added
	virtual void notifyAdd (Buffer *buffer)=0;
	/// @brief Is called when set is cleared
	virtual void clear ()=0;
};

#endif /* BUFFERSETOBSERVER_H_ */
