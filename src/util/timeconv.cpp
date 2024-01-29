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
using namespace std;

const std::string QT_DATETIME_FORMAT {"yyyy-MM-dd hh:mm:ss.zzz"};

string str_format = "%Y-%m-%d %H:%M:%S.%f";
string date_str_format = "%Y-%m-%d";
string time_str_format = "%H:%M:%S.%f";
boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));

boost::posix_time::ptime fromString(const std::string& value)
{
    boost::posix_time::ptime timestamp;

    istringstream iss(value);

    // small f not working due to boost bug
    iss.imbue(std::locale(std::locale::classic(), new boost::posix_time::time_input_facet("%Y-%m-%d %H:%M:%S%F")));

    iss >> timestamp;
    return timestamp;
}

boost::posix_time::ptime fromString(const std::string& value, const std::string& facet)
{
    boost::posix_time::ptime timestamp;

    istringstream iss(value);

    // small f not working due to boost bug
    iss.imbue(std::locale(std::locale::classic(), new boost::posix_time::time_input_facet(facet)));

    iss >> timestamp;
    return timestamp;
}

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

string toString(boost::posix_time::ptime value, unsigned int partial_digits)
{
    ostringstream date_stream;

    date_stream.imbue(locale(date_stream.getloc(), new boost::posix_time::time_facet(str_format.c_str())));
    date_stream << value;

    string tmp = date_stream.str();

    if (partial_digits == 3) // only remove microsecs
        tmp.erase(tmp.length()-3); // remove microseconds since not supported by boost
    else if (partial_digits == 0) // remove all partials and point
        tmp.erase(tmp.length()-7);
    else
        tmp.erase(tmp.length()-3-(3-partial_digits));

    return tmp;
}

string toString(boost::posix_time::time_duration duration, unsigned int partial_digits)
{
    ostringstream date_stream;

    date_stream.imbue(locale(date_stream.getloc(), new boost::posix_time::time_facet(time_str_format.c_str())));
    date_stream << duration;

    string tmp = date_stream.str();

//    if (partial_digits == 3) // only remove microsecs
//        tmp.erase(tmp.length()-3); // remove microseconds since not supported by boost
//    else if (partial_digits == 0) // remove all partials and point
//        tmp.erase(tmp.length()-7);
//    else
//        tmp.erase(tmp.length()-3-(3-partial_digits));

    return tmp;

//    ostringstream os;
//    auto f = new boost::posix_time::time_facet(time_str_format.c_str());
//    f->time_duration_format(time_str_format.c_str());
//    os.imbue(locale(locale::classic(), f));
//    os << duration;
//    return os.str();
}


string toStringLong(unsigned long value)
{
   return toString(fromLong(value));
}

string toTimeString(boost::posix_time::ptime value)
{
    stringstream date_stream;

    date_stream.imbue(locale(date_stream.getloc(), new boost::posix_time::time_facet(time_str_format.c_str())));
    date_stream << value;

    //loginf << "UGA1 " << Time::toString(value) << " " << date_stream.str();

    string tmp = date_stream.str();
    tmp.erase(tmp.length()-3);

    return tmp;
}

string toDateString(boost::posix_time::ptime value)
{
    stringstream date_stream;

    date_stream.imbue(locale(date_stream.getloc(), new boost::posix_time::time_facet(date_str_format.c_str())));
    date_stream << value;

    //loginf << "UGA1 " << Time::toString(value) << " " << date_stream.str();

    return date_stream.str();
}

boost::posix_time::ptime fromDateString(string value)
{
    return boost::posix_time::ptime(boost::gregorian::from_string(value));
}

boost::posix_time::ptime currentUTCTime()
{
    return boost::posix_time::microsec_clock::universal_time();
}

boost::posix_time::time_duration partialSeconds(double seconds, bool ignore_full_seconds)
// can be negative but not exceed maxint seconds
{
    int full_seconds = (int) seconds;
    int partial_microseconds = 1000000 * (seconds - full_seconds);

    boost::posix_time::time_duration value;

    if (!ignore_full_seconds)
        value = boost::posix_time::seconds(full_seconds);

    value += boost::posix_time::microseconds(partial_microseconds);

    return value;
}

double partialSeconds(boost::posix_time::time_duration seconds)
{
    double value {0.0};

    //loginf << " UTA " << seconds.total_seconds() << " " << (double) seconds.total_microseconds() / 1000000.0;

    //value = seconds.total_seconds();
    value = (double) seconds.total_microseconds() / 1000000.0;

    return value;
}

}

}
