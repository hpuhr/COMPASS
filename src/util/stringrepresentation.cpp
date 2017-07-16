#include "stringrepresentation.h"

#include <boost/assign/list_of.hpp>

std::map<StringRepresentation,std::string> representation_2_string = boost::assign::map_list_of
        (StringRepresentation::STANDARD, "STANDARD")
        (StringRepresentation::SECONDS_TO_TIME, "SECONDS_TO_TIME")
        (StringRepresentation::DEC_TO_OCTAL, "DEC_TO_OCTAL")
        (StringRepresentation::DEC_TO_HEX, "DEC_TO_HEX")
        (StringRepresentation::FEET_TO_FLIGHTLEVEL, "FEET_TO_FLIGHTLEVEL");

std::map<std::string, StringRepresentation> string_2_representation = boost::assign::map_list_of
        ("STANDARD", StringRepresentation::STANDARD)
        ("SECONDS_TO_TIME", StringRepresentation::SECONDS_TO_TIME)
        ("DEC_TO_OCTAL", StringRepresentation::DEC_TO_OCTAL)
        ("DEC_TO_HEX", StringRepresentation::DEC_TO_HEX)
        ("FEET_TO_FLIGHTLEVEL", StringRepresentation::FEET_TO_FLIGHTLEVEL);
