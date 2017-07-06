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


#include "dbovariableselectionwidget.h"
#include "dbobjectmanager.h"

#include "global.h"

#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include "atsdb.h"
#include "dbobject.h"


/*
 */
DBOVariableSelectionWidget::DBOVariableSelectionWidget (bool show_title, bool h_box, QWidget* parent )
    :   QGroupBox (show_title ? "Select DBO Variable": "", parent), sel_var_(0), h_box_(h_box)
{
    createControls();
}

/*
 */
DBOVariableSelectionWidget::DBOVariableSelectionWidget (DBOVariable *init, bool show_title, bool h_box, QWidget* parent )
    :   QGroupBox (show_title ? "Select DBO Variable": ""), sel_var_( init ), h_box_(h_box)
{
    assert (sel_var_);

    createControls();

    QString str = QString::fromStdString(sel_var_->dbObject().name());

    sel_edit_type_->setText (str);
    sel_edit_id_->setText (QString::fromStdString( sel_var_->name()));
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

    QPixmap* pixmapmanage = new QPixmap("./data/icons/expand.png");
    sel_button_ = new QPushButton(this);
    sel_button_->setIcon(QIcon(*pixmapmanage));
    sel_button_->setFixedSize ( 25, 25 );

    if (h_box_)
    {
        layout = new QHBoxLayout;
        //QHBoxLayout *select_layout = new QHBoxLayout ();
        sel_edit_type_ = new QLabel (this);
        layout->addWidget( sel_edit_type_);

        //layout->addLayout (select_layout);

        sel_edit_id_ = new QLabel (this);
        layout->addWidget( sel_edit_id_);

        layout->addWidget(sel_button_);
    }
    else
    {
        layout = new QVBoxLayout;
        QHBoxLayout *select_layout = new QHBoxLayout ();
        sel_edit_type_ = new QLabel (this);
        select_layout->addWidget( sel_edit_type_);
        select_layout->addWidget(sel_button_);
        layout->addLayout (select_layout);

        sel_edit_id_ = new QLabel (this);
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

    for(auto object_it : ATSDB::instance().objectManager().objects())
    {
        auto variables = object_it.second->variables();

        QMenu* m2 = menu_.addMenu(QString::fromStdString(object_it.first));

        for (auto variable_it : variables)
        {
            QAction* action = m2->addAction(QString::fromStdString (variable_it.first));

            QVariantMap vmap;
            vmap.insert( QString::fromStdString (variable_it.first), QVariant (QString::fromStdString (object_it.first)));
            action->setData( QVariant( vmap ) );
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
      QVariantMap vmap = action->data().toMap();
      std::string var_name = vmap.begin().key().toStdString();
      std::string obj_name = vmap.begin().value().toString().toStdString();

      sel_var_ = &ATSDB::instance().objectManager().object(obj_name).variable(var_name);

      sel_edit_type_->setText (obj_name.c_str());
      sel_edit_id_->setText( action->text() );

      emit selectionChanged();
}

void DBOVariableSelectionWidget::setSelectedVariable (DBOVariable *var)
{
    sel_edit_type_->setText (QString::fromStdString(var->dbObject().name()));
    sel_edit_id_->setText (var->name().c_str());

    sel_var_=var;
}

/*
 */
DBOVariable* DBOVariableSelectionWidget::getSelectedVariable() const
{
    assert (sel_var_);
    return sel_var_;
}
