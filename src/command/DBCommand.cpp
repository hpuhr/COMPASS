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
 * DBCommand.cpp
 *
 *  Created on: Feb 1, 2012
 *      Author: sk
 */

#include "DBCommand.h"

DBCommand::DBCommand()
{
	expect_data_result_=false;
}

DBCommand::~DBCommand()
{
}

void DBCommand::setCommandString (std::string command)
{
	command_=command;
}
void DBCommand::setPropertyList (PropertyList list)
{
	result_list_=list;
	expect_data_result_=true;
}

std::string DBCommand::getCommandString ()
{
	return command_;
}
bool DBCommand::getExpectDataResult ()
{
	return expect_data_result_;
}
PropertyList *DBCommand::getResultList ()
{
	return &result_list_;
}
