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
 * DBResult.h
 *
 *  Created on: Feb 1, 2012
 *      Author: sk
 */

#ifndef DBRESULT_H_
#define DBRESULT_H_

class Buffer;

/**
 * @brief Generalized result of database query
 *
 * Simple encapsulation of a buffer (result data from query) and a flag indicating if a buffer was set.
 */
class DBResult
{
public:
  /// @brief Constructor with parameters
	DBResult(bool contains_data, Buffer *buffer);
	/// @brief Default constructor
	DBResult();
	/// @brief Destructor
	virtual ~DBResult();

	/// @brief Sets the result buffer
	void setBuffer (Buffer *buffer);
	/// @brief Returns the result buffer
	Buffer *getBuffer ();

	/// @brief Returns if contains data flag was set
	bool containsData ();

private:
	/// @brief Contains result data flag
	bool contains_data_;
	/// @brief Result data buffer
	Buffer *buffer_;
};

#endif /* DBRESULT_H_ */
