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
 * StringManipulation.h
 *
 *  Created on: Mar 3, 2014
 *      Author: sk
 */

#ifndef STRINGMANIPULATION_H_
#define STRINGMANIPULATION_H_

#include "Global.h"
#include "Property.h"
#include <vector>

namespace Utils
{

// TODO make this faster by using local streams

class String
{
public:
    String ();
    virtual ~String();

    /// @brief Returns string with parsed number
    static std::string intToString(int number);
    static std::string uIntToString(unsigned int number);
    /// @brief Returns string with parsed number, with leading zeros up to width
    static std::string intToString(int number, int width, char c);
    /// @brief Returns string with parsed number
    static std::string doubleToString(double number);
    static std::string doubleToStringPrecision (double number, unsigned int precision);
    static std::string doubleToStringNoScientific(double number);
    /// @brief Returns string with number in percent
    static std::string percentToString(double number);
    /// @brief Returns unsigned int from parsed octal string
    static unsigned int intFromOctalString (std::string number);
    static unsigned int intFromHexString (std::string number);
    /// @brief Returns int from parsed string
    static int intFromString (std::string number);
    static unsigned int uIntFromString (std::string number);
    /// @brief Returns double from parsed string
    static double doubleFromString (std::string number);
    /// @brief Returns container with strings, which are the contents of the supplied strings split by character delim
    static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
    /// @brief Returns container with strings, which are elems and the contents of the supplied strings split by character delim
    static std::vector<std::string> split(const std::string &s, char delim);
    /// @brief Returns time string from number in seconds
    static std::string timeStringFromDouble (double seconds);
    /// @brief Returns number from parsed time string
    static double timeFromString (std::string seconds);
    /// @brief Returns string with octal representation of decimal number
    static std::string octStringFromInt (int number);
    static std::string octStringFromInt (int number, int width, char c);
    static std::string hexStringFromInt (int number);
    static std::string hexStringFromInt (int number, int width, char c);
    /// @brief Returns appended number from a string
    static int getAppendedInt (std::string text);
    static int getLeadingInt (std::string text);
    //extern void multiplyString (std::string &text, PropertyDataType data_type, double factor);

    //extern std::string getPropertyValueString (void *data, PropertyDataType data_type);
    //extern std::string getPropertyValueHexString (void *data, PropertyDataType data_type);
    static std::string getHexString (void *data, unsigned int num_bytes);

    //extern bool isLargerAs (std::string org, std::string val, PropertyDataType type);
    /// @brief Returns equivalent PROPERTY_DATA_TYPE from SQL data type
    //extern PropertyDataType getDataTypeFromDB (std::string type);

    static double doubleFromLatitudeString(std::string &latitude_str);
    static double doubleFromLongitudeString(std::string &longitude_str);
};

template <typename T> std::string formatBinaryString (T val)
{
    int length = sizeof(T) * 8;
    std::string binary(length, '0');
    for (int i = 0; i < length; i++)
    {
        if ((val >> i) & 1)
            binary[length - 1 - i] = '1';
    }

    return binary;
}

}

#endif /* STRINGMANIPULATION_H_ */
