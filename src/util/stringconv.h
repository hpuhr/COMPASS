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

#ifndef STRINGMANIPULATION_H_
#define STRINGMANIPULATION_H_

#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <iomanip>
#include <map>
#include <vector>

#include "global.h"
#include "logger.h"
#include "property.h"

namespace Utils
{
namespace String
{
inline bool isNumber(const std::string& number_str)
{
    try
    {
        std::stoi(number_str);
    }
    catch (std::invalid_argument& e)
    {
        return false;
    }
    return true;
}

inline std::string intToString(int number, int width, char c)
{
    std::ostringstream out;
    out << std::setfill(c) << std::setw(width) << number;
    return out.str();
}

inline std::string categoryString(unsigned int cat)
{
    std::ostringstream out;
    out << std::setfill('0') << std::setw(3) << cat;
    return out.str();
}

inline std::string doubleToStringPrecision(double number, unsigned int precision)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(precision) << number;
    return out.str();
}

inline std::string doubleToStringNoScientific(double number)
{
    std::ostringstream out;
    out << std::fixed << number;
    return out.str();
}

inline std::string percentToString(double number)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << number;
    return out.str();
}

inline unsigned int intFromOctalString(std::string number) { return std::stoi(number, 0, 8); }

inline unsigned int intFromHexString(std::string number) { return std::stoi(number, 0, 16); }

inline std::vector<std::string>& split(const std::string& s, char delim,
                                       std::vector<std::string>& elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}

