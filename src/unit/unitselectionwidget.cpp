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

#include "unitselectionwidget.h"
#include "unitmanager.h"
#include "unit.h"
#include "quantity.h"
#include "logger.h"

UnitSelectionWidget::UnitSelectionWidget(const std::string &quantity, const std::string &unit)
 : QPushButton (),quantity_(quantity), unit_(unit)
{
  logdbg  << "UnitSelectionWidget: constructor";

  if (quantity_.size() > 0)
    setText (QString::fromStdString(quantity_)+":"+QString::fromStdString(unit_));

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
  menu_.addAction( "" );

  for (auto it : UnitManager::instance().quantities())
  {
    const std::map <std::string, Unit*> &units = it.second->units();

    if (units.size() > 0)
    {
//      loginf  << "UnitSelectionWidget: createMenu: unit " << it->first;
      QMenu* m2 = menu_.addMenu( QString::fromStdString(it.first));

      for (auto it2: units)
      {
//        loginf  << "UnitSelectionWidget: createMenu: unitunit " << unitit->first;
        QAction* action = m2->addAction( QString::fromStdString(it2.first) );

        QVariantMap vmap;
        vmap.insert( QString::fromStdString(it2.first), QVariant( QString::fromStdString(it.first) ) );
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
  std::string quantity, unit;

  if (action->text().size() != 0)
  {
    quantity = vmap.begin().value().toString().toStdString();
    unit = vmap.begin().key().toStdString();
  }

  loginf  << "UnitSelectionWidget: triggerSlot: got quantity " << quantity << " unit " << unit;

  quantity_ = quantity;
  unit_ = unit;

  if (quantity_.size() > 0)
    setText (QString::fromStdString(quantity)+":"+QString::fromStdString(unit_));
  else
      setText ("");

//  emit selectionChanged();
}
