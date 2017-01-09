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
 * MainWidget.h
 *
 *  Created on: Jun 25, 2014
 *      Author: sk
 */

#ifndef MAINWIDGET_H_
#define MAINWIDGET_H_

#include "Configurable.h"

#include <QWidget>

//namespace MFImport
//{

//class GPSTrailManagerWidget;
//class MeasurementFlightManagerWidget;

/**
 * @brief Main widget containing other important widgets.
 *
 * So I heard you liked widgets...
 */
class MainWidget : public QWidget //, public Configurable
{
public:
    /// @brief Constructor.
    MainWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    /// @brief Destructor.
    virtual ~MainWidget();

protected:
    //GPSTrailManagerWidget *trail_widget_;
    //MeasurementFlightManagerWidget *flight_widget_;

    /// @brief Creates GUI elements.
    void createGUIElements ();
};

//} /* namespace MFImport */

#endif /* MAINWIDGET_H_ */
