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

#include "timeconv.h"
#include "logger.h"

namespace Utils
{
namespace Time
{

std::string str_format = "%Y-%m-%d %H:%M:%S.%f";
std::string date_str_format = "%Y-%m-%d";
boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));

boost::posix_time::ptime fromLong(unsigned long value)
{
    boost::posix_time::ptime timestamp;

    timestamp = epoch + boost::posix_time::millisec(value);

    return timestamp;
}

long toLong(boost::posix_time::ptime value)
{
    long result;

    boost::posix_time::time_duration duration = value - epoch;
    result = duration.total_milliseconds();

    return result;
}

std::string toString(boost::posix_time::ptime value)
{
    std::stringstream date_stream;

    date_stream.imbue(std::locale(date_stream.getloc(), new boost::posix_time::time_facet(str_format.c_str())));
    date_stream << value;

    std::string tmp = date_stream.str();

    tmp.erase(tmp.length()-3); // remove microseconds since not supported by boost

    return tmp;
}

std::string toString(unsigned long value)
{
   return toString(fromLong(value));
}

std::string toDateString(boost::posix_time::ptime value)
{
    std::stringstream date_stream;

    date_stream.imbue(std::locale(date_stream.getloc(), new boost::posix_time::time_facet(date_str_format.c_str())));
    date_stream << value;

    //loginf << "UGA1 " << Time::toString(value) << " " << date_stream.str();

    return date_stream.str();
}

boost::posix_time::ptime fromDateString(std::string value)
{
    return boost::posix_time::ptime(boost::gregorian::from_string(value));
}

}

}
