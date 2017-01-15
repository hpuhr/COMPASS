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

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/format.hpp>
#include <iomanip>
#include <boost/regex.hpp>

#include "stringconv.h"
#include "logger.h"

//std::map<DB_OBJECT_TYPE,std::string> DB_OBJECT_TYPE_STRINGS = boost::assign::map_list_of
//        (DBO_UNDEFINED,     "DBO_UNDEFINED"    )
//        (DBO_PLOTS,     "DBO_PLOTS"    )
//        (DBO_SYSTEM_TRACKS, "DBO_SYSTEM_TRACKS")
//        (DBO_ADS_B, "DBO_ADS_B")
//        (DBO_MLAT, "DBO_MLAT")
//        (DBO_REFERENCE_TRAJECTORIES, "DBO_REFERENCE_TRAJECTORIES")
//        (DBO_SENSOR_INFORMATION,    "DBO_SENSOR_INFORMATION"   );

//std::map<STRING_REPRESENTATION,std::string> STRING_REPRESENTATION_STRINGS = boost::assign::map_list_of
//        (R_STANDARD,     "R_STANDARD"    )
//        (R_TIME_SECONDS,     "R_TIME_SECONDS"    )
//        (R_OCTAL, "R_OCTAL")
//        (R_FLIGHT_LEVEL, "R_FLIGHT_LEVEL")
//        (R_SENSOR_NAME, "R_SENSOR_NAME")
//        (R_HEX, "R_HEX");

using namespace std;
using boost::algorithm::iequals;

