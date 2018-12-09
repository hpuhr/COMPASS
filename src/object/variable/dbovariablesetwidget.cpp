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

#include "DBObjectManager.h"
#include "ATSDB.h"
#include "DBObject.h"
#include "Global.h"

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>

#include "DBOVariableSetWidget.h"

DBOVariableSetWidget::DBOVariableSetWidget(DBOVariableSet &set, QWidget * parent, Qt::WindowFlags f)
 : QWidget (parent, f), list_widget_(0)
{

  set_.add(set);
  set_.setChanged (false);

  createElements ();
  updateEntries ();
}

DBOVariableSetWidget::~DBOVariableSetWidget()
{

}

void DBOVariableSetWidget::createElements ()
{
  QVBoxLayout *main_layout = new QVBoxLayout ();

  QFont font_bold;
  font_bold.setBold(true);

  QLabel *var_label = new QLabel ("Variables");
  var_label->setFont (font_bold);
  main_layout->addWidget (var_label);

  list_widget_ = new QListWidget ();
  updateVariableList ();
  main_layout->addWidget (list_widget_);

  QHBoxLayout *button_layout = new QHBoxLayout ();

  QPushButton *remove_button = new QPushButton ();
  remove_button->setText( "Remove" );
  connect( remove_button, SIGNAL(clicked()), this, SLOT(remove()) );
  button_layout->addWidget( remove_button);

  QPushButton *add_button = new QPushButton();
  add_button->setText( "Add" );
  connect( add_button, SIGNAL(clicked()), this, SLOT(showMenuSlot()) );
  button_layout->addWidget( add_button);

  main_layout->addLayout (button_layout);

  connect( &menu_, SIGNAL(triggered(QAction*)), this, SLOT(triggerSlot(QAction*)));

  setLayout (main_layout);
}

void DBOVariableSetWidget::updateEntries()
{
  menu_.clear();

  QString str;
  std::string typestr;
  std::map<std::string,DBOVariable*>::const_iterator it, itend;

  std::string type;

  const std::map <std::string, DBObject*> &dobs = DBObjectManager::getInstance().getDBObjects ();
  std::map <std::string, DBObject*>::const_iterator dobit;

  for( dobit = dobs.begin(); dobit != dobs.end(); dobit++ )
  {
    type = dobit->second->getType();

    logdbg  << "DBOVariableSetWidget: updateEntries: looking for type " << type << " contained " << ATSDB::getInstance().contains (type);

    if (!ATSDB::getInstance().contains (type))
      continue;

    typestr = DBObjectManager::getInstance().getDBObject( type )->getName();

    logdbg  << "DBOVariableSetWidget: updateEntries: found dbo " << typestr;

    std::map<std::string,DBOVariable*>& vars = DBObjectManager::getInstance().getDBOVariables( type );

    QMenu* m2 = menu_.addMenu( QString::fromStdString( typestr ) );

    itend = vars.end();
    for( it=vars.begin(); it!=itend; ++it )
    {
//      if( it->second->data_type_int_ == P_TYPE_STRING )
//        continue;

      str = QString::fromStdString( it->first );
//      if( str == "id" )
//        continue;

      QAction* action = m2->addAction( str );

      QVariantMap vmap;

      assert (false);
      //TODO FIXXMEE

      //vmap.insert( str, QVariant( type ) );
      action->setData( QVariant( vmap ) );
    }
  }
}

void DBOVariableSetWidget::showMenuSlot()
{
  menu_.exec( QCursor::pos() );
}

/*
 */
void DBOVariableSetWidget::triggerSlot( QAction* action )
{
  QVariantMap vmap = action->data().toMap();
  std::string id = vmap.begin().key().toStdString();

  //TODO FIXXMEE
  assert (false);

//  const std::string &type = (DB_OBJECT_TYPE)( vmap.begin().value().toUInt() );

//  assert (DBObjectManager::getInstance().existsDBOVariable(type, id));

//  set_.add (DBObjectManager::getInstance().getDBOVariable(type, id));

//  if (set_.getChanged())
//  {
//    updateVariableList ();
//    set_.setChanged (false);
//    emit setChanged();
//  }
}

void DBOVariableSetWidget::remove ()
{
  assert (list_widget_);
  int index = list_widget_->currentRow ();

  if (index < 0)
    return;

  set_.removeVariableAt (index);

  updateVariableList ();
  set_.setChanged (false);
  emit setChanged();
}

void DBOVariableSetWidget::updateVariableList ()
{
  assert (list_widget_);
  list_widget_->clear();

  std::vector <DBOVariable*> &variables = set_.getSet ();
  std::vector <DBOVariable*>::iterator it;

  for (it = variables.begin(); it != variables.end(); it++)
  {
    assert (DBObjectManager::getInstance().existsDBObject ((*it)->getDBOType()));
    list_widget_->addItem ((DBObjectManager::getInstance().getDBObject ((*it)->getDBOType())->getName()+", "+(*it)->getName()).c_str());
  }
}
