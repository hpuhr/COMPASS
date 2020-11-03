/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "dboassociationcollection.h"

#include <sstream>

void DBOAssociationCollection::add(unsigned int rec_num, DBOAssociationEntry&& entry)
{
    entries_.emplace(rec_num, entry);
}

bool DBOAssociationCollection::contains(unsigned int rec_num) const
{
    return entries_.count(rec_num);
}

std::vector<unsigned int> DBOAssociationCollection::getUTNsFor(unsigned int rec_num) const
{
    std::vector<unsigned int> ret;

    typedef std::multimap<unsigned int, DBOAssociationEntry>::const_iterator MMAPIterator;

    std::pair<MMAPIterator, MMAPIterator> result = entries_.equal_range(rec_num);

    for (MMAPIterator it = result.first; it != result.second; it++)
        ret.push_back(it->second.utn_);

    return ret;
}

std::string DBOAssociationCollection::getUTNsStringFor(unsigned int rec_num) const
{
    std::stringstream ss;

    typedef std::multimap<unsigned int, DBOAssociationEntry>::const_iterator MMAPIterator;

    std::pair<MMAPIterator, MMAPIterator> result = entries_.equal_range(rec_num);

    bool first = true;

    for (MMAPIterator it = result.first; it != result.second; it++)
    {
        if (first)
            ss << std::to_string(it->second.utn_);
        else
            ss << "," << std::to_string(it->second.utn_);

        first = false;
    }

    return ss.str();
}

std::vector<unsigned int> DBOAssociationCollection::getRecNumsForUTN(unsigned int utn) const
{
    std::vector<unsigned int> result;

    for (auto& entry : entries_)
    {
        if (entry.second.utn_ == utn)
            result.push_back(entry.first);
    }

    return result;
}
