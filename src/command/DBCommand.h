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
 * DBCommand.h
 *
 *  Created on: Feb 1, 2012
 *      Author: sk
 */

#ifndef DBCOMMAND_H_
#define DBCOMMAND_H_

#include "PropertyList.h"

/**
 * @brief Encapsulation of a database SQL command
 *
 * @details Contains the command string and the PropertyList of the expected data structure returned by the command.
 * The PropertyList is copied when set and deleted in the destructor.
 */
class DBCommand
{
public:
  /// @brief Constructor
	DBCommand();
	/// @brief Destructor
	virtual ~DBCommand();

	/// @brief Sets command string
	void setCommandString (std::string command);
	/// @brief Sets PropertyList of exptected data.
	void setPropertyList (PropertyList list);

	/// @brief Returns command string
	std::string getCommandString ();
	/// @brief Returns flag indicating if returned data is expected.
	bool getExpectDataResult ();
	/// @brief Returns PropertyList of expected data.
	PropertyList *getResultList ();

private:
	/// SQL Command
	std::string command_;
	/// Flag if return of data is expected
	bool expect_data_result_;
	/// PropertyList of expected data
	PropertyList result_list_;
};

#endif /* DBCOMMAND_H_ */
