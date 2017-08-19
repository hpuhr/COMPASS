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

    if (value_str == NULL_STRING)
        return value_str;

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

const std::string& Utils::String::getLargerValueString (const std::string& value_a_str, const std::string& value_b_str, PropertyDataType data_type)
{
    logdbg << "String: getLargerValueString: value a " << value_a_str << " b " << value_b_str << " data_type " << Property::asString(data_type);

    if (value_a_str == NULL_STRING || value_b_str == NULL_STRING)
    {
        if (value_a_str != NULL_STRING)
            return value_a_str;
        if (value_b_str != NULL_STRING)
            return value_b_str;
        return NULL_STRING;
    }

    switch (data_type)
    {
    case PropertyDataType::BOOL:
    case PropertyDataType::UCHAR:
    case PropertyDataType::UINT:
    case PropertyDataType::ULONGINT:
    {
        if (std::stoul(value_a_str) > std::stoul(value_b_str))
            return value_a_str;
        else
            return value_b_str;
    }
    case PropertyDataType::CHAR:
    case PropertyDataType::INT:
    {
        if (std::stoi(value_a_str) > std::stoi(value_b_str))
            return value_a_str;
        else
            return value_b_str;
    }
    case PropertyDataType::LONGINT:
    {
        if (std::stol(value_a_str) > std::stol(value_b_str))
            return value_a_str;
        else
            return value_b_str;
    }
    case PropertyDataType::FLOAT:
    {
        if (std::stof(value_a_str) > std::stof(value_b_str))
            return value_a_str;
        else
            return value_b_str;
    }
    case PropertyDataType::DOUBLE:
    {
        if (std::stod(value_a_str) > std::stod(value_b_str))
            return value_a_str;
        else
            return value_b_str;
    }
    case PropertyDataType::STRING:
        throw std::invalid_argument ("String: getLargerValueString: operation on string variable impossible");
    default:
        logerr  <<  "String: getLargerValueString:: unknown property type " << Property::asString(data_type);
        throw std::runtime_error ("String: getLargerValueString:: unknown property type " + Property::asString(data_type));
    }
}

const std::string& Utils::String::getSmallerValueString (const std::string& value_a_str, const std::string& value_b_str, PropertyDataType data_type)
{
    logdbg << "String: getSmallerValueString: value a " << value_a_str << " b " << value_b_str << " data_type " << Property::asString(data_type);

    if (value_a_str == NULL_STRING || value_b_str == NULL_STRING)
    {
        if (value_a_str != NULL_STRING)
            return value_a_str;
        if (value_b_str != NULL_STRING)
            return value_b_str;
        return NULL_STRING;
    }


    switch (data_type)
    {
    case PropertyDataType::BOOL:
    case PropertyDataType::UCHAR:
    case PropertyDataType::UINT:
    case PropertyDataType::ULONGINT:
    {
        if (std::stoul(value_a_str) < std::stoul(value_b_str))
            return value_a_str;
        else
            return value_b_str;
    }
    case PropertyDataType::CHAR:
    case PropertyDataType::INT:
    {
        if (std::stoi(value_a_str) < std::stoi(value_b_str))
            return value_a_str;
        else
            return value_b_str;
    }
    case PropertyDataType::LONGINT:
    {
        if (std::stol(value_a_str) < std::stol(value_b_str))
            return value_a_str;
        else
            return value_b_str;
    }
    case PropertyDataType::FLOAT:
    {
        if (std::stof(value_a_str) < std::stof(value_b_str))
            return value_a_str;
        else
            return value_b_str;
    }
    case PropertyDataType::DOUBLE:
    {
        if (std::stod(value_a_str) < std::stod(value_b_str))
            return value_a_str;
        else
            return value_b_str;
    }
    case PropertyDataType::STRING:
        throw std::invalid_argument ("String: getSmallerValueString: operation on string variable impossible");
    default:
        logerr  <<  "String: getSmallerValueString:: unknown property type " << Property::asString(data_type);
        throw std::runtime_error ("String: getSmallerValueString:: unknown property type " + Property::asString(data_type));
    }
}

