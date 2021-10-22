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

#include "dbodatasourcedefinitionwidget.h"

#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "compass.h"
#include "dbobject.h"
#include "dbodatasource.h"
#include "dbovariable.h"
#include "dbschema.h"
#include "dbschemamanager.h"
#include "metadbtable.h"

DBODataSourceDefinitionWidget::DBODataSourceDefinitionWidget(DBObject& object,
                                                             DBODataSourceDefinition& definition,
                                                             QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f),
      object_(object),
      definition_(definition),
      schema_manager_(COMPASS::instance().schemaManager())
{
    setMinimumSize(QSize(800, 600));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    QVBoxLayout* main_layout = new QVBoxLayout();

    QLabel* main_label = new QLabel("Edit data source definition");
    main_label->setFont(font_big);
    main_layout->addWidget(main_label);

    // object parameters
    QGridLayout* grid = new QGridLayout();

    unsigned int row = 0;

    grid->addWidget(new QLabel("Object"), row, 0);
    grid->addWidget(new QLabel(object_.name().c_str()), row, 1);

    row++;
    grid->addWidget(new QLabel("Local key"), row, 0);

    local_key_box_ = new QComboBox();
    updateLocalKeySlot();
    connect(local_key_box_, SIGNAL(activated(int)), this, SLOT(changedLocalKeySlot()));
    grid->addWidget(local_key_box_, row, 1);

    row++;
    grid->addWidget(new QLabel("Meta table"), row, 0);

    meta_name_box_ = new QComboBox();
    updateMetaTableSlot();
    connect(meta_name_box_, SIGNAL(activated(int)), this, SLOT(changedMetaTableSlot()));
    grid->addWidget(meta_name_box_, row, 1);

    row++;
    grid->addWidget(new QLabel("Foreign key"), row, 0);

    foreign_key_box_ = new QComboBox();
    updateForeignKeySlot();
    connect(foreign_key_box_, SIGNAL(activated(int)), this, SLOT(changedForeignKeySlot()));
    grid->addWidget(foreign_key_box_, row, 1);

    row++;
    grid->addWidget(new QLabel("Short name column"), row, 0);

    short_name_box_ = new QComboBox();
    updateShortNameColumnSlot();
    connect(short_name_box_, SIGNAL(activated(int)), this, SLOT(changedShortNameColumnSlot()));
    grid->addWidget(short_name_box_, row, 1);

    row++;
    grid->addWidget(new QLabel("Name column"), row, 0);

    name_box_ = new QComboBox();
    updateNameColumnSlot();
    connect(name_box_, SIGNAL(activated(int)), this, SLOT(changedNameColumnSlot()));
    grid->addWidget(name_box_, row, 1);

    row++;
    grid->addWidget(new QLabel("SAC column"), row, 0);

    sac_box_ = new QComboBox();
    updateSacColumnSlot();
    connect(sac_box_, SIGNAL(activated(int)), this, SLOT(changedSacColumnSlot()));
    grid->addWidget(sac_box_, row, 1);

    row++;
    grid->addWidget(new QLabel("SIC column"), row, 0);

    sic_box_ = new QComboBox();
    updateSicColumnSlot();
    connect(sic_box_, SIGNAL(activated(int)), this, SLOT(changedSicColumnSlot()));
    grid->addWidget(sic_box_, row, 1);

    row++;
    grid->addWidget(new QLabel("Latitude column"), row, 0);

    latitude_box_ = new QComboBox();
    updateLatitudeColumnSlot();
    connect(latitude_box_, SIGNAL(activated(int)), this, SLOT(changedLatitudeColumnSlot()));
    grid->addWidget(latitude_box_, row, 1);

    row++;
    grid->addWidget(new QLabel("Longitude column"), row, 0);

    longitude_box_ = new QComboBox();
    updateLongitudeColumnSlot();
    connect(longitude_box_, SIGNAL(activated(int)), this, SLOT(changedLongitudeColumnSlot()));
    grid->addWidget(longitude_box_, row, 1);

    row++;
    grid->addWidget(new QLabel("Altitude column"), row, 0);

    altitude_box_ = new QComboBox();
    updateAltitudeColumnSlot();
    connect(altitude_box_, SIGNAL(activated(int)), this, SLOT(changedAltitudeColumnSlot()));
    grid->addWidget(altitude_box_, row, 1);

    // psr
    row++;
    grid->addWidget(new QLabel("Primary Azimuth Standard Deviation column"), row, 0);

    primary_azimuth_stddev_box_ = new QComboBox();
    updatePrimaryAzimuthStdDevColumnSlot();
    connect(primary_azimuth_stddev_box_, SIGNAL(activated(int)),
            this, SLOT(changedPrimaryAzimuthStdDevColumnSlot()));
    grid->addWidget(primary_azimuth_stddev_box_, row, 1);

    row++;
    grid->addWidget(new QLabel("Primary Range Standard Deviation column"), row, 0);

    primary_range_stddev_box_ = new QComboBox();
    updatePrimaryRangeStdDevColumnSlot();
    connect(primary_range_stddev_box_, SIGNAL(activated(int)),
            this, SLOT(changedPrimaryRangeStdDevColumnSlot()));
    grid->addWidget(primary_range_stddev_box_, row, 1);

    row++;
    grid->addWidget(new QLabel("Primary IR Minimum column"), row, 0);

    primary_ir_min_box_ = new QComboBox();
    updatePrimaryIRMinColumnSlot();
    connect(primary_ir_min_box_, SIGNAL(activated(int)),
            this, SLOT(changedPrimaryIRMinSlot()));
    grid->addWidget(primary_ir_min_box_, row, 1);

    row++;
    grid->addWidget(new QLabel("Primary IR Maximum column"), row, 0);

    primary_ir_max_box_ = new QComboBox();
    updatePrimaryIRMaxColumnSlot();
    connect(primary_ir_max_box_, SIGNAL(activated(int)),
            this, SLOT(changedPrimaryIRMaxSlot()));
    grid->addWidget(primary_ir_max_box_, row, 1);

    // ssr
    row++;
    grid->addWidget(new QLabel("Secondary Azimuth Standard Deviation column"), row, 0);

    secondary_azimuth_stddev_box_ = new QComboBox();
    updateSecondaryAzimuthStdDevColumnSlot();
    connect(secondary_azimuth_stddev_box_, SIGNAL(activated(int)),
            this, SLOT(changedSecondaryAzimuthStdDevColumnSlot()));
    grid->addWidget(secondary_azimuth_stddev_box_, row, 1);

    row++;
    grid->addWidget(new QLabel("Secondary Range Standard Deviation column"), row, 0);

    secondary_range_stddev_box_ = new QComboBox();
    updateSecondaryRangeStdDevColumnSlot();
    connect(secondary_range_stddev_box_, SIGNAL(activated(int)),
            this, SLOT(changedSecondaryRangeStdDevColumnSlot()));
    grid->addWidget(secondary_range_stddev_box_, row, 1);

    row++;
    grid->addWidget(new QLabel("Secondary IR Minimum column"), row, 0);

    secondary_ir_min_box_ = new QComboBox();
    updateSecondaryIRMinColumnSlot();
    connect(secondary_ir_min_box_, SIGNAL(activated(int)),
            this, SLOT(changedSecondaryIRMinSlot()));
    grid->addWidget(secondary_ir_min_box_, row, 1);

    row++;
    grid->addWidget(new QLabel("Secondary IR Maximum column"), row, 0);

    secondary_ir_max_box_ = new QComboBox();
    updateSecondaryIRMaxColumnSlot();
    connect(secondary_ir_max_box_, SIGNAL(activated(int)),
            this, SLOT(changedSecondaryIRMaxSlot()));
    grid->addWidget(secondary_ir_max_box_, row, 1);

    // mode s
    row++;
    grid->addWidget(new QLabel("Mode S Azimuth Standard Deviation column"), row, 0);

    mode_s_azimuth_stddev_box_ = new QComboBox();
    updateModeSAzimuthStdDevColumnSlot();
    connect(mode_s_azimuth_stddev_box_, SIGNAL(activated(int)),
            this, SLOT(changedModeSAzimuthStdDevColumnSlot()));
    grid->addWidget(mode_s_azimuth_stddev_box_, row, 1);

    row++;
    grid->addWidget(new QLabel("Mode S Range Standard Deviation column"), row, 0);

    mode_s_range_stddev_box_ = new QComboBox();
    updateModeSRangeStdDevColumnSlot();
    connect(mode_s_range_stddev_box_, SIGNAL(activated(int)),
            this, SLOT(changedModeSRangeStdDevColumnSlot()));
    grid->addWidget(mode_s_range_stddev_box_, row, 1);

    row++;
    grid->addWidget(new QLabel("Mode S IR Minimum column"), row, 0);

    mode_s_ir_min_box_ = new QComboBox();
    updateModeSIRMinColumnSlot();
    connect(mode_s_ir_min_box_, SIGNAL(activated(int)),
            this, SLOT(changedModeSIRMinSlot()));
    grid->addWidget(mode_s_ir_min_box_, row, 1);

    row++;
    grid->addWidget(new QLabel("Mode S IR Maximum column"), row, 0);

    mode_s_ir_max_box_ = new QComboBox();
    updateModeSIRMaxColumnSlot();
    connect(mode_s_ir_max_box_, SIGNAL(activated(int)),
            this, SLOT(changedModeSIRMaxSlot()));
    grid->addWidget(mode_s_ir_max_box_, row, 1);

    main_layout->addLayout(grid);

    setLayout(main_layout);
}

