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
#include "files.h"

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>

using namespace Utils;

DBOVariableOrderedSetWidget::DBOVariableOrderedSetWidget(DBOVariableOrderedSet& set, QWidget* parent, Qt::WindowFlags f)
    :QWidget (parent, f), set_(set)
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


    // up/down buttons
    {
        QVBoxLayout *vupdown_layout = new QVBoxLayout();

        QPushButton *up = new QPushButton ();
        up->setIcon(QIcon(Files::getIconFilepath("up.png").c_str()));
        up->setFixedSize (UI_ICON_SIZE);
        up->setFlat(UI_ICON_BUTTON_FLAT);
        up->setToolTip(tr("Move variable up"));
        connect(up, &QPushButton::clicked, this, &DBOVariableOrderedSetWidget::moveUpSlot);

        vupdown_layout->addWidget (up);

        vupdown_layout->addStretch();

        QPushButton *down = new QPushButton ();
        down->setIcon(QIcon(Files::getIconFilepath("down.png").c_str()));
        down->setFixedSize (UI_ICON_SIZE);
        down->setFlat(UI_ICON_BUTTON_FLAT);
        down->setToolTip(tr("Move variable down"));
        connect (down, &QPushButton::clicked, this, &DBOVariableOrderedSetWidget::moveDownSlot);

        vupdown_layout->addWidget (down);

        hvars_layout->addLayout (vupdown_layout);
    }

    main_layout->addLayout (hvars_layout);

    // buttons
    {
        QHBoxLayout *button_layout = new QHBoxLayout ();

        QPushButton *remove_button = new QPushButton ();
        remove_button->setText( "Remove" );
        connect (remove_button, &QPushButton::clicked, this, &DBOVariableOrderedSetWidget::removeSlot);
        button_layout->addWidget( remove_button);

        QPushButton *add_button = new QPushButton();
        add_button->setText( "Add" );
        connect (add_button, &QPushButton::clicked, this, &DBOVariableOrderedSetWidget::showMenuSlot);
        button_layout->addWidget( add_button);

        main_layout->addLayout (button_layout);
    }

    connect (&menu_, &QMenu::triggered, this, &DBOVariableOrderedSetWidget::triggerSlot);

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

    for (auto& object_it : ATSDB::instance().objectManager())
    {

        QMenu* m2 = menu_.addMenu(QString::fromStdString(object_it.first));

        for (auto& var_it : *object_it.second)
        {
            QAction* action = m2->addAction(QString::fromStdString (var_it.first));

            QVariantMap vmap;
            vmap.insert (QString::fromStdString (var_it.first), QVariant (QString::fromStdString (object_it.first)));
            action->setData (QVariant (vmap));
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

void DBOVariableOrderedSetWidget::removeSlot ()
{
    assert (list_widget_);
    int index = list_widget_->currentRow ();

    loginf << "DBOVariableOrderedSetWidget: remove: index " << index;

    if (index < 0)
        return;

    set_.removeVariableAt (index);
    current_index_ = -1;
}

void DBOVariableOrderedSetWidget::moveUpSlot ()
{
    assert (list_widget_);
    int index = list_widget_->currentRow ();
    loginf << "DBOVariableOrderedSetWidget: moveUp: index " << index;

    if (index <= 0)
        return;

    set_.moveVariableUp (index);

    current_index_ = index-1;
    list_widget_->setCurrentRow(current_index_);
}
void DBOVariableOrderedSetWidget::moveDownSlot ()
{
    assert (list_widget_);
    int index = list_widget_->currentRow ();
    loginf << "DBOVariableOrderedSetWidget: moveDown: index " << index;

    if (index < 0 || index == (int)set_.getSize()-1)
        return;

    set_.moveVariableDown (index);

    current_index_ = index+1;
    list_widget_->setCurrentRow(current_index_);
}

void DBOVariableOrderedSetWidget::updateVariableListSlot ()
{
    logdbg << "DBOVariableOrderedSetWidget: updateVariableListSlot";

    assert (list_widget_);

    list_widget_->clear();

    logdbg << "DBOVariableOrderedSetWidget: updateVariableListSlot: clear done";

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
        logdbg << "DBOVariableOrderedSetWidget: updateVariableListSlot: current index " << current_index_;
        list_widget_->setCurrentRow(current_index_);
        current_index_=-1;
    }
}
