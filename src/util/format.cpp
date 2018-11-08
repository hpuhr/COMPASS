#include "format.h"

#include <algorithm>
#include <initializer_list>
#include <cassert>

const std::vector<std::string> no_format = {""};
const std::vector<std::string> integer_formats {"", "decimal", "hexadecimal", "octal"};

const std::map<PropertyDataType, std::vector<std::string>> Format::format_options_ {
    {PropertyDataType::BOOL,       no_format},
    {PropertyDataType::CHAR,       integer_formats},
    {PropertyDataType::UCHAR,      integer_formats},
    {PropertyDataType::INT,        integer_formats},
    {PropertyDataType::UINT,       integer_formats},
    {PropertyDataType::LONGINT,    integer_formats},
    {PropertyDataType::ULONGINT,   integer_formats},
    {PropertyDataType::FLOAT,      no_format},
    {PropertyDataType::DOUBLE,     no_format},
    {PropertyDataType::STRING,     {"", "decimal", "hexadecimal", "octal", "epoch_tod"}}};

void Format::set(PropertyDataType data_type, const std::string& value)
{
    assert (format_options_.count(data_type) > 0);
    assert (std::find(format_options_.at(data_type).begin(), format_options_.at(data_type).end(), value)
                      != format_options_.at(data_type).end());
    std::string::operator =(value);
}