DBODataSourceDefinitionWidget::~DBODataSourceDefinitionWidget() {}

void DBODataSourceDefinitionWidget::changedLocalKeySlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedLocalKeySlot";
    assert(local_key_box_);
    std::string value = local_key_box_->currentText().toStdString();
    definition_.localKey(value);
}

void DBODataSourceDefinitionWidget::changedMetaTableSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedMetaTableSlot";
    assert(meta_name_box_);
    std::string value = meta_name_box_->currentText().toStdString();
    definition_.metaTable(value);

    updateForeignKeySlot();
    updateShortNameColumnSlot();
    updateNameColumnSlot();
    updateSacColumnSlot();
    updateSicColumnSlot();

    updateLatitudeColumnSlot();
    updateLongitudeColumnSlot();
    updateAltitudeColumnSlot();

    updatePrimaryAzimuthStdDevColumnSlot();
    updatePrimaryRangeStdDevColumnSlot();
    updatePrimaryIRMinColumnSlot();
    updatePrimaryIRMaxColumnSlot();

    updateSecondaryAzimuthStdDevColumnSlot();
    updateSecondaryRangeStdDevColumnSlot();
    updateSecondaryIRMinColumnSlot();
    updateSecondaryIRMaxColumnSlot();

    updateModeSAzimuthStdDevColumnSlot();
    updateModeSRangeStdDevColumnSlot();
    updateModeSIRMinColumnSlot();
    updateModeSIRMaxColumnSlot();
}

