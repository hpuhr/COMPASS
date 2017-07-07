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

#include "dbobjectmanager.h"
#include "atsdb.h"
#include "dbobject.h"
#include "global.h"
#include "dbovariableorderedsetwidget.h"

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>

DBOVariableOrderedSetWidget::DBOVariableOrderedSetWidget(DBOVariableOrderedSet &set, QWidget * parent, Qt::WindowFlags f)
: QWidget (parent, f), set_(set), list_widget_(nullptr), current_index_(-1)
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
    updateVariableListSlot ();
    vvars_layout->addWidget (list_widget_);
    hvars_layout->addLayout (vvars_layout);

    QVBoxLayout *vupdown_layout = new QVBoxLayout();

    QPixmap* pixmapdown = new QPixmap("./data/icons/collapse.png");
    QPixmap* pixmapup = new QPixmap("./data/icons/expand.png");

    QPushButton *down = new QPushButton ();
    down->setIcon(QIcon(*pixmapdown));
    down->setFixedSize (UI_ICON_SIZE);
    down->setFlat(UI_ICON_BUTTON_FLAT);
    down->setToolTip(tr("Move variable down"));
    connect( down, SIGNAL(clicked()), this, SLOT(moveDown()) );

    vupdown_layout->addWidget (down);


    vupdown_layout->addStretch();

    QPushButton *up = new QPushButton ();
    up->setIcon(QIcon(*pixmapup));
    up->setFixedSize (UI_ICON_SIZE);
    up->setFlat(UI_ICON_BUTTON_FLAT);
    up->setToolTip(tr("Move variable up"));
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
    updateMenuEntries ();
}

DBOVariableOrderedSetWidget::~DBOVariableOrderedSetWidget()
{

}

void DBOVariableOrderedSetWidget::updateMenuEntries()
{
    menu_.clear();

    QMenu* meta_menu = menu_.addMenu(QString::fromStdString (META_OBJECT_NAME));
    for (auto meta_it : ATSDB::instance().objectManager().metaVariables())
    {
        QAction* action = meta_menu->addAction(QString::fromStdString (meta_it.first));

        QVariantMap vmap;
        vmap.insert (QString::fromStdString (meta_it.first), QVariant (QString::fromStdString (META_OBJECT_NAME)));
        action->setData (QVariant(vmap));

    }

    for (auto object_it : ATSDB::instance().objectManager().objects())
    {

        auto variables = object_it.second->variables();

        QMenu* m2 = menu_.addMenu(QString::fromStdString(object_it.first));

        for (auto variable_it : variables)
        {
            QAction* action = m2->addAction(QString::fromStdString (variable_it.first));

            QVariantMap vmap;
            vmap.insert (QString::fromStdString (variable_it.first), QVariant (QString::fromStdString (object_it.first)));
            action->setData (QVariant (vmap));
        }
    }

//    menu_.clear();

//    QString str;
//    std::string typestr;
//    std::map<std::string,DBOVariable*>::const_iterator it, itend;

//    DB_OBJECT_TYPE type;

//    std::map <DB_OBJECT_TYPE, DBObject*> &dobs = DBObjectManager::getInstance().getDBObjects ();
//    std::map <DB_OBJECT_TYPE, DBObject*>::iterator dobit;

//    for( dobit = dobs.begin(); dobit != dobs.end(); dobit++ )
//    {
//        type = dobit->second->getType();

//        logdbg  << "DBOVariableOrderedSetWidget: updateEntries: looking for type " << type << " contained " << ATSDB::getInstance().contains (type);

//        if (!ATSDB::getInstance().contains (type))
//            continue;

//        typestr = DBObjectManager::getInstance().getDBObject( type )->getName();

//        logdbg  << "DBOVariableOrderedSetWidget: updateEntries: found dbo " << typestr;

//        std::map<std::string,DBOVariable*>& vars = DBObjectManager::getInstance().getDBOVariables( type );

//        QMenu* m2 = menu_.addMenu( QString::fromStdString( typestr ) );

//        itend = vars.end();
//        for( it=vars.begin(); it!=itend; ++it )
//        {
//            //      if( it->second->data_type_int_ == P_TYPE_STRING )
//            //        continue;

//            str = QString::fromStdString( it->first );
//            //      if( str == "id" )
//            //        continue;

//            QAction* action = m2->addAction( str );

//            QVariantMap vmap;
//            vmap.insert( str, QVariant( type ) );
//            action->setData( QVariant( vmap ) );
//        }
//    }
}

void DBOVariableOrderedSetWidget::showMenuSlot()
{
    menu_.exec( QCursor::pos() );
}

/*
 */
void DBOVariableOrderedSetWidget::triggerSlot( QAction* action )
{
    QVariantMap vmap = action->data().toMap();
    std::string var_name = vmap.begin().key().toStdString();
    std::string obj_name = vmap.begin().value().toString().toStdString();

    DBObjectManager &manager = ATSDB::instance().objectManager();

    if (obj_name == META_OBJECT_NAME)
    {
        assert (manager.existsMetaVariable(var_name));
        set_.add(manager.metaVariable(var_name));
    }
    else
    {
        assert (manager.existsObject(obj_name));
        assert (manager.object(obj_name).hasVariable(var_name));
        set_.add (manager.object(obj_name).variable(var_name));
    }
}

void DBOVariableOrderedSetWidget::remove ()
{
    assert (list_widget_);
    int index = list_widget_->currentRow ();

    if (index < 0)
        return;

    set_.removeVariableAt (index);
    current_index_ = -1;
}

void DBOVariableOrderedSetWidget::moveUp ()
{
    assert (list_widget_);
    int index = list_widget_->currentRow ();

    if (index < 0 || index == (int)set_.getSize()-1)
        return;

    set_.moveVariableUp (index);

    current_index_ = index+1;
}
void DBOVariableOrderedSetWidget::moveDown ()
{
    assert (list_widget_);
    int index = list_widget_->currentRow ();

    if (index <= 0)
        return;

    set_.moveVariableDown (index);

    current_index_ = index-1;
}

void DBOVariableOrderedSetWidget::updateVariableListSlot ()
{
    assert (list_widget_);
    list_widget_->clear();

    const std::map <unsigned int, DBOVariableOrderDefinition*> &variables = set_.definitions();
    std::map <unsigned int, DBOVariableOrderDefinition*>::const_iterator it;

    DBObjectManager &manager = ATSDB::instance().objectManager();
    DBOVariableOrderDefinition *def=nullptr;

    for (it = variables.begin(); it != variables.end(); it++)
    {
        def = it->second;
        if (def->dboName() == META_OBJECT_NAME)
            assert (manager.existsMetaVariable(def->variableName()));
        else
        {
            assert (manager.existsObject(def->dboName()));
            assert (manager.object(def->dboName()).hasVariable(def->variableName()));
        }
        list_widget_->addItem ((def->dboName()+", "+def->variableName()).c_str());
    }

    if (current_index_ != -1)
    {
        list_widget_->setCurrentRow(current_index_);
        current_index_=-1;
    }
}
