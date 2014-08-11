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
 * StringManipulation.cpp
 *
 *  Created on: Mar 3, 2014
 *      Author: sk
 */

#include "String.h"
#include "Logger.h"
#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/format.hpp>
#include <iomanip>
#include <boost/regex.hpp>

std::map<DB_OBJECT_TYPE,std::string> DB_OBJECT_TYPE_STRINGS = boost::assign::map_list_of
(DBO_UNDEFINED,     "DBO_UNDEFINED"    )
(DBO_PLOTS,     "DBO_PLOTS"    )
(DBO_SYSTEM_TRACKS, "DBO_SYSTEM_TRACKS")
(DBO_ADS_B, "DBO_ADS_B")
(DBO_MLAT, "DBO_MLAT")
(DBO_REFERENCE_TRAJECTORIES, "DBO_REFERENCE_TRAJECTORIES")
(DBO_SENSOR_INFORMATION,    "DBO_SENSOR_INFORMATION"   );

std::map<PROPERTY_DATA_TYPE,std::string> PROPERTY_DATA_TYPE_STRINGS = boost::assign::map_list_of
(P_TYPE_BOOL,     "P_TYPE_BOOL"    )
(P_TYPE_CHAR,     "P_TYPE_CHAR"    )
(P_TYPE_INT, "P_TYPE_INT")
(P_TYPE_UCHAR, "P_TYPE_UCHAR")
(P_TYPE_UINT, "P_TYPE_UINT")
(P_TYPE_STRING, "P_TYPE_STRING")
(P_TYPE_FLOAT,    "P_TYPE_FLOAT"   )
(P_TYPE_DOUBLE,    "P_TYPE_DOUBLE"   )
(P_TYPE_POINTER,    "P_TYPE_POINTER"   )
(P_TYPE_LONGINT, "P_TYPE_LONGINT")
(P_TYPE_ULONGINT, "P_TYPE_ULONGINT");

std::map<STRING_REPRESENTATION,std::string> STRING_REPRESENTATION_STRINGS = boost::assign::map_list_of
(R_STANDARD,     "R_STANDARD"    )
(R_TIME_SECONDS,     "R_TIME_SECONDS"    )
(R_OCTAL, "R_OCTAL")
(R_FLIGHT_LEVEL, "R_FLIGHT_LEVEL")
(R_SENSOR_NAME, "R_SENSOR_NAME")
(R_HEX, "R_HEX");

using namespace std;
using boost::algorithm::iequals;