void DBODataSourceDefinitionWidget::changedForeignKeySlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedForeignKeySlot";
    assert(foreign_key_box_);
    std::string value = foreign_key_box_->currentText().toStdString();
    definition_.foreignKey(value);
}

void DBODataSourceDefinitionWidget::changedShortNameColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedShortNameColumnSlot";
    assert(short_name_box_);
    std::string value = short_name_box_->currentText().toStdString();
    definition_.shortNameColumn(value);
}

void DBODataSourceDefinitionWidget::changedNameColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedNameColumnSlot";
    assert(name_box_);
    std::string value = name_box_->currentText().toStdString();
    definition_.nameColumn(value);
}

void DBODataSourceDefinitionWidget::changedSacColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedSacColumnSlot";
    assert(sac_box_);
    std::string value = sac_box_->currentText().toStdString();
    definition_.sacColumn(value);
}

void DBODataSourceDefinitionWidget::changedSicColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedSicColumnSlot";
    assert(sic_box_);
    std::string value = sic_box_->currentText().toStdString();
    definition_.sicColumn(value);
}

void DBODataSourceDefinitionWidget::changedLatitudeColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedLatitudeColumnSlot";
    assert(latitude_box_);
    std::string value = latitude_box_->currentText().toStdString();
    definition_.latitudeColumn(value);
}

void DBODataSourceDefinitionWidget::changedLongitudeColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedLongitudeColumnSlot";
    assert(longitude_box_);
    std::string value = longitude_box_->currentText().toStdString();
    definition_.longitudeColumn(value);
}

