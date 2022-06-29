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

#include "system.h"

#include <fstream>
#include <limits>
#include <string>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <array>

#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>

#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/date_time/microsec_time_clock.hpp>

#include "logger.h"

const double megabyte = 1024 * 1024;
const double gigabyte = 1024 * 1024 * 1024;

namespace Utils
{
namespace System
{
float getFreeRAMinGB()
{
    //    struct sysinfo info;
    //    sysinfo (&info);

    //    return ((uint64_t) (info.freeram + info.bufferram) * info.mem_unit)/gigabyte;

    std::string token;
    std::ifstream file("/proc/meminfo");
    while (file >> token)
    {
        if (token == "MemAvailable:")
        {
            unsigned long mem;

            if (file >> mem)  // returns in kB
            {
                return mem / megabyte;
            }
            else
            {
                return 0;
            }
        }
        // ignore rest of the line
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    return 0;  // nothing found
}

std::string exec(const std::string& cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);

    if (!pipe)
    {
        logerr  << "Utils: exec: command '" << cmd << "' popen failed";
        throw std::runtime_error("Utils: exec: popen failed");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}

std::string getUserName()
{
    uid_t uid = geteuid ();
    struct passwd *pw = getpwuid (uid);
    if (pw)
    {
        return std::string(pw->pw_name);
    }
    return {};
}

double secondsSinceMidnightUTC ()
{
    using namespace boost::posix_time;

    auto p_time = microsec_clock::universal_time (); // UTC.
    return (p_time.time_of_day().total_milliseconds() / 1000.0);
}

}
}
