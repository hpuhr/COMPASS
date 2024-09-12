#include "stringconv.h"
//#include "global.h"
#include "logger.h"
//#include "property.h"
#include "util/timeconv.h"

#include <openssl/sha.h>

#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string.hpp>

namespace Utils
{
namespace String
{

bool isNumber(const std::string& number_str)
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

std::string intToString(int number, int width, char c)
{
    std::ostringstream out;
    out << std::setfill(c) << std::setw(width) << number;
    return out.str();
}

std::string categoryString(unsigned int cat)
{
    std::ostringstream out;
    out << std::setfill('0') << std::setw(3) << cat;
    return out.str();
}

std::string doubleToStringPrecision(double number, unsigned int precision)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(precision) << number;
    return out.str();
}

std::string doubleToStringNoScientific(double number)
{
    std::ostringstream out;
    out << std::fixed << number;
    return out.str();
}

std::string percentToString(double number, unsigned int precision)
{
    std::ostringstream out;

    out << std::fixed << std::setprecision(precision) << number;

    return out.str();
}

std::string boolToString(bool value)
{
    return value ? "true" : "false";
}

unsigned int intFromOctalString(std::string number)
{
    return std::stoi(number, 0, 8);
}

unsigned int intFromHexString(std::string number)
{
    return std::stoi(number, 0, 16);
}

std::vector<std::string>& split(const std::string& s, char delim, std::vector<std::string>& elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string& s, char delim)
{
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

std::string timeStringFromDouble(double seconds, bool milliseconds)
{
    int hours, minutes;
    std::ostringstream out;

    if (seconds < 0)
    {
        out << "-";
        seconds *= -1;
    }

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

double timeFromString(std::string time_str, bool* ok)
{
    std::vector<std::string> chunks = split(time_str, ':');

    double time;

    if (chunks.size() != 3)
    {
        if (ok)
            *ok = false;

        return 0;
    }

    time = std::stod(chunks[0]) * 3600.0;

    if (time >= 0)
    {
        time += std::stod(chunks[1]) * 60.0;
        time += std::stod(chunks[2]);
    }
    else
    {
        time -= std::stod(chunks[1]) * 60.0;
        time -= std::stod(chunks[2]);
    }

    if (ok)
        *ok = true;

    return time;
}

std::string octStringFromInt(int number)
{
    std::ostringstream out;
    out << std::oct << number;
    return out.str();
}

std::string octStringFromInt(int number, int width, char c)
{
    std::ostringstream out;
    out << std::oct << std::setfill(c) << std::setw(width) << number;
    return out.str();
}

std::string hexStringFromInt(int number)
{
    std::ostringstream out;
    out << std::hex << number;
    return out.str();
}

std::string hexStringFromInt(int number, int width, char c)
{
    std::ostringstream out;
    out << std::hex << std::setfill(c) << std::setw(width) << number;
    return out.str();
}

int getAppendedInt(std::string text)
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
        loginf << "Util: getAppendedInt: no int found, returning 0";

    return ret;
}

unsigned int lineFromStr(const std::string& line_str)
{
    assert (line_str.size());
    unsigned int line = line_str.back() - '0';
    assert (line >= 1 && line <= 4);
    return line-1;
}

std::string lineStrFrom(unsigned int line)
{
    assert (line >= 0 && line <= 3);
    return "L" + std::to_string(line + 1);
}

int getLeadingInt(std::string text)
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

double doubleFromLatitudeString(std::string& latitude_str)
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

double doubleFromLongitudeString(std::string& longitude_str)
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

unsigned int hash(const std::string& str)
{
    unsigned char hash[SHA_DIGEST_LENGTH];  // == 20

    unsigned char str_data[str.size() + 1];  // needs copy, reinterpret cast failed in ub14
    std::copy(str.begin(), str.end(), str_data);

    SHA1(str_data, sizeof(str_data), hash);

    unsigned int tmp = 0;
    for (unsigned int cnt = 0; cnt < 4; ++cnt)
    {
        tmp <<= 8;
        tmp += (unsigned int)hash[SHA_DIGEST_LENGTH - 1 - cnt];
    }

    return tmp;
}

std::string getValueString(const std::string& value)
{
    return value;
}

typedef std::numeric_limits<double> double_limit;
typedef std::numeric_limits<float> float_limit;

std::string getValueString(const float& value)
{
    std::ostringstream out;
    out << std::setprecision(float_limit::max_digits10) << value;
    return out.str();
}

std::string getValueString(const double& value)
{
    std::ostringstream out;
    out << std::setprecision(double_limit::max_digits10) << value;
    return out.str();
}

std::string getValueString(const nlohmann::json& value)
{
    return value.dump();
}

std::string getValueString(const boost::posix_time::ptime& value)
{
    return Time::toString(value);
}

std::string compress(const std::vector<std::string>& values, char seperator)
{
    std::ostringstream ss;

    bool first_val = true;
    for (auto& val_it : values)
    {
        if (!first_val)
            ss << seperator;

        ss << val_it;

        first_val = false;
    }

    return ss.str();
}

std::string compress(const std::set<std::string>& values, char seperator)
{
    std::ostringstream ss;

    bool first_val = true;
    for (auto& val_it : values)
    {
        if (!first_val)
            ss << seperator;

        ss << val_it;

        first_val = false;
    }

    return ss.str();
}

bool hasEnding(std::string const& full_string, std::string const& ending)
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

bool replace(std::string& str, const std::string& from, const std::string& to)
{
   size_t start_pos = str.find(from);
   if (start_pos == std::string::npos)
       return false;
   str.replace(start_pos, from.length(), to);
   return true;
}

int compareVersions(const std::string& v1_str, const std::string& v2_str)
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

std::string latexString(std::string str)
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

    boost::replace_all(str, "^", R"(\^)");

    boost::replace_all(str, "<", R"(\textless)");
    boost::replace_all(str, ">", R"(\textgreater)");

    return str;
}

std::string ipFromString(const std::string& name)
{
    // string like "224.9.2.252:15040"

    std::vector<std::string> parts = split(name, ':');
    assert (parts.size() == 2);
    return parts.at(0);
}

unsigned int portFromString(const std::string& name)
{
    // string like "224.9.2.252:15040"

    std::vector<std::string> parts = split(name, ':');
    assert (parts.size() == 2);
    return stoi(parts.at(1));
}

std::string trim(const std::string& name)
{
   return boost::algorithm::trim_copy(name);
}

}
}