namespace Utils
{

String::String()
{
}

String::~String()
{
}



std::string String::intToString(int number)
{
    std::stringstream ss;//create a stringstream
    ss << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}

std::string String::uIntToString(unsigned int number)
{
    std::stringstream ss;//create a stringstream
    ss << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}

std::string String::intToString(int number, int width, char c)
{
    std::stringstream ss;//create a stringstream
    ss << std::setfill(c) << std::setw(width) << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}

std::string String::doubleToString(double number)
{
    std::stringstream ss;//create a stringstream
    ss << boost::format("%g") % number;
    //ss << fixed << noshowpoint << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}

std::string String::doubleToStringPrecision(double number, unsigned int precision)
{
    std::stringstream ss;//create a stringstream
    ss << std::fixed << setprecision(precision) << number;
    //ss << fixed << noshowpoint << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}

std::string String::doubleToStringNoScientific(double number)
{
    std::stringstream ss;//create a stringstream
    //ss << boost::format("%g") % number;
    ss << fixed << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}

std::string String::percentToString(double number)
{
    std::stringstream ss;//create a stringstream
    ss << fixed << setprecision(2) << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}

unsigned int String::intFromOctalString (std::string number)
{
    unsigned int x = std::stoi( number, 0, 8 );
    logdbg << "Util: intFromOctalString: returning " << x << " from " << number;

    return x;
}

unsigned int String::intFromHexString (std::string number)
{
    unsigned int x = std::stoi( number, 0, 16 );

    logdbg << "Util: intFromHexString: returning " << x << " from " << number;

    return x;
}

int String::intFromString (std::string number)
{
    int x = boost::lexical_cast<int>(number);

    logdbg << "Util: intFromString: returning " << x << " from " << number;

    return x;
}

unsigned int String::uIntFromString (std::string number)
{
    unsigned int x = boost::lexical_cast<unsigned int>(number);

    logdbg << "Util: uIntFromString: returning " << x << " from " << number;

    return x;
}

double String::doubleFromString (std::string number)
{
    double x = boost::lexical_cast<double>(number);

    logdbg << "Util: doubleFromString: returning " << x << " from " << number;

    return x;
}

std::vector<std::string> &String::split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}


//bool isLargerAs (std::string org, std::string val, PropertyDataType type)
//{
//    logdbg << "Util: isLargerAs: org '" << org << "' val '" << val << "'";
//    if (type == P_TYPE_BOOL)
//    {
//        bool org_val = intFromString (org);
//        bool val_val = intFromString (val);
//        return val_val > org_val;
//    }
//    else if (type == P_TYPE_CHAR)
//    {
//        char org_val = intFromString (org);
//        char val_val = intFromString (val);
//        return val_val > org_val;
//    }
//    else if (type == P_TYPE_INT)
//    {
//        int org_val = intFromString (org);
//        int val_val = intFromString (val);
//        return val_val > org_val;
//    }
//    else if (type == P_TYPE_UCHAR)
//    {
//        unsigned char org_val = intFromString (org);
//        unsigned char val_val = intFromString (val);
//        return val_val > org_val;
//    }
//    else if (type == P_TYPE_UINT)
//    {
//        unsigned int org_val = intFromString (org);
//        unsigned int val_val = intFromString (val);
//        return val_val > org_val;
//    }
//    else if (type == P_TYPE_STRING)
//    {
//        return org.compare(val) > 0;
//    }
//    else if (type == P_TYPE_FLOAT)
//    {
//        float org_val = doubleFromString (org);
//        float val_val = doubleFromString (val);
//        return val_val > org_val;
//    }
//    else if (type == P_TYPE_DOUBLE)
//    {
//        double org_val = doubleFromString (org);
//        double val_val = doubleFromString (val);
//        return val_val > org_val;
//    }
//    else
//        throw std::runtime_error ("Util: createDataFromStringBuffer: unknown property type");
//}

std::vector<std::string> String::split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

std::string String::timeStringFromDouble (double seconds)
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

double String::timeFromString (std::string seconds)
{
    std::vector<std::string> chunks = split(seconds, ':');

    double time;

    if (chunks.size() != 3)
        throw std::runtime_error( "Util: timeFromString: wrong number of chunks" );

    time =  doubleFromString(chunks[0]) * 3600.0;
    time += doubleFromString(chunks[1]) * 60.0;
    time += doubleFromString(chunks[2]);

    logdbg << "Util: timeFromString: returning " << time << " from " << seconds;

    return time;
}

std::string String::octStringFromInt (int number)
{
    std::stringstream ss;
    ss <<  std::oct << number;
    return ss.str();
}

std::string String::octStringFromInt(int number, int width, char c)
{
    std::stringstream ss;//create a stringstream
    ss << std::oct << std::setfill(c) << std::setw(width) << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}


std::string String::hexStringFromInt (int number)
{
    std::stringstream ss;
    ss <<  std::hex << number;
    return ss.str();
}

std::string String::hexStringFromInt (int number, int width, char c)
{
    std::stringstream ss;
    ss <<  std::hex << std::setfill(c) << std::setw(width) << number;
    return ss.str();
}


int String::getAppendedInt (std::string text)
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

int String::getLeadingInt (std::string text)
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
//PropertyDataType getDataTypeFromDB (std::string type)
//{
//    boost::cmatch what;
//    //TODO hacked data types smallint, enum?, bigint, tinyblob, blob, mediumblob
//    if (boost::iequals (type,"boolean"))
//    {
//        return P_TYPE_BOOL;
//    }
//    else if (boost::iequals (type,"tinyint"))
//    {
//        return P_TYPE_CHAR;
//    }
//    else if (boost::iequals (type,"char"))
//    {
//        return P_TYPE_CHAR;
//    }
//    else if (boost::iequals (type,"smallint"))
//    {
//        return P_TYPE_INT;
//    }
//    else if (boost::iequals (type,"int") || boost::iequals (type,"integer"))
//    {
//        return P_TYPE_INT;
//    }
//    else if (boost::iequals (type,"mediumint"))
//    {
//        return P_TYPE_INT;
//    }
//    else if (boost::iequals (type,"bigint"))
//    {
//        return P_TYPE_INT;
//    }
//    else if (boost::iequals (type,"double"))
//    {
//        return P_TYPE_DOUBLE;
//    }
//    else if (boost::iequals (type,"enum"))
//    {
//        return P_TYPE_STRING;
//    }
//    else if (boost::iequals (type,"varchar") || boost::regex_match(type.c_str(), what, boost::regex ("VARCHAR.*")))
//    {
//        return P_TYPE_STRING;
//    }
//    else if (boost::iequals (type,"tinyblob"))
//    {
//        return P_TYPE_STRING;
//    }
//    else if (boost::iequals (type,"blob"))
//    {
//        return P_TYPE_STRING;
//    }
//    else if (boost::iequals (type,"mediumblob"))
//    {
//        return P_TYPE_STRING;
//    }
//    else if (boost::iequals (type,"binary"))
//    {
//        return P_TYPE_STRING;
//    }
//    else
//        throw std::runtime_error ("Util: getDataTypeFromDB: unknown db type '"+type+"'");

//}


//void multiplyString (std::string &text, PropertyDataType data_type, double factor)
//{
//    if (data_type == P_TYPE_CHAR)
//        text = intToString(intFromString(text)*factor);
//    else if (data_type == P_TYPE_INT)
//        text = intToString(intFromString(text)*factor);
//    else if (data_type == P_TYPE_UCHAR)
//        text = intToString(intFromString(text)*factor);
//    else if (data_type == P_TYPE_UINT)
//        text = intToString(intFromString(text)*factor);
//    else if (data_type == P_TYPE_FLOAT)
//        text = doubleToString(doubleFromString(text)*factor);
//    else if (data_type == P_TYPE_DOUBLE)
//        text = doubleToString(doubleFromString(text)*factor);
//    else
//    {
//        logerr << "Util: multiplyString: text '" << text << "' data_type " << Property::getDataTypeStr(data_type)
//               << " factor " << factor;
//        throw std::runtime_error ("Util: multiplyString: incorrect data type "+intToString (data_type));

//    }
//}

//std::string getPropertyValueString (void *data, PropertyDataType data_type)
//{
//    std::stringstream ss;

//    switch (data_type)
//    {
//    case P_TYPE_BOOL:
//        ss << *(bool *) data;
//        break;
//    case P_TYPE_UCHAR:
//        ss << (int) *(unsigned char *) data;
//        break;
//    case P_TYPE_CHAR:
//    case P_TYPE_INT:
//        ss << (int)*(char *) data;
//        break;
//    case P_TYPE_UINT:
//        ss << *(unsigned int *) data;
//        break;
//    case P_TYPE_STRING:
//        ss << *(std::string *) data;
//        break;
//    case P_TYPE_FLOAT:
//        ss << *(float *) data;
//        break;
//    case P_TYPE_DOUBLE:
//        ss << *(double *) data;
//        break;
//    case P_TYPE_POINTER:
//        ss << std::hex << *(void**) data;
//        break;
//    case P_TYPE_ULONGINT:
//        ss << *(unsigned long int *) data;
//        break;
//    case P_TYPE_LONGINT:
//        ss << *(long int *) data;
//        break;
//    default:
//        logerr  <<  "Util: getPropertyValueString: unknown property type";
//        throw std::runtime_error ("Util: getPropertyValueString: unknown property type");
//    }

//    return ss.str();
//}


//std::string getPropertyValueHexString (void *data, PropertyDataType data_type)
//{
//    std::stringstream ss;

//    ss << hex;

//    switch (data_type)
//    {
//    case P_TYPE_BOOL:
//        ss << *(bool *) data;
//        break;
//    case P_TYPE_UCHAR:
//        ss << (int) *(unsigned char *) data;
//        break;
//    case P_TYPE_CHAR:
//    case P_TYPE_INT:
//        ss << (int)*(char *) data;
//        break;
//    case P_TYPE_UINT:
//        ss << *(unsigned int *) data;
//        break;
//    case P_TYPE_STRING:
//        ss << *(std::string *) data;
//        break;
//    case P_TYPE_FLOAT:
//        ss << *(float *) data;
//        break;
//    case P_TYPE_DOUBLE:
//        ss << *(double *) data;
//        break;
//    case P_TYPE_POINTER:
//        ss << std::hex << *(void**) data;
//        break;
//    case P_TYPE_ULONGINT:
//        ss << std::hex << *(unsigned long int *) data;
//        break;
//    case P_TYPE_LONGINT:
//        ss << std::hex << *(long int *) data;
//        break;
//    default:
//        logerr  <<  "Util: getPropertyValueString: unknown property type";
//        throw std::runtime_error ("Util: getPropertyValueString: unknown property type");
//    }

//    return ss.str();
//}

std::string String::getHexString (void *data, unsigned int num_bytes)
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

double String::doubleFromLatitudeString(std::string &latitude_str)
{
    unsigned int len = latitude_str.size();
    assert (len == 12);
    char last_char = latitude_str.at(len-1);
    assert (last_char == 'N' || last_char == 'S');

    double x=0.0;

    x = doubleFromString(latitude_str.substr(0, 2));
    x += doubleFromString(latitude_str.substr(2, 2))/60.0;
    x += doubleFromString(latitude_str.substr(4, 7))/3600.0;

    if (last_char == 'S')
        x *= -1.0;

    return x;
}

double String::doubleFromLongitudeString(std::string &longitude_str)
{
    unsigned int len = longitude_str.size();
    assert (len == 13);
    char last_char = longitude_str.at(len-1);
    assert (last_char == 'E' || last_char == 'W');

    double x=0.0;

    x = doubleFromString(longitude_str.substr(0, 3));
    x += doubleFromString(longitude_str.substr(3, 2))/60.0;
    x += doubleFromString(longitude_str.substr(5, 7))/3600.0;

    if (last_char == 'W')
        x *= -1.0;

    return x;
}

}
