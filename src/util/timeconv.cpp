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

namespace Utils
{
namespace Time
{

std::string str_format = "%Y-%m-%d %H:%M:%S.%F";
boost::posix_time::time_input_facet str_facet (str_format.c_str());

boost::posix_time::ptime fromLong(unsigned long value)
{
    time_t seconds_since_epoch = floor(value / 1000);
    long milliseconds_since_second =
        floor((value - static_cast<long>(seconds_since_epoch)) * 1000);
    boost::posix_time::ptime result = boost::posix_time::from_time_t(seconds_since_epoch);
    boost::posix_time::time_duration fractional_seconds(0, 0, 0, milliseconds_since_second * 1000); // microseconds
    result += fractional_seconds;

    return result;
}

long toLong(boost::posix_time::ptime value)
{
    long result;

    const boost::posix_time::ptime epoch = boost::posix_time::from_time_t(0);
    boost::posix_time::time_duration duration = value - epoch;
    result = duration.total_milliseconds(); // duration.total_seconds() * 1000 +

    return result;
}

std::string toString(boost::posix_time::ptime value)
{
    std::stringstream date_stream;
    date_stream.imbue(std::locale(date_stream.getloc(), &str_facet));
    date_stream << value;

    return date_stream.str();
}

std::string toString(unsigned long value)
{
   return toString(fromLong(value));
}


}

}