inline std::vector<std::string> split(const std::string& s, char delim)
{
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

inline std::string timeStringFromDouble(double seconds, bool milliseconds = true)
{
    int hours, minutes;
    std::ostringstream out;

    hours = static_cast<int>(seconds / 3600.0);
    minutes = static_cast<int>(static_cast<double>(static_cast<int>(seconds) % 3600) / 60.0);
    seconds = seconds - hours * 3600.0 - minutes * 60.0;

    out << std::fixed << std::setw(2) << std::setfill('0') << hours << ":" << std::setw(2)
        << std::setfill('0') << minutes << ":";

    if (milliseconds)
        out << std::setw(6) << std::setfill('0') << std::setprecision(3) << seconds;
    else
        out << std::setw(2) << std::setfill('0') << std::setprecision(0)
            << static_cast<int>(seconds);

    return out.str();
}

inline double timeFromString(std::string time_str)
{
    std::vector<std::string> chunks = split(time_str, ':');

    double time;

    if (chunks.size() != 3)
        throw std::invalid_argument("Util: timeFromString: wrong number of chunks");

    time = std::stod(chunks[0]) * 3600.0;
    time += std::stod(chunks[1]) * 60.0;
    time += std::stod(chunks[2]);

    return time;
}

inline std::string octStringFromInt(int number)
{
    std::ostringstream out;
    out << std::oct << number;
    return out.str();
}

inline std::string octStringFromInt(int number, int width, char c)
{
    std::ostringstream out;
    out << std::oct << std::setfill(c) << std::setw(width) << number;
    return out.str();
}

inline std::string hexStringFromInt(int number)
{
    std::ostringstream out;
    out << std::hex << number;
    return out.str();
}

inline std::string hexStringFromInt(int number, int width, char c)
{
    std::ostringstream out;
    out << std::hex << std::setfill(c) << std::setw(width) << number;
    return out.str();
}

inline int getAppendedInt(std::string text)
{
    int ret = 0;
    boost::regex re("[0-9]+");
    boost::sregex_token_iterator i(text.begin(), text.end(), re, 0);
    boost::sregex_token_iterator j;

    unsigned count = 0;
    while (i != j)
    {
        ret = std::stoi(*i++);
        count++;
    }

    if (count == 0)
        throw std::runtime_error("Util: getAppendedInt: no int found");

    return ret;
}

inline int getLeadingInt(std::string text)
{
    boost::regex re("[0-9]+");
    boost::sregex_token_iterator i(text.begin(), text.end(), re, 0);
    boost::sregex_token_iterator j;

    if (i != j)
    {
        return std::stoi(*i++);
    }
    else
        throw std::runtime_error("Util: getLeadingInt: no int found");
}

inline double doubleFromLatitudeString(std::string& latitude_str)
{
    unsigned int len = latitude_str.size();
    assert(len == 12);
    char last_char = latitude_str.at(len - 1);
    assert(last_char == 'N' || last_char == 'S');

    double x = 0.0;

    x = std::stod(latitude_str.substr(0, 2));
    x += std::stod(latitude_str.substr(2, 2)) / 60.0;
    x += std::stod(latitude_str.substr(4, 7)) / 3600.0;

    if (last_char == 'S')
        x *= -1.0;

    return x;
}

inline double doubleFromLongitudeString(std::string& longitude_str)
{
    unsigned int len = longitude_str.size();
    assert(len == 13);
    char last_char = longitude_str.at(len - 1);
    assert(last_char == 'E' || last_char == 'W');

    double x = 0.0;

    x = std::stod(longitude_str.substr(0, 3));
    x += std::stod(longitude_str.substr(3, 2)) / 60.0;
    x += std::stod(longitude_str.substr(5, 7)) / 3600.0;

    if (last_char == 'W')
        x *= -1.0;

    return x;
}

inline std::string getValueString(const std::string& value) { return value; }

typedef std::numeric_limits<double> double_limit;
typedef std::numeric_limits<float> float_limit;

inline std::string getValueString(const float& value)
{
    std::ostringstream out;
    out << std::setprecision(float_limit::max_digits10) << value;
    return out.str();
}

inline std::string getValueString(const double& value)
{
    std::ostringstream out;
    out << std::setprecision(double_limit::max_digits10) << value;
    return out.str();
}

template <typename T>
std::string getValueString(T value)
{
    return std::to_string(value);
}

inline bool hasEnding(std::string const& full_string, std::string const& ending)
{
    if (full_string.length() >= ending.length())
    {
        return (0 == full_string.compare(full_string.length() - ending.length(), ending.length(),
                                         ending));
    }
    else
    {
        return false;
    }
}

inline bool replace(std::string& str, const std::string& from, const std::string& to)
{
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

// 0 if same, -1 if v1 > v2, 1 if v1 < v2
inline int compareVersions(const std::string& v1_str, const std::string& v2_str)
{
    std::vector<std::string> v1_parts = split(v1_str, '.');
    std::vector<std::string> v2_parts = split(v2_str, '.');

    assert(v1_parts.size() == v2_parts.size());

    int v1_part;
    int v2_part;
    for (unsigned int cnt = 0; cnt < v1_parts.size(); ++cnt)
    {
        v1_part = std::stoi(v1_parts.at(cnt));
        v2_part = std::stoi(v2_parts.at(cnt));

        if (v1_part > v2_part)  // -1 if v1 > v2
            return -1;

        if (v1_part < v2_part)  // 1 if v1 < v2
            return 1;

        //  if (v1_part == v2_part)
        //      continue;
    }

    return 0;  // same
}

inline std::string latexString(std::string str)
{
//    \textbackslash 	n/a
    boost::replace_all(str, R"(\)", R"(\textbackslash)");
//    \% 	%
    boost::replace_all(str, "%", R"(\%)");
//    \$ 	$
    boost::replace_all(str, "$", R"(\$)");
//    \{ 	{
    boost::replace_all(str, "{", R"(\{)");
//    \_ 	_
    boost::replace_all(str, "_", R"(\_)");
//    \# 	#
    boost::replace_all(str, "#", R"(\#)");
//    \& 	&
    boost::replace_all(str, "&", R"(\&)");
//    \} 	}
    boost::replace_all(str, "}", R"(\})");

    return str;
}

}  // namespace String

// template <typename T> std::string formatBinaryString (T val)
//{
//    int length = sizeof(T) * 8;
//    std::string binary(length, '0');
//    for (int i = 0; i < length; i++)
//    {
//        if ((val >> i) & 1)
//            binary[length - 1 - i] = '1';
//    }

//    return binary;
//}

}  // namespace Utils

#endif /* STRINGMANIPULATION_H_ */
