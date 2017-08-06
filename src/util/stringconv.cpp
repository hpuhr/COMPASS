#include "stringconv.h"

#include <boost/assign/list_of.hpp>

std::map<Utils::String::Representation,std::string> Utils::String::representation_2_string = boost::assign::map_list_of
        (Utils::String::Representation::STANDARD, "STANDARD")
        (Utils::String::Representation::SECONDS_TO_TIME, "SECONDS_TO_TIME")
        (Utils::String::Representation::DEC_TO_OCTAL, "DEC_TO_OCTAL")
        (Utils::String::Representation::DEC_TO_HEX, "DEC_TO_HEX")
        (Utils::String::Representation::FEET_TO_FLIGHTLEVEL, "FEET_TO_FLIGHTLEVEL");

std::map<std::string, Utils::String::Representation> Utils::String::string_2_representation = boost::assign::map_list_of
        ("STANDARD", Utils::String::Representation::STANDARD)
        ("SECONDS_TO_TIME", Utils::String::Representation::SECONDS_TO_TIME)
        ("DEC_TO_OCTAL", Utils::String::Representation::DEC_TO_OCTAL)
        ("DEC_TO_HEX", Utils::String::Representation::DEC_TO_HEX)
        ("FEET_TO_FLIGHTLEVEL", Utils::String::Representation::FEET_TO_FLIGHTLEVEL);

Utils::String::Representation Utils::String::stringToRepresentation (const std::string &representation_str)
{
    assert (Utils::String::string_2_representation.count(representation_str) == 1);
    return Utils::String::string_2_representation.at(representation_str);
}

std::string Utils::String::representationToString (Utils::String::Representation representation)
{
    assert (Utils::String::representation_2_string.count(representation) == 1);
    return Utils::String::representation_2_string.at(representation);
}


