/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DBODATASOURCEDEFINITIONWIDGET_H
#define DBODATASOURCEDEFINITIONWIDGET_H

#include <QWidget>

class QComboBox;
class DBObject;
class DBODataSourceDefinition;
class DBSchemaManager;

class DBODataSourceDefinitionWidget : public QWidget
{
    Q_OBJECT

  public slots:
    void changedForeignKeySlot();
    void changedLocalKeySlot();
    void changedMetaTableSlot();
    void changedShortNameColumnSlot();
    void changedNameColumnSlot();
    void changedSacColumnSlot();
    void changedSicColumnSlot();

    void changedLatitudeColumnSlot();
    void changedLongitudeColumnSlot();
    void changedAltitudeColumnSlot();

    // psr
    void changedPrimaryAzimuthStdDevColumnSlot();
    void changedPrimaryRangeStdDevColumnSlot();
    void changedPrimaryIRMinSlot();
    void changedPrimaryIRMaxSlot();
    // ssr
    void changedSecondaryAzimuthStdDevColumnSlot();
    void changedSecondaryRangeStdDevColumnSlot();
    void changedSecondaryIRMinSlot();
    void changedSecondaryIRMaxSlot();
    // mode s
    void changedModeSAzimuthStdDevColumnSlot();
    void changedModeSRangeStdDevColumnSlot();
    void changedModeSIRMinSlot();
    void changedModeSIRMaxSlot();

    /// @brief Updates data source local key selection
    void updateLocalKeySlot();
    /// @brief Updates data source meta table name selection
    void updateMetaTableSlot();
    /// @brief Updates data source foreign key selection
    void updateForeignKeySlot();
    /// @brief Updates data source name column selection
    void updateShortNameColumnSlot();
    void updateNameColumnSlot();
    void updateSacColumnSlot();
    void updateSicColumnSlot();
    void updateLatitudeColumnSlot();
    void updateLongitudeColumnSlot();
    void updateAltitudeColumnSlot();

    // psr
    void updatePrimaryAzimuthStdDevColumnSlot();
    void updatePrimaryRangeStdDevColumnSlot();
    void updatePrimaryIRMinColumnSlot();
    void updatePrimaryIRMaxColumnSlot();
    // ssr
    void updateSecondaryAzimuthStdDevColumnSlot();
    void updateSecondaryRangeStdDevColumnSlot();
    void updateSecondaryIRMinColumnSlot();
    void updateSecondaryIRMaxColumnSlot();
    // mode s
    void updateModeSAzimuthStdDevColumnSlot();
    void updateModeSRangeStdDevColumnSlot();
    void updateModeSIRMinColumnSlot();
    void updateModeSIRMaxColumnSlot();

    // signals:
    //    void definitionChangedSignal();
  public:
    DBODataSourceDefinitionWidget(DBObject& object, DBODataSourceDefinition& definition,
                                  QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~DBODataSourceDefinitionWidget();

  private:
    DBObject& object_;
    DBODataSourceDefinition& definition_;
    DBSchemaManager& schema_manager_;

    /// @brief Add new data source local key selection
    QComboBox* local_key_box_{nullptr};
    /// @brief Add new data source meta table selection
    QComboBox* meta_name_box_{nullptr};
    /// @brief Add new data source foreign key selection
    QComboBox* foreign_key_box_{nullptr};
    /// @brief Add new data source foreign name selection
    QComboBox* short_name_box_{nullptr};
    QComboBox* name_box_{nullptr};
    QComboBox* sac_box_{nullptr};
    QComboBox* sic_box_{nullptr};
    QComboBox* latitude_box_{nullptr};
    QComboBox* longitude_box_{nullptr};
    QComboBox* altitude_box_{nullptr};
    /// psr
    QComboBox* primary_azimuth_stddev_box_{nullptr};
    QComboBox* primary_range_stddev_box_{nullptr};
    QComboBox* primary_ir_min_box_{nullptr};
    QComboBox* primary_ir_max_box_{nullptr};
    // ssr
    QComboBox* secondary_azimuth_stddev_box_{nullptr};
    QComboBox* secondary_range_stddev_box_{nullptr};
    QComboBox* secondary_ir_min_box_{nullptr};
    QComboBox* secondary_ir_max_box_{nullptr};
    // mode s
    QComboBox* mode_s_azimuth_stddev_box_{nullptr};
    QComboBox* mode_s_range_stddev_box_{nullptr};
    QComboBox* mode_s_ir_min_box_{nullptr};
    QComboBox* mode_s_ir_max_box_{nullptr};

    /// @brief Updates a variable selection box
    void updateVariableSelectionBox(QComboBox* box, const std::string& schema_name,
                                    const std::string& meta_table_name, const std::string& value,
                                    bool empty_allowed = false);
};

#endif  // DBODATASOURCEDEFINITIONWIDGET_H
