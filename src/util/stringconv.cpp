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


std::string Utils::String::multiplyString (const std::string& value_str, double factor, PropertyDataType data_type)
{
    logdbg << "String: multiplyString: value " << value_str << " factor " << factor << " data_type " << Property::asString(data_type);

    std::string return_string;

    switch (data_type)
    {
    case PropertyDataType::BOOL:
    {
        bool value = std::stoul(value_str);
        value *= factor;
        return_string = std::to_string(value);
        break;
    }
    case PropertyDataType::CHAR:
    {
        char value = std::stoi(value_str);
        value *= factor;
        return_string = std::to_string(value);
        break;
    }
    case PropertyDataType::UCHAR:
    {
        unsigned char value = std::stoul(value_str);
        value *= factor;
        return_string = std::to_string(value);
        break;
    }
    case PropertyDataType::INT:
    {
        int value = std::stoi(value_str);
        value *= factor;
        return_string = std::to_string(value);
        break;
    }
    case PropertyDataType::UINT:
    {
        unsigned int value = std::stoul(value_str);
        value *= factor;
        return_string = std::to_string(value);
        break;
    }
    case PropertyDataType::LONGINT:
    {
        long value = std::stol(value_str);
        value *= factor;
        return_string = std::to_string(value);
        break;
    }
    case PropertyDataType::ULONGINT:
    {
        unsigned long value = std::stoul(value_str);
        value *= factor;
        return_string = std::to_string(value);
        break;
    }
    case PropertyDataType::FLOAT:
    {
        float value = std::stof(value_str);
        value *= factor;
        return_string = Utils::String::getValueString(value);
        break;
    }
    case PropertyDataType::DOUBLE:
    {
        double value = std::stod(value_str);
        value *= factor;
        return_string = Utils::String::getValueString(value);
        break;
    }
    case PropertyDataType::STRING:
        throw std::invalid_argument ("String: multiplyString: multiplication of string variable impossible");
    default:
        logerr  <<  "String: multiplyString:: unknown property type " << Property::asString(data_type);
        throw std::runtime_error ("String: multiplyString:: unknown property type " + Property::asString(data_type));
    }

    logdbg  <<  "String: multiplyString: return value " << return_string;

    return return_string;
}
