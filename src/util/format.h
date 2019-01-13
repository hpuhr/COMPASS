#ifndef FORMAT_H
#define FORMAT_H

#include "property.h"

#include <string>
#include <vector>
#include <map>

class Format : public std::string
{
public:
    Format () = default;
    Format (PropertyDataType data_type, const std::string& value) { set (data_type, value); }

    void set(PropertyDataType data_type, const std::string& value);

    const std::vector<std::string>& getFormatOptions (PropertyDataType data_type) {
        return format_options_.at(data_type); }

    const std::map<PropertyDataType, std::vector<std::string>>& getAllFormatOptions () { return format_options_; }

private:
    static const std::map<PropertyDataType, std::vector<std::string>> format_options_;
};

#endif // FORMAT_H
