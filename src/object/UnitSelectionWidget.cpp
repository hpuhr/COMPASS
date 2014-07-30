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

/*
 * UnitSelectionWidget.cpp
 *
 *  Created on: Oct 24, 2012
 *      Author: sk
 */

#include "UnitSelectionWidget.h"
#include "UnitManager.h"
#include "Logger.h"

UnitSelectionWidget::UnitSelectionWidget(std::string &unit_dimension, std::string &unit_unit)
 : QPushButton (),unit_dimension_(unit_dimension), unit_unit_(unit_unit)
{
  logdbg  << "UnitSelectionWidget: constructor";
  setText (unit_unit_.c_str());
  createMenu();

  connect( &menu_, SIGNAL(triggered(QAction*)), this, SLOT(triggerSlot(QAction*)));
  connect( this, SIGNAL(clicked()), this, SLOT(showMenuSlot()) );

}

UnitSelectionWidget::~UnitSelectionWidget()
{
}

void UnitSelectionWidget::createMenu()
{
  logdbg  << "UnitSelectionWidget: createMenu";
  std::map <std::string, Unit*> &units = UnitManager::getInstance().getUnits();
  std::map <std::string, Unit*>::iterator it;

  //menu_.addMenu("");
  menu_.addAction( "" );
  for (it = units.begin(); it != units.end(); it++)
  {
    std::map <std::string, double> &unit_units = it->second->getUnits ();
    std::map <std::string, double>::iterator unitit;

    if (unit_units.size() > 0)
    {
//      loginf  << "UnitSelectionWidget: createMenu: unit " << it->first;
      QMenu* m2 = menu_.addMenu( QString::fromStdString(it->first));

      for (unitit = unit_units.begin(); unitit != unit_units.end(); unitit++)
      {
//        loginf  << "UnitSelectionWidget: createMenu: unitunit " << unitit->first;
        QAction* action = m2->addAction( QString::fromStdString(unitit->first) );

        QVariantMap vmap;
        vmap.insert( QString::fromStdString(unitit->first), QVariant( QString::fromStdString(it->first) ) );
        action->setData( QVariant( vmap ) );
      }
    }
  }

//  loginf  << "UnitSelectionWidget: createMenu: end";
}

void UnitSelectionWidget::showMenuSlot()
{
  menu_.exec( QCursor::pos() );
}

void UnitSelectionWidget::triggerSlot( QAction* action )
{
  QVariantMap vmap = action->data().toMap();
  std::string unitunit, unit;

  if (action->text().size() != 0)
  {
    unitunit = vmap.begin().key().toStdString();
    unit = vmap.begin().value().toString().toStdString();
  }

  loginf  << "UnitSelectionWidget: triggerSlot: got unit " << unit << " unitunit " << unitunit;

  unit_dimension_ = unit;
  unit_unit_ = unitunit;

  setText (QString::fromStdString(unitunit));

//  emit selectionChanged();
}
