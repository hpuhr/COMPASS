/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DATASOURCESFILTER_H_
#define DATASOURCESFILTER_H_

#include "dbfilter.h"

/**
 * @brief Definition for a data source in a DataSourcesFilter
 */
class DataSourcesFilterDataSource
{
public:
    /// @brief Constructor
    DataSourcesFilterDataSource () {}
    /// @brief Destructor
    virtual ~DataSourcesFilterDataSource () {}

protected:
    /// Number id
    unsigned int number_;
    /// Name id
    std::string name_;
    /// Flag indicating if active in data
    bool active_in_data_;
    /// Flag indicating if active in filter
    bool active_in_filter_;

public:
    bool isActiveInData() const
    {
        return active_in_data_;
    }

    void setActiveInData(bool active_in_data)
    {
        active_in_data_ = active_in_data;
    }

    bool isActiveInFilter() const
    {
        return active_in_filter_;
    }

    void setActiveInFilter(bool active_in_filter)
    {
        active_in_filter_ = active_in_filter;
    }

    bool &getActiveInFilterReference () { return active_in_filter_; }

    std::string getName() const
    {
        return name_;
    }

    void setName(std::string name)
    {
        name_ = name;
    }

    unsigned int getNumber() const
    {
        return number_;
    }

    void setNumber(unsigned int number)
    {
        number_ = number;
    }

};

class DBObject;

/**
 * @brief DBFilter specialization for non-generic sensor filters
 *
 * Each DBObject can have data sources, and if such data is contained in the database, a filter for the seperate
 * data sources should always exist. Should not have sub-filters or conditions.
 *
 */
class DataSourcesFilter : public DBFilter
{
public:
  /// @brief Constructor
  DataSourcesFilter(const std::string& class_id, const std::string& instance_id, Configurable* parent);
  /// @brief Desctructor
  virtual ~DataSourcesFilter();

  virtual std::string getConditionString (const std::string& dbo_name, bool& first,
                                          std::vector <DBOVariable*>& filtered_variables);

  virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

  virtual bool filters (const std::string& dbo_name);
  virtual void reset ();

  const std::string& dbObjectName () { return dbo_name_; }

  std::map<int, DataSourcesFilterDataSource> &dataSources () { return data_sources_; }

protected:
  /// DBO type
  std::string dbo_name_;
  DBObject* object_ {nullptr};
  /// Sensor id column name in database table
  std::string ds_column_name_;
  /// Container with all possible data sources and active flag pointers
  std::map<int, DataSourcesFilterDataSource> data_sources_;

  /// @brief Load data sources and updates data_sources_ container
  void updateDataSources ();
  /// @brief Updates the data sources active in data information
  void updateDataSourcesActive ();

  /// @brief Does nothing.
  virtual void checkSubConfigurables ();
};

#endif /* DATASOURCESFILTER_H_ */