void DBODataSourceDefinitionWidget::changedAltitudeColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedAltitudeColumnSlot";
    assert(altitude_box_);
    std::string value = altitude_box_->currentText().toStdString();
    definition_.altitudeColumn(value);
}

void DBODataSourceDefinitionWidget::changedPrimaryAzimuthStdDevColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedPrimaryAzimuthStdDevColumnSlot";
    assert(primary_azimuth_stddev_box_);
    std::string value = primary_azimuth_stddev_box_->currentText().toStdString();
    definition_.primaryAzimuthStdDevColumn(value);
}
void DBODataSourceDefinitionWidget::changedPrimaryRangeStdDevColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedPrimaryRangeStdDevColumnSlot";
    assert(primary_range_stddev_box_);
    std::string value = primary_range_stddev_box_->currentText().toStdString();
    definition_.primaryRangeStdDevColumn(value);
}
void DBODataSourceDefinitionWidget::changedPrimaryIRMinSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedPrimaryIRMinSlot";
    assert(primary_ir_min_box_);
    std::string value = primary_ir_min_box_->currentText().toStdString();
    definition_.primaryIRMinColumn(value);
}
void DBODataSourceDefinitionWidget::changedPrimaryIRMaxSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedPrimaryIRMaxSlot";
    assert(primary_ir_max_box_);
    std::string value = primary_ir_max_box_->currentText().toStdString();
    definition_.primaryIRMaxColumn(value);
}
void DBODataSourceDefinitionWidget::changedSecondaryAzimuthStdDevColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedSecondaryAzimuthStdDevColumnSlot";
    assert(secondary_azimuth_stddev_box_);
    std::string value = secondary_azimuth_stddev_box_->currentText().toStdString();
    definition_.secondaryAzimuthStdDevColumn(value);
}
void DBODataSourceDefinitionWidget::changedSecondaryRangeStdDevColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedSecondaryRangeStdDevColumnSlot";
    assert(secondary_range_stddev_box_);
    std::string value = secondary_range_stddev_box_->currentText().toStdString();
    definition_.secondaryRangeStdDevColumn(value);
}
void DBODataSourceDefinitionWidget::changedSecondaryIRMinSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedSecondaryIRMinSlot";
    assert(secondary_ir_min_box_);
    std::string value = secondary_ir_min_box_->currentText().toStdString();
    definition_.secondaryIRMinColumn(value);
}
void DBODataSourceDefinitionWidget::changedSecondaryIRMaxSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedSecondaryIRMaxSlot";
    assert(secondary_ir_max_box_);
    std::string value = secondary_ir_max_box_->currentText().toStdString();
    definition_.secondaryIRMaxColumn(value);
}
void DBODataSourceDefinitionWidget::changedModeSAzimuthStdDevColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedModeSAzimuthStdDevColumnSlot";
    assert(mode_s_azimuth_stddev_box_);
    std::string value = mode_s_azimuth_stddev_box_->currentText().toStdString();
    definition_.modeSAzimuthStdDevColumn(value);
}
void DBODataSourceDefinitionWidget::changedModeSRangeStdDevColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedModeSRangeStdDevColumnSlot";
    assert(mode_s_range_stddev_box_);
    std::string value = mode_s_range_stddev_box_->currentText().toStdString();
    definition_.modeSRangeStdDevColumn(value);
}
void DBODataSourceDefinitionWidget::changedModeSIRMinSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedModeSIRMinSlot";
    assert(mode_s_ir_min_box_);
    std::string value = mode_s_ir_min_box_->currentText().toStdString();
    definition_.modeSIRMinColumn(value);
}
void DBODataSourceDefinitionWidget::changedModeSIRMaxSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: changedModeSIRMaxSlot";
    assert(mode_s_ir_max_box_);
    std::string value = mode_s_ir_max_box_->currentText().toStdString();
    definition_.modeSIRMaxColumn(value);
}
void DBODataSourceDefinitionWidget::updateLocalKeySlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updateLocalKeySlot";

    std::string selection;

    if (local_key_box_->count() > 0)
        selection = local_key_box_->currentText().toStdString();
    else
        selection = definition_.localKey();

    while (local_key_box_->count() > 0)
        local_key_box_->removeItem(0);

    int index_cnt = -1;
    unsigned int cnt = 0;

    for (auto& var_it : object_)
    {
        if (selection.size() > 0 && selection.compare(var_it.second.name()) == 0)
            index_cnt = cnt;

        local_key_box_->addItem(var_it.second.name().c_str());

        cnt++;
    }

    if (index_cnt != -1)
    {
        local_key_box_->setCurrentIndex(index_cnt);
    }
}
void DBODataSourceDefinitionWidget::updateMetaTableSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updateMetaTableSlot";
    assert(meta_name_box_);

    std::string selection;

    if (meta_name_box_->count() > 0)
        selection = meta_name_box_->currentText().toStdString();
    else
        selection = definition_.metaTableName();

    while (meta_name_box_->count() > 0)
        meta_name_box_->removeItem(0);

    DBSchema& schema = schema_manager_.getCurrentSchema();

    auto metatables = schema.metaTables();

    int index_cnt = -1;
    unsigned int cnt = 0;
    for (auto it = metatables.begin(); it != metatables.end(); it++)
    {
        if (selection.size() > 0 && selection.compare(it->second->name()) == 0)
            index_cnt = cnt;

        meta_name_box_->addItem(it->second->name().c_str());

        cnt++;
    }

    if (index_cnt != -1)
    {
        meta_name_box_->setCurrentIndex(index_cnt);
    }
}
void DBODataSourceDefinitionWidget::updateForeignKeySlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updateForeignKeySlot";
    assert(foreign_key_box_);
    assert(meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();


    updateVariableSelectionBox(foreign_key_box_, meta_table_name,
                               definition_.foreignKey());
}

