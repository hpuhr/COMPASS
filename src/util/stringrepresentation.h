#ifndef STRINGREPRESENTATION_H
#define STRINGREPRESENTATION_H

#include <map>

enum class StringRepresentation { STANDARD, SECONDS_TO_TIME, DEC_TO_OCTAL, DEC_TO_HEX, FEET_TO_FLIGHTLEVEL};

/// Special string representation types. Slighly outdated
//enum STRING_REPRESENTATION { R_STANDARD, R_TIME_SECONDS, R_OCTAL, R_FLIGHT_LEVEL, R_SENSOR_NAME, R_HEX };
/// Mappings for STRING_REPRESENTATION to strings, defined in util.cpp
//extern std::map<StringRepresentation,std::string> STRING_REPRESENTATION_STRINGS;

extern std::map<StringRepresentation,std::string> representation_2_string;
extern std::map<std::string, StringRepresentation> string_2_representation;

#endif // STRINGREPRESENTATION_H
