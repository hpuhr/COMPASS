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

#include "DBOVariableOrderedSetWidget.h"

DBOVariableOrderedSetWidget::DBOVariableOrderedSetWidget(DBOVariableOrderedSet *set, QWidget * parent, Qt::WindowFlags f)
: QWidget (parent, f), set_(set), list_widget_(0)
{
    assert (set_);
    //
    //  set_.add(set);
    //  set_.setChanged (false);

    createElements ();
    updateEntries ();
}

DBOVariableOrderedSetWidget::~DBOVariableOrderedSetWidget()
{

}

void DBOVariableOrderedSetWidget::createElements ()
{
    QVBoxLayout *main_layout = new QVBoxLayout ();

    QFont font_bold;
    font_bold.setBold(true);

    QHBoxLayout *hvars_layout = new QHBoxLayout();
    QVBoxLayout *vvars_layout = new QVBoxLayout();

    QLabel *var_label = new QLabel ("Variables");
    var_label->setFont (font_bold);
    main_layout->addWidget (var_label);

    list_widget_ = new QListWidget ();
    updateVariableList ();
    vvars_layout->addWidget (list_widget_);
    hvars_layout->addLayout (vvars_layout);

    QVBoxLayout *vupdown_layout = new QVBoxLayout();

    QPixmap* pixmapdown = new QPixmap("./Data/icons/collapse.png");
    QPixmap* pixmapup = new QPixmap("./Data/icons/expand.png");

    QPushButton *down = new QPushButton ();
    down->setIcon(QIcon(*pixmapdown));
    down->setFixedSize ( 20, 20 );
    down->setFlat(true);
    down->setToolTip(tr("Move variable forward"));
    connect( down, SIGNAL(clicked()), this, SLOT(moveDown()) );

    vupdown_layout->addWidget (down);


    vupdown_layout->addStretch();

    QPushButton *up = new QPushButton ();
    up->setIcon(QIcon(*pixmapup));
    up->setFixedSize ( 20, 20 );
    up->setFlat(true);
    up->setToolTip(tr("Move variable back"));
    connect( up, SIGNAL(clicked()), this, SLOT(moveUp()) );

    vupdown_layout->addWidget (up);

    hvars_layout->addLayout (vupdown_layout);

    main_layout->addLayout (hvars_layout);

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

void DBOVariableOrderedSetWidget::updateEntries()
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

        logdbg  << "DBOVariableOrderedSetWidget: updateEntries: looking for type " << type << " contained " << ATSDB::getInstance().contains (type);

        if (!ATSDB::getInstance().contains (type))
            continue;

        typestr = DBObjectManager::getInstance().getDBObject( type )->getName();

        logdbg  << "DBOVariableOrderedSetWidget: updateEntries: found dbo " << typestr;

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

            assert (false);
            // TODO FIXXMEE

//            QVariantMap vmap;
//            vmap.insert( str, QVariant( type ) );
//            action->setData( QVariant( vmap ) );
        }
    }
}

void DBOVariableOrderedSetWidget::showMenuSlot()
{
    menu_.exec( QCursor::pos() );
}

/*
 */
void DBOVariableOrderedSetWidget::triggerSlot( QAction* action )
{
    assert (false);
    // TODO FIXXMEE

//    QVariantMap vmap = action->data().toMap();
//    std::string id = vmap.begin().key().toStdString();
//    DB_OBJECT_TYPE type = (DB_OBJECT_TYPE)( vmap.begin().value().toUInt() );

//    assert (DBObjectManager::getInstance().existsDBOVariable(type, id));

//    set_->add (DBObjectManager::getInstance().getDBOVariable(type, id));

//    if (set_->getChanged())
//    {
//        updateVariableList ();
//        set_->setChanged (false);
//        emit setChanged();
//    }
}

void DBOVariableOrderedSetWidget::remove ()
{
    assert (list_widget_);
    int index = list_widget_->currentRow ();

    if (index < 0)
        return;

    set_->removeVariableAt (index);

    updateVariableList ();
    set_->setChanged (false);
    emit setChanged();
}

void DBOVariableOrderedSetWidget::moveUp ()
{
    assert (list_widget_);
    int index = list_widget_->currentRow ();

    if (index < 0 || index == (int)set_->getSize()-1)
        return;

    set_->moveVariableUp (index);

    updateVariableList ();
    list_widget_->setCurrentRow(index+1);

    set_->setChanged (false);
    emit setChanged();
}
void DBOVariableOrderedSetWidget::moveDown ()
{
    assert (list_widget_);
    int index = list_widget_->currentRow ();

    if (index <= 0)
        return;

    set_->moveVariableDown (index);

    updateVariableList ();
    list_widget_->setCurrentRow(index-1);

    set_->setChanged (false);
    emit setChanged();
}

void DBOVariableOrderedSetWidget::updateVariableList ()
{
    assert (list_widget_);
    list_widget_->clear();

    std::vector <DBOVariable*> &variables = set_->getSet ();
    std::vector <DBOVariable*>::iterator it;

    for (it = variables.begin(); it != variables.end(); it++)
    {
        assert (DBObjectManager::getInstance().existsDBObject ((*it)->getDBOType()));
        list_widget_->addItem ((DBObjectManager::getInstance().getDBObject ((*it)->getDBOType())->getName()+", "+(*it)->getName()).c_str());
    }
}
