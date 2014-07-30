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
 * DBResult.cpp
 *
 *  Created on: Feb 1, 2012
 *      Author: sk
 */

#include "Buffer.h"
#include "DBResult.h"

DBResult::DBResult(bool contains_data, Buffer *buffer)
{
	contains_data_=contains_data;
	buffer_=buffer;
}

DBResult::DBResult()
{
	contains_data_=false;
	buffer_=0;
}

DBResult::~DBResult()
{
}

void DBResult::setBuffer (Buffer *buffer)
{
	assert (buffer);
	assert (!buffer_);
	buffer_=buffer;
	contains_data_=true;
}
Buffer *DBResult::getBuffer ()
{
	assert (buffer_);
	return buffer_;
}

bool DBResult::containsData ()
{
	return contains_data_;
}
