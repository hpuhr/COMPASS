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
 * DBCommandList.cpp
 *
 *  Created on: Feb 3, 2012
 *      Author: sk
 */

#include "DBCommandList.h"

DBCommandList::DBCommandList()
{
	expect_data_result_=false;
}

DBCommandList::~DBCommandList()
{
}

void DBCommandList::addCommandString (std::string command)
{
	commands_.push_back (command);
}

void DBCommandList::addCommandStrings (std::vector<std::string> commands)
{
	commands_.insert (commands_.end(), commands.begin(), commands.end());
}

void DBCommandList::setPropertyList (PropertyList list)
{
	result_list_=list;
	expect_data_result_=true;
}

std::string DBCommandList::getCommandString (unsigned int i)
{
	assert (i < commands_.size());
	return commands_.at(i);
}
unsigned int DBCommandList::getNumCommands ()
{
	return commands_.size();
}
std::vector <std::string> DBCommandList::getCommands ()
{
	return commands_;
}

bool DBCommandList::getExpectDataResult ()
{
	return expect_data_result_;
}
PropertyList *DBCommandList::getResultList ()
{
	return &result_list_;
}
