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

#include "radarplotpositioncalculatortaskwidget.h"
#include "radarplotpositioncalculatortask.h"
#include "dbobjectcombobox.h"
#include "dbovariable.h"
#include "dbovariableselectionwidget.h"
#include "logger.h"
#include "atsdb.h"
#include "stringconv.h"
#include "projectionmanager.h"
#include "projectionmanagerwidget.h"
#include "dbobjectmanager.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

using namespace Utils::String;


RadarPlotPositionCalculatorTaskWidget::RadarPlotPositionCalculatorTaskWidget(RadarPlotPositionCalculatorTask& task,
                                                                             QWidget* parent, Qt::WindowFlags f)
: QWidget (parent, f), task_(task)
{
    setMinimumSize(QSize(800, 600));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    QLabel *main_label = new QLabel ("Calculate radar plot positions");
    main_label->setFont (font_big);
    main_layout->addWidget (main_label);

    main_layout->addWidget(ProjectionManager::instance().widget());

    QGridLayout *grid = new QGridLayout ();
    unsigned int row_cnt=0;

    grid->addWidget (new QLabel ("DBObject"), row_cnt, 0);

    object_box_ = new DBObjectComboBox (false);
    connect (object_box_, SIGNAL(changedObject()), this, SLOT(dbObjectChangedSlot()));
    grid->addWidget (object_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget (new QLabel ("Key Variable"), row_cnt, 0);
    key_box_ = new DBOVariableSelectionWidget ();
    connect (key_box_, SIGNAL(selectionChanged()), this, SLOT(anyVariableChangedSlot()));
    grid->addWidget (key_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget (new QLabel ("Data Source Variable"), row_cnt, 0);
    datasource_box_ = new DBOVariableSelectionWidget ();
    connect (datasource_box_, SIGNAL(selectionChanged()), this, SLOT(anyVariableChangedSlot()));
    grid->addWidget (datasource_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget (new QLabel ("Range Variable"), row_cnt, 0);
    range_box_ = new DBOVariableSelectionWidget ();
    connect (range_box_, SIGNAL(selectionChanged()), this, SLOT(anyVariableChangedSlot()));
    grid->addWidget (range_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget (new QLabel ("Azimuth Variable"), row_cnt, 0);
    azimuth_box_ = new DBOVariableSelectionWidget ();
    connect (azimuth_box_, SIGNAL(selectionChanged()), this, SLOT(anyVariableChangedSlot()));
    grid->addWidget (azimuth_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget (new QLabel ("Altitude Variable"), row_cnt, 0);
    altitude_box_ = new DBOVariableSelectionWidget ();
    connect (altitude_box_, SIGNAL(selectionChanged()), this, SLOT(anyVariableChangedSlot()));
    grid->addWidget (altitude_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget (new QLabel ("Latitude Variable"), row_cnt, 0);
    latitude_box_ = new DBOVariableSelectionWidget ();
    connect (latitude_box_, SIGNAL(selectionChanged()), this, SLOT(anyVariableChangedSlot()));
    grid->addWidget (latitude_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget (new QLabel ("Longitude"), row_cnt, 0);
    longitude_box_ = new DBOVariableSelectionWidget ();
    connect (longitude_box_, SIGNAL(selectionChanged()), this, SLOT(anyVariableChangedSlot()));
    grid->addWidget (longitude_box_, row_cnt, 1);

    main_layout->addLayout(grid);

    calc_button_ = new QPushButton ("Calculate");
    connect(calc_button_, SIGNAL( clicked() ), this, SLOT( calculateSlot() ));
    main_layout->addWidget(calc_button_);

    setLayout (main_layout);

    update();

    show();
}

RadarPlotPositionCalculatorTaskWidget::~RadarPlotPositionCalculatorTaskWidget()
{
}

void RadarPlotPositionCalculatorTaskWidget::update ()
{
    std::string object_name = task_.dbObjectStr();

    if (object_name.size())
    {
        assert (object_box_);
        object_box_->setObjectName(object_name);
        setDBOBject (object_name);

        assert (ATSDB::instance().objectManager().existsObject(object_name));
        DBObject &object = ATSDB::instance().objectManager().object(object_name);

        assert (key_box_);
        if (task_.keyVarStr().size() && object.hasVariable(task_.keyVarStr()))
            key_box_->selectedVariable(object.variable(task_.keyVarStr()));

        assert (datasource_box_);
        if (task_.datasourceVarStr().size() && object.hasVariable(task_.datasourceVarStr()))
            datasource_box_->selectedVariable(object.variable(task_.datasourceVarStr()));

        assert (range_box_);
        if (task_.rangeVarStr().size() && object.hasVariable(task_.rangeVarStr()))
            range_box_->selectedVariable(object.variable(task_.rangeVarStr()));

        assert (azimuth_box_);
        if (task_.azimuthVarStr().size() && object.hasVariable(task_.azimuthVarStr()))
            azimuth_box_->selectedVariable(object.variable(task_.azimuthVarStr()));

        assert (altitude_box_);
        if (task_.altitudeVarStr().size() && object.hasVariable(task_.altitudeVarStr()))
            altitude_box_->selectedVariable(object.variable(task_.altitudeVarStr()));

        assert (latitude_box_);
        if (task_.latitudeVarStr().size() && object.hasVariable(task_.latitudeVarStr()))
            latitude_box_->selectedVariable(object.variable(task_.latitudeVarStr()));

        assert (longitude_box_);
        if (task_.longitudeVarStr().size() && object.hasVariable(task_.longitudeVarStr()))
            longitude_box_->selectedVariable(object.variable(task_.longitudeVarStr()));
    }
}

void RadarPlotPositionCalculatorTaskWidget::dbObjectChangedSlot()
{
    assert (object_box_);

    std::string object_name = object_box_->getObjectName();

    loginf << "RadarPlotPositionCalculatorTaskWidget: dbObjectChangedSlot: " << object_name;
    setDBOBject (object_name);
}

void RadarPlotPositionCalculatorTaskWidget::anyVariableChangedSlot()
{
    assert (key_box_);
    if (key_box_->hasVariable())
        task_.keyVarStr(key_box_->selectedVariable().name());
    else
        task_.keyVarStr("");

    assert (datasource_box_);
    if (datasource_box_->hasVariable())
        task_.datasourceVarStr(datasource_box_->selectedVariable().name());
    else
        task_.datasourceVarStr("");

    assert (range_box_);
    if (range_box_->hasVariable())
        task_.rangeVarStr(range_box_->selectedVariable().name());
    else
        task_.rangeVarStr("");

    assert (azimuth_box_);
    if (azimuth_box_->hasVariable())
        task_.azimuthVarStr(azimuth_box_->selectedVariable().name());
    else
        task_.azimuthVarStr("");

    assert (altitude_box_);
    if (altitude_box_->hasVariable())
        task_.altitudeVarStr(altitude_box_->selectedVariable().name());
    else
        task_.altitudeVarStr("");

    assert (latitude_box_);
    if (latitude_box_->hasVariable())
        task_.latitudeVarStr(latitude_box_->selectedVariable().name());
    else
        task_.latitudeVarStr("");

    assert (longitude_box_);
    if (longitude_box_->hasVariable())
        task_.longitudeVarStr(longitude_box_->selectedVariable().name());
    else
        task_.longitudeVarStr("");
}

void RadarPlotPositionCalculatorTaskWidget::calculateSlot ()
{
    loginf << "RadarPlotPositionCalculatorTaskWidget: calculateSlot";

    if (!task_.canCalculate())
    {
        QMessageBox::warning (this, "Unable to Calculate",
                              "The task can not be peformed with the entered items.\n"
                              "The following conditions have to be met: The DBObject must exist, must have data, and"
                              " all variables have to be set and exist in the current schema and database");
        return;
    }

    assert (calc_button_);

    std::string db_object_str = task_.dbObjectStr();
    DBObjectManager& obj_man = ATSDB::instance().objectManager();

    assert (obj_man.existsObject(db_object_str));
    DBObject& db_object = obj_man.object(db_object_str);

    std::map<int, DBODataSource>& data_sources = db_object.dataSources();

    bool not_final = false;
    for (auto& ds_it : data_sources)
    {
        ds_it.second.finalize();
        if (!ds_it.second.isFinalized())
        {
            not_final = true;
            break;
        }
    }

    if (not_final)
    {
        QMessageBox::warning (this, "EPSG Value Wrong",
                              "The coordinates of the data sources of selected database object could not be calculated."
                              " Please select a suitable EPSG value and try again");
        return;
    }

    loginf << "RadarPlotPositionCalculatorTaskWidget: calculateSlot: starting calculation";

    calc_button_->setDisabled(true);

    assert (!task_.isCalculating());
    task_.calculate();
}

void RadarPlotPositionCalculatorTaskWidget::calculationDoneSlot ()
{
    assert (calc_button_);
    calc_button_->setDisabled(false);
}


void RadarPlotPositionCalculatorTaskWidget::setDBOBject (const std::string& object_name)
{
    task_.dbObjectStr(object_name);

    key_box_->showDBOOnly(object_name);
    datasource_box_->showDBOOnly(object_name);
    range_box_->showDBOOnly(object_name);
    azimuth_box_->showDBOOnly(object_name);
    altitude_box_->showDBOOnly(object_name);

    latitude_box_->showDBOOnly(object_name);
    longitude_box_->showDBOOnly(object_name);
}
