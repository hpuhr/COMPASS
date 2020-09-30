#ifndef VIEWABLEDATACONFIG_H
#define VIEWABLEDATACONFIG_H

#include "json.hpp"

class ViewableDataConfig
{
public:
  ViewableDataConfig(const nlohmann::json::object_t& data)
  {
      data_ = data;
  }

  ViewableDataConfig(const std::string& json_str)
  {
      data_ = nlohmann::json::parse(json_str);
  }

  const nlohmann::json& data() const { return data_; }

protected:
    nlohmann::json data_;
};

#endif // VIEWABLEDATACONFIG_H
