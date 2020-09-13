#ifndef ACTIVEDATASOURCE_H
#define ACTIVEDATASOURCE_H

#include "json.hpp"

#include <string>

/**
 * @brief Definition for a data source in a DataSourcesFilter
 */
class ActiveDataSource
{
  public:
    /// @brief Constructor
    ActiveDataSource(unsigned int number, const std::string& name,
                                nlohmann::json::boolean_t& active_flag)
        : number_(number), name_(name), active_flag_(active_flag)
    {
    }
    /// @brief Destructor
    virtual ~ActiveDataSource() {}

  protected:
    /// Number id
    unsigned int number_ {0};
    /// Name id
    std::string name_;
    /// Flag indicating if active in data
    bool active_in_data_ {false};
    /// Flag indicating if active (to be used)
    nlohmann::json::boolean_t& active_flag_;

  public:
    bool isActiveInData() const { return active_in_data_; }

    void setActiveInData(bool active_in_data) { active_in_data_ = active_in_data; }

    bool isActive() const
    {
        return active_flag_;
    }

    void setActive(bool active_flag)
    {
        active_flag_ = active_flag;
    }

    std::string getName() const { return name_; }

    unsigned int getNumber() const { return number_; }
};

#endif // ACTIVEDATASOURCE_H