namespace Utils
{
namespace String
{
std::string intToString(int number)
{
    std::stringstream ss;//create a stringstream
    ss << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}

std::string uIntToString(unsigned int number)
{
    std::stringstream ss;//create a stringstream
    ss << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}

std::string intToString(int number, int width, char c)
{
    std::stringstream ss;//create a stringstream
    ss << std::setfill(c) << std::setw(width) << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}

std::string doubleToString(double number)
{
    std::stringstream ss;//create a stringstream
    ss << boost::format("%g") % number;
    //ss << fixed << noshowpoint << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}

std::string doubleToStringPrecision(double number, unsigned int precision)
{
    std::stringstream ss;//create a stringstream
    ss << std::fixed << setprecision(precision) << number;
    //ss << fixed << noshowpoint << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}

std::string doubleToStringNoScientific(double number)
{
    std::stringstream ss;//create a stringstream
    //ss << boost::format("%g") % number;
    ss << fixed << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}

std::string percentToString(double number)
{
    std::stringstream ss;//create a stringstream
    ss << fixed << setprecision(2) << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}

unsigned int intFromOctalString (std::string number, bool* ok)
{
    unsigned int x=0;
    std::stringstream ss;
    ss << std::oct << number;
    bool ssok = ( ss >> x );
    if( ok )
        *ok = ssok;

    if( !ssok )
        logdbg  << "Util: intFromOctalString: Bad operation: '" << number << "'";

    return x;
}

unsigned int intFromHexString (std::string number, bool* ok)
{
    unsigned int x=0;
    std::stringstream ss;
    ss << std::hex << number;
    bool ssok = ( ss >> x );
    if( ok )
        *ok = ssok;

    if( !ssok )
        logdbg  << "Util: intFromHexString: Bad operation: '" << number << "'";

    loginf << "Util: intFromHexString: returning " << x << " from " << number;

    return x;
}

int intFromString (std::string number, bool* ok)
{
    int x=0;
    std::stringstream ss;
    ss <<  number;
    bool ssok = ( ss >> x );
    if( ok )
        *ok = ssok;

    if( !ssok )
        logdbg  << "Util: intFromString: Bad operation: '" << number << "'";

    return x;
}

unsigned int uIntFromString (std::string number, bool* ok)
{
    unsigned int x=0;
    std::stringstream ss;
    ss <<  number;
    bool ssok = ( ss >> x );
    if( ok )
        *ok = ssok;

    if( !ssok )
        logdbg  << "Util: intFromString: Bad operation: '" << number << "'";

    return x;
}

double doubleFromString (std::string number, bool* ok)
{
    double x=0.0;
    std::stringstream ss;
    ss << fixed << number;
    bool ssok = ( ss >> x );
    if( ok )
        *ok = ssok;

    if( !ssok )
        logdbg  << "Util: doubleFromString: Bad operation: '" << number << "'";

    return x;
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}


bool isLargerAs (std::string org, std::string val, PROPERTY_DATA_TYPE type)
{
    logdbg << "Util: isLargerAs: org '" << org << "' val '" << val << "'";
    if (type == P_TYPE_BOOL)
    {
        bool org_val = intFromString (org);
        bool val_val = intFromString (val);
        return val_val > org_val;
    }
    else if (type == P_TYPE_CHAR)
    {
        char org_val = intFromString (org);
        char val_val = intFromString (val);
        return val_val > org_val;
    }
    else if (type == P_TYPE_INT)
    {
        int org_val = intFromString (org);
        int val_val = intFromString (val);
        return val_val > org_val;
    }
    else if (type == P_TYPE_UCHAR)
    {
        unsigned char org_val = intFromString (org);
        unsigned char val_val = intFromString (val);
        return val_val > org_val;
    }
    else if (type == P_TYPE_UINT)
    {
        unsigned int org_val = intFromString (org);
        unsigned int val_val = intFromString (val);
        return val_val > org_val;
    }
    else if (type == P_TYPE_STRING)
    {
        return org.compare(val) > 0;
    }
    else if (type == P_TYPE_FLOAT)
    {
        float org_val = doubleFromString (org);
        float val_val = doubleFromString (val);
        return val_val > org_val;
    }
    else if (type == P_TYPE_DOUBLE)
    {
        double org_val = doubleFromString (org);
        double val_val = doubleFromString (val);
        return val_val > org_val;
    }
    else
        throw std::runtime_error ("Util: createDataFromStringBuffer: unknown property type");
}

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

std::string timeStringFromDouble (double seconds)
{
    int hours, minutes;
    std::string text;
    std::ostringstream ss;

    hours = (int) (seconds / 3600.0);
    minutes = (int) ((double)((int)seconds%3600)/60.0);
    seconds = seconds-hours*3600.0-minutes*60.0;

    //ss << setw(2) << setfill('0') << hours << ":" << setw(2) << setfill('0') << minutes << ":" << setw(2) << setfill('0') << setprecision(2) << seconds;
    ss << fixed << setw(2) << setfill('0') << hours << ":" << setw(2) << setfill('0') << minutes << ":" << setw(5) << setfill('0') << setprecision(2) << seconds;
    return ss.str();
}

double timeFromString (std::string seconds, bool* ok)
{
    std::vector<std::string> chunks = split(seconds, ':');

    double time;

    if (chunks.size() != 3)
        throw std::runtime_error( "Util: timeFromString: wrong number of chunks" );

    bool ok0, ok1, ok2;
    time =  doubleFromString(chunks[0], &ok0) * 3600.0;
    time += doubleFromString(chunks[1], &ok1) * 60.0;
    time += doubleFromString(chunks[2], &ok2);

    if( ok )
    {
        *ok = ok0 && ok1 && ok2;
        if (!*ok)
            logwrn << "Util: timeFromString: time conversion failed 0: " << chunks[0] << " " << ok0
            << " 1: " << chunks[1] << " " << ok1 << " 2: " << chunks[2] << " " << ok2;
    }
    return time;
}

std::string octStringFromInt (int number)
{
    std::stringstream ss;
    ss <<  std::oct << number;
    return ss.str();
}

std::string octStringFromInt(int number, int width, char c)
{
    std::stringstream ss;//create a stringstream
    ss << std::oct << std::setfill(c) << std::setw(width) << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}


std::string hexStringFromInt (int number)
{
    std::stringstream ss;
    ss <<  std::hex << number;
    return ss.str();
}

std::string hexStringFromInt (int number, int width, char c)
{
    std::stringstream ss;
    ss <<  std::hex << std::setfill(c) << std::setw(width) << number;
    return ss.str();
}


int getAppendedInt (std::string text)
{
    int ret=0;
    boost::regex re("[0-9]+");
    boost::sregex_token_iterator i(text.begin(), text.end(), re, 0);
    boost::sregex_token_iterator j;

    unsigned count = 0;
    while(i != j)
    {
        ret = intFromString(*i++);
        count++;
    }

    if (count == 0)
        throw std::runtime_error ("Util: getAppendedInt: no int found");

    return ret;
}

int getLeadingInt (std::string text)
{
    boost::regex re("[0-9]+");
    boost::sregex_token_iterator i(text.begin(), text.end(), re, 0);
    boost::sregex_token_iterator j;

    if (i != j)
    {
        return intFromString(*i++);
    }
    else
        throw std::runtime_error ("Util: getLeadingInt: no int found");
}

//P_TYPE_BOOL=0, P_TYPE_CHAR, P_TYPE_INT, P_TYPE_UCHAR, P_TYPE_UINT, P_TYPE_STRING, P_TYPE_FLOAT, P_TYPE_DOUBLE, P_TYPE_SENTINEL
PROPERTY_DATA_TYPE getDataTypeFromDB (std::string type)
{
    boost::cmatch what;
    //TODO hacked data types smallint, enum?, bigint, tinyblob, blob, mediumblob
    if (boost::iequals (type,"boolean"))
    {
        return P_TYPE_BOOL;
    }
    else if (boost::iequals (type,"tinyint"))
    {
        return P_TYPE_CHAR;
    }
    else if (boost::iequals (type,"char"))
    {
        return P_TYPE_CHAR;
    }
    else if (boost::iequals (type,"smallint"))
    {
        return P_TYPE_INT;
    }
    else if (boost::iequals (type,"int") || boost::iequals (type,"integer"))
    {
        return P_TYPE_INT;
    }
    else if (boost::iequals (type,"mediumint"))
    {
        return P_TYPE_INT;
    }
    else if (boost::iequals (type,"bigint"))
    {
        return P_TYPE_INT;
    }
    else if (boost::iequals (type,"double"))
    {
        return P_TYPE_DOUBLE;
    }
    else if (boost::iequals (type,"enum"))
    {
        return P_TYPE_STRING;
    }
    else if (boost::iequals (type,"varchar") || boost::regex_match(type.c_str(), what, boost::regex ("VARCHAR.*")))
    {
        return P_TYPE_STRING;
    }
    else if (boost::iequals (type,"tinyblob"))
    {
        return P_TYPE_STRING;
    }
    else if (boost::iequals (type,"blob"))
    {
        return P_TYPE_STRING;
    }
    else if (boost::iequals (type,"mediumblob"))
    {
        return P_TYPE_STRING;
    }
    else if (boost::iequals (type,"binary"))
    {
        return P_TYPE_STRING;
    }
    else
        throw std::runtime_error ("Util: getDataTypeFromDB: unknown db type '"+type+"'");

}


void multiplyString (std::string &text, PROPERTY_DATA_TYPE data_type, double factor)
{
    if (data_type == P_TYPE_CHAR)
        text = intToString(intFromString(text)*factor);
    else if (data_type == P_TYPE_INT)
        text = intToString(intFromString(text)*factor);
    else if (data_type == P_TYPE_UCHAR)
        text = intToString(intFromString(text)*factor);
    else if (data_type == P_TYPE_UINT)
        text = intToString(intFromString(text)*factor);
    else if (data_type == P_TYPE_FLOAT)
        text = doubleToString(doubleFromString(text)*factor);
    else if (data_type == P_TYPE_DOUBLE)
        text = doubleToString(doubleFromString(text)*factor);
    else
    {
        logerr << "Util: multiplyString: text '" << text << "' data_type " << PROPERTY_DATA_TYPE_STRINGS[data_type]
               << " factor " << factor;
        throw std::runtime_error ("Util: multiplyString: incorrect data type "+intToString (data_type));

    }
}

std::string getPropertyValueString (void *data, PROPERTY_DATA_TYPE data_type)
{
    std::stringstream ss;

    switch (data_type)
    {
    case P_TYPE_BOOL:
        ss << *(bool *) data;
        break;
    case P_TYPE_UCHAR:
        ss << (int) *(unsigned char *) data;
        break;
    case P_TYPE_CHAR:
    case P_TYPE_INT:
        ss << (int)*(char *) data;
        break;
    case P_TYPE_UINT:
        ss << *(unsigned int *) data;
        break;
    case P_TYPE_STRING:
        ss << *(std::string *) data;
        break;
    case P_TYPE_FLOAT:
        ss << *(float *) data;
        break;
    case P_TYPE_DOUBLE:
        ss << *(double *) data;
        break;
    case P_TYPE_POINTER:
        ss << std::hex << *(void**) data;
        break;
    case P_TYPE_ULONGINT:
        ss << *(unsigned long int *) data;
        break;
    case P_TYPE_LONGINT:
        ss << *(long int *) data;
        break;
    default:
        logerr  <<  "Util: getPropertyValueString: unknown property type";
        throw std::runtime_error ("Util: getPropertyValueString: unknown property type");
    }

    return ss.str();
}


std::string getPropertyValueHexString (void *data, PROPERTY_DATA_TYPE data_type)
{
    std::stringstream ss;

    ss << hex;

    switch (data_type)
    {
    case P_TYPE_BOOL:
        ss << *(bool *) data;
        break;
    case P_TYPE_UCHAR:
        ss << (int) *(unsigned char *) data;
        break;
    case P_TYPE_CHAR:
    case P_TYPE_INT:
        ss << (int)*(char *) data;
        break;
    case P_TYPE_UINT:
        ss << *(unsigned int *) data;
        break;
    case P_TYPE_STRING:
        ss << *(std::string *) data;
        break;
    case P_TYPE_FLOAT:
        ss << *(float *) data;
        break;
    case P_TYPE_DOUBLE:
        ss << *(double *) data;
        break;
    case P_TYPE_POINTER:
        ss << std::hex << *(void**) data;
        break;
    case P_TYPE_ULONGINT:
        ss << std::hex << *(unsigned long int *) data;
        break;
    case P_TYPE_LONGINT:
        ss << std::hex << *(long int *) data;
        break;
    default:
        logerr  <<  "Util: getPropertyValueString: unknown property type";
        throw std::runtime_error ("Util: getPropertyValueString: unknown property type");
    }

    return ss.str();
}

std::string getHexString (void *data, unsigned int num_bytes)
{
    assert (num_bytes > 0);
    std::stringstream ss;

    for (unsigned int cnt=0; cnt < num_bytes; cnt++)
    {
        if (cnt != 0)
            ss << " ";

        ss << hex << fixed << setw(2) << setfill('0') << (int)*(((unsigned char*) data)+cnt);
    }


    return ss.str();
}

double doubleFromLatitudeString(std::string &latitude_str, bool &ok)
{
    ok = true;
    unsigned int len = latitude_str.size();
    assert (len == 12);
    char last_char = latitude_str.at(len-1);
    assert (last_char == 'N' || last_char == 'S');

    double x=0.0;
    double tmp=0.0;
    std::stringstream ss;

    ss << latitude_str.substr(0, 2);
    ok = (ss >> tmp);

    if(!ok)
    {
        logwrn  << "Util: doubleFromLatitudeString: bad operation: '" << latitude_str << "'";
        return 0.0;
    }
    x = tmp;

    ss.str(std::string());
    ss << latitude_str.substr(2, 2);
    ok = (ss >> tmp);

    if(!ok)
    {
        logwrn  << "Util: doubleFromLatitudeString: bad operation: '" << latitude_str << "'";
        return 0.0;
    }
    x += tmp/60.0;

    ss.str(std::string());
    ss << latitude_str.substr(4, 7);
    ok = (ss >> tmp);

    if(!ok)
    {
        logwrn  << "Util: doubleFromLatitudeString: bad operation: '" << latitude_str << "'";
        return 0.0;
    }
    x += tmp/3600.0;

    if (last_char == 'S')
        x *= -1.0;

    return x;
}

double doubleFromLongitudeString(std::string &longitude_str, bool &ok)
{
    ok = true;
    unsigned int len = longitude_str.size();
    assert (len == 12);
    char last_char = longitude_str.at(len-1);
    assert (last_char == 'E' || last_char == 'W');

    double x=0.0;
    double tmp=0.0;
    std::stringstream ss;

    ss << longitude_str.substr(0, 2);
    ok = (ss >> tmp);

    if(!ok)
    {
        logwrn  << "Util: doubleFromLongitudeString: bad operation: '" << longitude_str << "'";
        return 0.0;
    }
    x = tmp;

    ss.str(std::string());
    ss << longitude_str.substr(2, 2);
    ok = (ss >> tmp);

    if(!ok)
    {
        logwrn  << "Util: doubleFromLongitudeString: bad operation: '" << longitude_str << "'";
        return 0.0;
    }
    x += tmp/60.0;

    ss.str(std::string());
    ss << longitude_str.substr(4, 7);
    ok = (ss >> tmp);

    if(!ok)
    {
        logwrn  << "Util: doubleFromLongitudeString: bad operation: '" << longitude_str << "'";
        return 0.0;
    }
    x += tmp/3600.0;

    if (last_char == 'W')
        x *= -1.0;

    return x;
}


}
}
