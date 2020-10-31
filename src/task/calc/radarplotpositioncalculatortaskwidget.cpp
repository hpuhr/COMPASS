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

#include "radarplotpositioncalculatortaskwidget.h"

#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "compass.h"
#include "dbobjectcombobox.h"
#include "dbobjectmanager.h"
#include "dbovariable.h"
#include "dbovariableselectionwidget.h"
#include "logger.h"
#include "projectionmanager.h"
#include "projectionmanagerwidget.h"
#include "radarplotpositioncalculatortask.h"
#include "stringconv.h"
#include "taskmanager.h"

using namespace Utils::String;

RadarPlotPositionCalculatorTaskWidget::RadarPlotPositionCalculatorTaskWidget(
    RadarPlotPositionCalculatorTask& task, QWidget* parent, Qt::WindowFlags f)
    : TaskWidget(parent, f), task_(task)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    main_layout->addWidget(ProjectionManager::instance().widget());

    QGridLayout* grid = new QGridLayout();
    int row_cnt = 0;

    grid->addWidget(new QLabel("DBObject"), row_cnt, 0);

    object_box_ = new DBObjectComboBox(false);
    connect(object_box_, &DBObjectComboBox::changedObject, this,
            &RadarPlotPositionCalculatorTaskWidget::dbObjectChangedSlot);
    grid->addWidget(object_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget(new QLabel("Key Variable"), row_cnt, 0);
    key_box_ = new DBOVariableSelectionWidget();
    connect(key_box_, &DBOVariableSelectionWidget::selectionChanged, this,
            &RadarPlotPositionCalculatorTaskWidget::keyVarChangedSlot);
    grid->addWidget(key_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget(new QLabel("Data Source Variable"), row_cnt, 0);
    datasource_box_ = new DBOVariableSelectionWidget();
    connect(datasource_box_, &DBOVariableSelectionWidget::selectionChanged, this,
            &RadarPlotPositionCalculatorTaskWidget::datasourceVarChangedSlot);
    grid->addWidget(datasource_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget(new QLabel("Range Variable"), row_cnt, 0);
    range_box_ = new DBOVariableSelectionWidget();
    connect(range_box_, &DBOVariableSelectionWidget::selectionChanged, this,
            &RadarPlotPositionCalculatorTaskWidget::rangeVarChangedSlot);
    grid->addWidget(range_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget(new QLabel("Azimuth Variable"), row_cnt, 0);
    azimuth_box_ = new DBOVariableSelectionWidget();
    connect(azimuth_box_, &DBOVariableSelectionWidget::selectionChanged, this,
            &RadarPlotPositionCalculatorTaskWidget::azimuthVarChangedSlot);
    grid->addWidget(azimuth_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget(new QLabel("Altitude Variable"), row_cnt, 0);
    altitude_box_ = new DBOVariableSelectionWidget();
    connect(altitude_box_, &DBOVariableSelectionWidget::selectionChanged, this,
            &RadarPlotPositionCalculatorTaskWidget::altitudeVarChangedSlot);
    grid->addWidget(altitude_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget(new QLabel("Latitude Variable"), row_cnt, 0);
    latitude_box_ = new DBOVariableSelectionWidget();
    connect(latitude_box_, &DBOVariableSelectionWidget::selectionChanged, this,
            &RadarPlotPositionCalculatorTaskWidget::latitudeVarChangedSlot);
    grid->addWidget(latitude_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget(new QLabel("Longitude"), row_cnt, 0);
    longitude_box_ = new DBOVariableSelectionWidget();
    connect(longitude_box_, &DBOVariableSelectionWidget::selectionChanged, this,
            &RadarPlotPositionCalculatorTaskWidget::longitudeVarChangedSlot);
    grid->addWidget(longitude_box_, row_cnt, 1);

    main_layout->addLayout(grid);

    expertModeChangedSlot();
    update();

    main_layout->addStretch();

    setLayout(main_layout);
}

RadarPlotPositionCalculatorTaskWidget::~RadarPlotPositionCalculatorTaskWidget() {}

void RadarPlotPositionCalculatorTaskWidget::update()
{
    std::string object_name = task_.dbObjectStr();

    if (object_name.size())
    {
        assert(object_box_);
        object_box_->setObjectName(object_name);
        setDBOBject(object_name);

        assert(COMPASS::instance().objectManager().existsObject(object_name));
        DBObject& object = COMPASS::instance().objectManager().object(object_name);

        assert(key_box_);
        if (task_.keyVarStr().size() && object.hasVariable(task_.keyVarStr()))
            key_box_->selectedVariable(object.variable(task_.keyVarStr()));

        assert(datasource_box_);
        if (task_.datasourceVarStr().size() && object.hasVariable(task_.datasourceVarStr()))
            datasource_box_->selectedVariable(object.variable(task_.datasourceVarStr()));

        assert(range_box_);
        if (task_.rangeVarStr().size() && object.hasVariable(task_.rangeVarStr()))
            range_box_->selectedVariable(object.variable(task_.rangeVarStr()));

        assert(azimuth_box_);
        if (task_.azimuthVarStr().size() && object.hasVariable(task_.azimuthVarStr()))
            azimuth_box_->selectedVariable(object.variable(task_.azimuthVarStr()));

        assert(altitude_box_);
        if (task_.altitudeVarStr().size() && object.hasVariable(task_.altitudeVarStr()))
            altitude_box_->selectedVariable(object.variable(task_.altitudeVarStr()));

        assert(latitude_box_);
        if (task_.latitudeVarStr().size() && object.hasVariable(task_.latitudeVarStr()))
            latitude_box_->selectedVariable(object.variable(task_.latitudeVarStr()));

        assert(longitude_box_);
        if (task_.longitudeVarStr().size() && object.hasVariable(task_.longitudeVarStr()))
            longitude_box_->selectedVariable(object.variable(task_.longitudeVarStr()));
    }
}

void RadarPlotPositionCalculatorTaskWidget::dbObjectChangedSlot()
{
    assert(object_box_);

    std::string object_name = object_box_->getObjectName();

    loginf << "RadarPlotPositionCalculatorTaskWidget: dbObjectChangedSlot: " << object_name;
    setDBOBject(object_name);
}

void RadarPlotPositionCalculatorTaskWidget::keyVarChangedSlot()
{
    assert(key_box_);
    if (key_box_->hasVariable())
    {
        if ((task_.keyVarStr() != key_box_->selectedVariable().name()))
            task_.keyVarStr(key_box_->selectedVariable().name());
    }
    else
        task_.keyVarStr("");
}

void RadarPlotPositionCalculatorTaskWidget::datasourceVarChangedSlot()
{
    assert(datasource_box_);
    if (datasource_box_->hasVariable())
    {
        if ((task_.datasourceVarStr() != datasource_box_->selectedVariable().name()))
            task_.datasourceVarStr(datasource_box_->selectedVariable().name());
    }
    else
        task_.datasourceVarStr("");
}
void RadarPlotPositionCalculatorTaskWidget::rangeVarChangedSlot()
{
    assert(range_box_);
    if (range_box_->hasVariable())
    {
        if ((task_.rangeVarStr() != range_box_->selectedVariable().name()))
            task_.rangeVarStr(range_box_->selectedVariable().name());
    }
    else
        task_.rangeVarStr("");
}
void RadarPlotPositionCalculatorTaskWidget::azimuthVarChangedSlot()
{
    assert(azimuth_box_);
    if (azimuth_box_->hasVariable())
    {
        if ((task_.azimuthVarStr() != azimuth_box_->selectedVariable().name()))
            task_.azimuthVarStr(azimuth_box_->selectedVariable().name());
    }
    else
        task_.azimuthVarStr("");
}
void RadarPlotPositionCalculatorTaskWidget::altitudeVarChangedSlot()
{
    assert(altitude_box_);
    if (altitude_box_->hasVariable())
    {
        if ((task_.altitudeVarStr() != altitude_box_->selectedVariable().name()))
            task_.altitudeVarStr(altitude_box_->selectedVariable().name());
    }
    else
        task_.altitudeVarStr("");
}
void RadarPlotPositionCalculatorTaskWidget::latitudeVarChangedSlot()
{
    assert(latitude_box_);
    if (latitude_box_->hasVariable())
    {
        if ((task_.latitudeVarStr() != latitude_box_->selectedVariable().name()))
            task_.latitudeVarStr(latitude_box_->selectedVariable().name());
    }
    else
        task_.latitudeVarStr("");
}
void RadarPlotPositionCalculatorTaskWidget::longitudeVarChangedSlot()
{
    assert(longitude_box_);
    if (longitude_box_->hasVariable())
    {
        if ((task_.longitudeVarStr() != longitude_box_->selectedVariable().name()))
            task_.longitudeVarStr(longitude_box_->selectedVariable().name());
    }
    else
        task_.longitudeVarStr("");
}

void RadarPlotPositionCalculatorTaskWidget::expertModeChangedSlot()
{
    bool expert_mode = task_.manager().expertMode();

    assert(object_box_);
    object_box_->setEnabled(expert_mode);

    assert(key_box_);
    key_box_->setEnabled(expert_mode);

    assert(datasource_box_);
    datasource_box_->setEnabled(expert_mode);

    assert(range_box_);
    range_box_->setEnabled(expert_mode);

    assert(azimuth_box_);
    azimuth_box_->setEnabled(expert_mode);

    assert(altitude_box_);
    altitude_box_->setEnabled(expert_mode);

    assert(latitude_box_);
    latitude_box_->setEnabled(expert_mode);

    assert(longitude_box_);
    longitude_box_->setEnabled(expert_mode);
}

void RadarPlotPositionCalculatorTaskWidget::setDBOBject(const std::string& object_name)
{
    if (task_.dbObjectStr() != object_name)
        task_.dbObjectStr(object_name);

    key_box_->showDBOOnly(object_name);
    datasource_box_->showDBOOnly(object_name);
    range_box_->showDBOOnly(object_name);
    azimuth_box_->showDBOOnly(object_name);
    altitude_box_->showDBOOnly(object_name);

    latitude_box_->showDBOOnly(object_name);
    longitude_box_->showDBOOnly(object_name);
}