void DBODataSourceDefinitionWidget::updateShortNameColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updateShortNameColumnSlot";
    assert(short_name_box_);
    assert(meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();


    updateVariableSelectionBox(short_name_box_, meta_table_name,
                               definition_.shortNameColumn(), true);
}

void DBODataSourceDefinitionWidget::updateNameColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updateNameColumnSlot";
    assert(name_box_);
    assert(meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();


    updateVariableSelectionBox(name_box_, meta_table_name, definition_.nameColumn());
}

void DBODataSourceDefinitionWidget::updateSacColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updateSacColumnSlot";
    assert(sac_box_);
    assert(meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();


    updateVariableSelectionBox(sac_box_, meta_table_name, definition_.sacColumn(),
                               true);
}

void DBODataSourceDefinitionWidget::updateSicColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updateSicColumnSlot";
    assert(sic_box_);
    assert(meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();


    updateVariableSelectionBox(sic_box_, meta_table_name, definition_.sicColumn(),
                               true);
}

void DBODataSourceDefinitionWidget::updateLatitudeColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updateLatitudeColumnSlot";
    assert(latitude_box_);
    assert(meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();


    updateVariableSelectionBox(latitude_box_, meta_table_name,
                               definition_.latitudeColumn(), true);
}

void DBODataSourceDefinitionWidget::updateLongitudeColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updateLongitudeColumnSlot";
    assert(longitude_box_);
    assert(meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();


    updateVariableSelectionBox(longitude_box_, meta_table_name,
                               definition_.longitudeColumn(), true);
}

void DBODataSourceDefinitionWidget::updateAltitudeColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updateAltitudeColumnSlot";
    assert(altitude_box_);
    assert(meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();


    updateVariableSelectionBox(altitude_box_, meta_table_name,
                               definition_.altitudeColumn(), true);
}

void DBODataSourceDefinitionWidget::updatePrimaryAzimuthStdDevColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updatePrimaryAzimuthStdDevColumnSlot";
    assert(primary_azimuth_stddev_box_);
    assert(meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();


    updateVariableSelectionBox(primary_azimuth_stddev_box_, meta_table_name,
                               definition_.primaryAzimuthStdDevColumn(), true);
}
void DBODataSourceDefinitionWidget::updatePrimaryRangeStdDevColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updatePrimaryRangeStdDevColumnSlot";
    assert(primary_range_stddev_box_);
    assert(meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();


    updateVariableSelectionBox(primary_range_stddev_box_, meta_table_name,
                               definition_.primaryRangeStdDevColumn(), true);
}
void DBODataSourceDefinitionWidget::updatePrimaryIRMinColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updatePrimaryIRMinColumnSlot";
    assert(primary_ir_min_box_);
    assert(meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();


    updateVariableSelectionBox(primary_ir_min_box_, meta_table_name,
                               definition_.primaryIRMinColumn(), true);
}
void DBODataSourceDefinitionWidget::updatePrimaryIRMaxColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updatePrimaryIRMaxColumnSlot";
    assert(primary_ir_max_box_);
    assert(meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();


    updateVariableSelectionBox(primary_ir_max_box_, meta_table_name,
                               definition_.primaryIRMaxColumn(), true);
}

