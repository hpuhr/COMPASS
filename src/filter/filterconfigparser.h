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
 * FilterConfigParser.h
 *
 *  Created on: Oct 28, 2011
 *      Author: sk
 */

#ifndef FILTERCONFIGPARSER_H_
#define FILTERCONFIGPARSER_H_

#include <string>
#include <vector>

#include "DBFilter.h"

/**
 * @brief Outdated class, parses SQL statements
 */
class FilterConfigParser
{
protected:
	std::vector<DBFilter*> filters_;
public:
	FilterConfigParser();
	virtual ~FilterConfigParser();

	void loadFilterConfig (std::string filename);
	std::vector<DBFilter *> getFilters ();
};

#endif /* FILTERCONFIGPARSER_H_ */
