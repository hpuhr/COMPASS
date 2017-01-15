/*
 * This file is part of MFImport.
 *
 * MFImport is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MFImport is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with MFImport.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * MainWidget.cpp
 *
 *  Created on: Jun 25, 2014
 *      Author: sk
 */

#include <QVBoxLayout>
#include <QTabWidget>


#include "mainwidget.h"
//#include "GPSTrailManagerWidget.h"
//#include "MeasurementFlightManagerWidget.h"

//namespace MFImport
//{

MainWidget::MainWidget (QWidget* parent, Qt::WindowFlags f)
: QWidget (parent, f) //, trail_widget_(0), flight_widget_(0)
{
    createGUIElements ();
}

MainWidget::~MainWidget()
{
//    if (trail_widget_)
//        delete trail_widget_;
//    trail_widget_=0;

//    if (flight_widget_)
//        delete flight_widget_;
//    flight_widget_=0;

}

void MainWidget::createGUIElements ()
{
    QVBoxLayout *layout = new QVBoxLayout ();

    QTabWidget *tab = new QTabWidget ();

//    trail_widget_ = new GPSTrailManagerWidget ();
//    //layout->addWidget (trail_widget_);
//    tab->addTab(trail_widget_, "GPS Trails");

    QWidget *manage_widget = new QWidget ();
    QVBoxLayout *manage_layout = new QVBoxLayout ();

//    flight_widget_ = new MeasurementFlightManagerWidget ();
//    manage_layout->addWidget (flight_widget_);
//    connect (trail_widget_, SIGNAL (convert(GPSTrail*)), flight_widget_, SLOT(convert(GPSTrail*)));

    //connect (flight_widget_, SIGNAL (insertFlight(std::string)), insert_widget_, SLOT(updateCurrentMeasurementFlight(std::string)));

    manage_widget->setLayout (manage_layout);
    tab->addTab(manage_widget, "Measurement Flights");

    layout->addWidget (tab);

    setLayout (layout);
}

//} /* namespace MFImport */
