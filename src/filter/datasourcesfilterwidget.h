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

#ifndef DATASOURCESFILTERWIDGET_H_
#define DATASOURCESFILTERWIDGET_H_

#include <QMenu>

#include "datasourcesfilter.h"
#include "dbfilterwidget.h"

class QGridLayout;

/**
 * @brief Specialization of DBFilterWidget, which represents a SensorFilter
 *
 * Uses same control elements but has no management button. Holds all data sources for a DBObject
 * with a checkbox for each to enable/disable loading of data from this source.
 */
class DataSourcesFilterWidget : public DBFilterWidget
{
    Q_OBJECT
  protected slots:
    /// @brief Updates the sensor active checkboxes
    void toggleDataSource();
    /// @brief Selects all sensor active checkboxes
    void selectSensorsAll();
    /// @brief Unselects all sensor active checkboxes
    void selectSensorsNone();
    void setSourcesInactive();

  public:
    /// @brief Constructor
    DataSourcesFilterWidget(DataSourcesFilter& filter, const std::string& class_id,
                            const std::string& instance_id);
    /// @brief Destructor
    virtual ~DataSourcesFilterWidget();

    /// @brief Returns the sensors active container
    // std::map<int, QCheckBox*> &getSensorActiveCheckboxes ();

    /// @brief Sets the sensor active checkboxes according to sensor active container
    virtual void update();

  protected:
    std::map<QCheckBox*, int> data_sources_checkboxes_;
    /// Container with all sensor names (sensor number -> sensor name)
    // std::map<int, std::string> sensor_names_;

    /// Represented sensor filter
    DataSourcesFilter& filter_;
    /// Filtered DBObject type
    std::string dbo_name_;
    /// Grid layout for all sensor checkboxes
    // QGridLayout *sensorboxlay_;
    /// Container with checkboxes for all sensors (sensor number -> checkbox)
    std::map<int, DataSourcesFilterDataSource>& data_sources_;

    void updateCheckboxesChecked();
    void updateCheckboxesDisabled();

    void createMenu(bool inactive_disabled);
};

#endif /* DATASOURCESFILTERWIDGET_H_ */