void DBODataSourceDefinitionWidget::updateSecondaryAzimuthStdDevColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updateSecondaryAzimuthStdDevColumnSlot";
    assert(secondary_azimuth_stddev_box_);
    assert(meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();


    updateVariableSelectionBox(secondary_azimuth_stddev_box_, meta_table_name,
                               definition_.secondaryAzimuthStdDevColumn(), true);
}
void DBODataSourceDefinitionWidget::updateSecondaryRangeStdDevColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updateSecondaryRangeStdDevColumnSlot";
    assert(secondary_range_stddev_box_);
    assert(meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();


    updateVariableSelectionBox(secondary_range_stddev_box_, meta_table_name,
                               definition_.secondaryRangeStdDevColumn(), true);
}
void DBODataSourceDefinitionWidget::updateSecondaryIRMinColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updateSecondaryIRMinColumnSlot";
    assert(secondary_ir_min_box_);
    assert(meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();


    updateVariableSelectionBox(secondary_ir_min_box_, meta_table_name,
                               definition_.secondaryIRMinColumn(), true);
}
void DBODataSourceDefinitionWidget::updateSecondaryIRMaxColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updateSecondaryIRMaxColumnSlot";
    assert(secondary_ir_max_box_);
    assert(meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();


    updateVariableSelectionBox(secondary_ir_max_box_, meta_table_name,
                               definition_.secondaryIRMaxColumn(), true);
}

void DBODataSourceDefinitionWidget::updateModeSAzimuthStdDevColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updateModeSAzimuthStdDevColumnSlot";
    assert(mode_s_azimuth_stddev_box_);
    assert(meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();


    updateVariableSelectionBox(mode_s_azimuth_stddev_box_, meta_table_name,
                               definition_.modeSAzimuthStdDevColumn(), true);
}
void DBODataSourceDefinitionWidget::updateModeSRangeStdDevColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updateModeSRangeStdDevColumnSlot";
    assert(mode_s_range_stddev_box_);
    assert(meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();


    updateVariableSelectionBox(mode_s_range_stddev_box_, meta_table_name,
                               definition_.modeSRangeStdDevColumn(), true);
}
void DBODataSourceDefinitionWidget::updateModeSIRMinColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updateModeSIRMinColumnSlot";
    assert(mode_s_ir_min_box_);
    assert(meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();


    updateVariableSelectionBox(mode_s_ir_min_box_, meta_table_name,
                               definition_.modeSIRMinColumn(), true);
}
void DBODataSourceDefinitionWidget::updateModeSIRMaxColumnSlot()
{
    logdbg << "DBODataSourceDefinitionWidget: updateModeSIRMaxColumnSlot";
    assert(mode_s_ir_max_box_);
    assert(meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();


    updateVariableSelectionBox(mode_s_ir_max_box_, meta_table_name,
                               definition_.modeSIRMaxColumn(), true);
}

void DBODataSourceDefinitionWidget::updateVariableSelectionBox(QComboBox* box,
                                                               const std::string& meta_table_name,
                                                               const std::string& value,
                                                               bool empty_allowed)
{
    logdbg << "DBODataSourceDefinitionWidget: updateVariableSelectionBox: value " << value;
    assert(box);

    while (box->count() > 0)
        box->removeItem(0);

    DBSchema& schema = schema_manager_.getCurrentSchema();

    assert(schema.hasMetaTable(meta_table_name));

    const MetaDBTable& meta_table = schema.metaTable(meta_table_name);

    auto table_columns = meta_table.columns();

    if (empty_allowed)
        box->addItem("");

    for (auto it = table_columns.begin(); it != table_columns.end(); it++)
        box->addItem(it->first.c_str());

    box->setCurrentText(value.c_str());
}
