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


#include "DBOVariableSelectionWidget.h"
#include "DBObjectManager.h"

#include "Global.h"

#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>
#include "ATSDB.h"
#include "DBObject.h"


/*
 */
DBOVariableSelectionWidget::DBOVariableSelectionWidget( bool h_box, QWidget* parent )
    :   QGroupBox( "Select DBO Variable", parent ), sel_var_(0), h_box_(h_box)
{
    createControls();
}

/*
 */
DBOVariableSelectionWidget::DBOVariableSelectionWidget( DBOVariable *init, bool h_box, QWidget* parent )
    :   QGroupBox( "Select DBO Variable", parent ), sel_var_( init ), h_box_(h_box)
{
    assert (sel_var_);

    createControls();

    QString str = QString::fromStdString( DBObjectManager::getInstance().getDBObject( sel_var_->getDBOType() )->getName() );

    sel_edit_type_->setText( str );
    sel_edit_id_->setText( QString::fromStdString( sel_var_->getId() ) );
}

/*
 */
DBOVariableSelectionWidget::~DBOVariableSelectionWidget()
{
}

/*
 */
void DBOVariableSelectionWidget::createControls()
{
    QBoxLayout *layout;

    QPixmap* pixmapmanage = new QPixmap("./Data/icons/expand.png");
    sel_button_ = new QPushButton(this);
    sel_button_->setIcon(QIcon(*pixmapmanage));
    sel_button_->setFixedSize ( 25, 25 );

    if (h_box_)
    {
        layout = new QHBoxLayout;
        //QHBoxLayout *select_layout = new QHBoxLayout ();
        sel_edit_type_ = new QLineEdit( this );
        sel_edit_type_->setReadOnly( true );
        layout->addWidget( sel_edit_type_);

        //layout->addLayout (select_layout);

        sel_edit_id_ = new QLineEdit( this );
        sel_edit_id_->setReadOnly( true );
        layout->addWidget( sel_edit_id_);

        layout->addWidget(sel_button_);
    }
    else
    {
        layout = new QVBoxLayout;
        QHBoxLayout *select_layout = new QHBoxLayout ();
        sel_edit_type_ = new QLineEdit( this );
        sel_edit_type_->setReadOnly( true );
        select_layout->addWidget( sel_edit_type_);
        select_layout->addWidget(sel_button_);
        layout->addLayout (select_layout);

        sel_edit_id_ = new QLineEdit( this );
        sel_edit_id_->setReadOnly( true );
        layout->addWidget( sel_edit_id_);

    }
    setLayout( layout );

    connect( &menu_, SIGNAL(triggered(QAction*)), this, SLOT(triggerSlot(QAction*)));
    connect( sel_button_, SIGNAL(clicked()), this, SLOT(showMenuSlot()) );

    updateEntries();
}

/*
 */
void DBOVariableSelectionWidget::updateEntries()
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

        logdbg  << "DBOVariableSelectionWidget: updateEntries: looking for type " << type << " contained " << ATSDB::getInstance().contains (type);

        if (!ATSDB::getInstance().contains (type))
            continue;

        typestr = DBObjectManager::getInstance().getDBObject( type )->getName();

        logdbg  << "DBOVariableSelectionWidget: updateEntries: found dbo " << typestr;

        std::map<std::string,DBOVariable*>& vars = DBObjectManager::getInstance().getDBOVariables( type );

        QMenu* m2 = menu_.addMenu( QString::fromStdString( typestr ) );

        itend = vars.end();
        for( it=vars.begin(); it!=itend; ++it )
        {
            //      if( it->second->data_type_int_ == P_TYPE_STRING )
            //        continue;
            // HACK HACK

            str = QString::fromStdString( it->first );
            if( str == "id" )
                continue;

            QAction* action = m2->addAction( str );

            assert (false);
            // TODO FIXMEE

            //      QVariantMap vmap;
            //      vmap.insert( str, QVariant( type ) );
            //      action->setData( QVariant( vmap ) );
        }
    }
}

/*
 */
void DBOVariableSelectionWidget::showMenuSlot()
{
    menu_.exec( QCursor::pos() );
}

/*
 */
void DBOVariableSelectionWidget::triggerSlot( QAction* action )
{
    assert (false);
    // TODO FIXMEE

    //  QVariantMap vmap = action->data().toMap();
    //  std::string id = vmap.begin().key().toStdString();
    //  DB_OBJECT_TYPE type = (DB_OBJECT_TYPE)( vmap.begin().value().toUInt() );

    //  sel_var_ = DBObjectManager::getInstance().getDBOVariable( type, id );

    //  QString str = QString::fromStdString( DBObjectManager::getInstance().getDBObject( type )->getName() );

    //  sel_edit_type_->setText( str );
    //  sel_edit_id_->setText( action->text() );

    //  emit selectionChanged();
}

void DBOVariableSelectionWidget::setSelectedVariable (DBOVariable *var)
{
    const std::string &type = var->getDBOType();
    std::string name = var->getId();

    QString str = QString::fromStdString( DBObjectManager::getInstance().getDBObject( type )->getName() );

    sel_edit_type_->setText( str );
    sel_edit_id_->setText( name.c_str() );

    sel_var_=var;
}

/*
 */
DBOVariable* DBOVariableSelectionWidget::getSelectedVariable() const
{
    assert (sel_var_);
    return sel_var_;
}
