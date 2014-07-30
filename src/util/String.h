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
#include <vector>

namespace Utils
{

namespace String
{
/// @brief Returns string with parsed number
extern std::string intToString(int number);
extern std::string uIntToString(unsigned int number);
/// @brief Returns string with parsed number, with leading zeros up to width
extern std::string intToString(int number, int width, char c);
/// @brief Returns string with parsed number
extern std::string doubleToString(double number);
extern std::string doubleToStringPrecision (double number, unsigned int precision);
extern std::string doubleToStringNoScientific(double number);
/// @brief Returns string with number in percent
extern std::string percentToString(double number);
/// @brief Returns unsigned int from parsed octal string
extern unsigned int intFromOctalString (std::string number, bool* ok=0);
extern unsigned int intFromHexString (std::string number, bool* ok=0);
/// @brief Returns int from parsed string
extern int intFromString (std::string number, bool* ok=0);
extern unsigned int uIntFromString (std::string number, bool* ok=0);
/// @brief Returns double from parsed string
extern double doubleFromString (std::string number, bool* ok=0);
/// @brief Returns container with strings, which are the contents of the supplied strings split by character delim
extern std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
/// @brief Returns container with strings, which are elems and the contents of the supplied strings split by character delim
extern std::vector<std::string> split(const std::string &s, char delim);
/// @brief Returns time string from number in seconds
extern std::string timeStringFromDouble (double seconds);
/// @brief Returns number from parsed time string
extern double timeFromString (std::string seconds, bool* ok=0);
/// @brief Returns string with octal representation of decimal number
extern std::string octStringFromInt (int number);
extern std::string octStringFromInt (int number, int width, char c);
extern std::string hexStringFromInt (int number);
extern std::string hexStringFromInt (int number, int width, char c);
/// @brief Returns appended number from a string
extern int getAppendedInt (std::string text);
extern int getLeadingInt (std::string text);
extern void multiplyString (std::string &text, PROPERTY_DATA_TYPE data_type, double factor);

extern std::string getPropertyValueString (void *data, PROPERTY_DATA_TYPE data_type);
extern std::string getPropertyValueHexString (void *data, PROPERTY_DATA_TYPE data_type);
extern std::string getHexString (void *data, unsigned int num_bytes);

extern bool isLargerAs (std::string org, std::string val, PROPERTY_DATA_TYPE type);
/// @brief Returns equivalent PROPERTY_DATA_TYPE from SQL data type
extern PROPERTY_DATA_TYPE getDataTypeFromDB (std::string type);


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
}

#endif /* STRINGMANIPULATION_H_ */
