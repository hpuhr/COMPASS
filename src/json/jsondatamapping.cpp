#include "jsondatamapping.h"
#include "dbovariable.h"

JSONDataMapping::JSONDataMapping (const std::string& json_key, DBOVariable& variable, bool mandatory)
    : JSONDataMapping(json_key, variable, mandatory, {variable.dataType(), ""}, "", "")
{}

JSONDataMapping::JSONDataMapping (const std::string& json_key, DBOVariable& variable, bool mandatory,
                            Format json_value_format)
    : JSONDataMapping(json_key, variable, mandatory, json_value_format, "", "")
{}

JSONDataMapping::JSONDataMapping (const std::string& json_key, DBOVariable& variable, bool mandatory,
                            const std::string& dimension, const std::string& unit)
    : JSONDataMapping(json_key, variable, mandatory, {variable.dataType(), ""}, dimension, unit)
{}

JSONDataMapping::JSONDataMapping (const std::string& json_key, DBOVariable& variable, bool mandatory,
                            Format json_value_format, const std::string& dimension, const std::string& unit)
    : json_key_(json_key), variable_(variable), mandatory_(mandatory), json_value_format_(json_value_format),
      dimension_(dimension), unit_(unit)
{
    sub_keys_ = Utils::String::split(json_key_, '.');
    has_sub_keys_ = sub_keys_.size() > 1;
    num_sub_keys_ = sub_keys_.size();

    logdbg << "JSONDataMapping: ctor: key " << json_key_ << " num subkeys " << sub_keys_.size();

}


DBOVariable &JSONDataMapping::variable() const
{
    return variable_;
}

bool JSONDataMapping::mandatory() const
{
    return mandatory_;
}

void JSONDataMapping::mandatory(bool mandatory)
{
    mandatory_ = mandatory;
}

Format JSONDataMapping::jsonValueFormat() const
{
    return json_value_format_;
}

void JSONDataMapping::jsonValueFormat(const Format &json_value_format)
{
    json_value_format_ = json_value_format;
}

std::string JSONDataMapping::jsonKey() const
{
    return json_key_;
}

void JSONDataMapping::jsonKey(const std::string &json_key)
{
    json_key_ = json_key;
}
