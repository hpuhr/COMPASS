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


#ifndef TIMECONV_H
#define TIMECONV_H

#include "boost/date_time/posix_time/posix_time.hpp"

#include <string>

namespace Utils
{
namespace Time
{
extern const std::string QT_DATETIME_FORMAT;

extern boost::posix_time::ptime fromString(const std::string& value);
extern boost::posix_time::ptime fromString(const std::string& value, const std::string& facet);
extern boost::posix_time::ptime fromLong(unsigned long value);
extern long toLong(boost::posix_time::ptime value);
extern long toLongQtUTC(boost::posix_time::ptime value);
extern long correctLongQtUTC(long t);
extern std::string toString(boost::posix_time::ptime value, unsigned int partial_digits=3);
extern std::string toString(boost::posix_time::time_duration duration, unsigned int partial_digits=3);
extern std::string toStringLong(unsigned long value);
extern std::string toTimeString(boost::posix_time::ptime value);
extern std::string toDateString(boost::posix_time::ptime value);
extern boost::posix_time::ptime fromDateString(std::string value);
extern boost::posix_time::ptime currentUTCTime();
// can be negative but not exceed maxint seconds
extern boost::posix_time::time_duration partialSeconds(double seconds, bool ignore_full_seconds=false);
extern double partialSeconds(boost::posix_time::time_duration seconds);


}  // namespace Time

}  // namespace Utils

#endif // TIMECONV_H
