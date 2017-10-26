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

#include <QAction>
#include <QWidget>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QPushButton>

#include "dbfilterwidget.h"
#include "dbfilter.h"
#include "logger.h"
#include "dbfiltercondition.h"
#include "global.h"
#include "atsdb.h"
#include "filtermanager.h"

/**
 * Initializes members, registers Parameter, creates GUI elements and the menu, calls update
 */
DBFilterWidget::DBFilterWidget(const std::string &class_id, const std::string &instance_id, DBFilter &filter)
    : QFrame (), Configurable (class_id, instance_id, &filter),filter_(filter), visible_checkbox_(0),
      active_checkbox_(0), manage_button_ (0), child_layout_ (0)
{
    logdbg  << "DBFilterWidget: constructor";

    registerParameter ("visible", &visible_, false);


    QVBoxLayout *main_layout = new QVBoxLayout ();
    main_layout->setContentsMargins (1, 1, 1, 1);
    main_layout->setSpacing (1);

    setFrameStyle(QFrame::Panel | QFrame::Raised);

    QHBoxLayout *config_layout = new QHBoxLayout ();
    config_layout->setContentsMargins (1, 1, 1, 1);
    config_layout->setSpacing (1);

    active_checkbox_ = new QCheckBox();
    connect( active_checkbox_, SIGNAL( clicked() ), this, SLOT( toggleActive() ) );
    config_layout->addWidget (active_checkbox_);

    visible_checkbox_ = new QCheckBox(tr(filter_.getName().c_str()));
    connect( visible_checkbox_, SIGNAL( clicked() ), this, SLOT( toggleVisible() ) );

    visible_checkbox_->setStyleSheet (" QCheckBox::indicator {  width: 12px; height: 12px; }  "
            "QCheckBox::indicator:checked   {     image: url(./data/icons/collapse.png);   }"
            "QCheckBox::indicator:unchecked   {     image: url(./data/icons/expand.png); }");

    config_layout->addWidget (visible_checkbox_);
    config_layout->addStretch();

    QPixmap* pixmapmanage = new QPixmap("./data/icons/edit.png");
    manage_button_ = new QPushButton ();
    manage_button_->setIcon(QIcon(*pixmapmanage));
    manage_button_->setFixedSize (UI_ICON_SIZE);
    manage_button_->setFlat(UI_ICON_BUTTON_FLAT);
    manage_button_->setToolTip(tr("Manage filter"));
    connect(manage_button_, SIGNAL( clicked() ), this, SLOT( showMenuSlot() ));
    config_layout->addWidget (manage_button_);
    main_layout->addLayout (config_layout);

    child_ = new QWidget ();
    child_->setVisible (visible_);

    child_layout_ = new QVBoxLayout ();
    child_layout_->setContentsMargins (5, 1, 1, 1);
    child_layout_->setSpacing (1);

    updateChildWidget();

    child_->setLayout (child_layout_);

    main_layout->addWidget (child_);
    setLayout (main_layout);

    connect (this, SIGNAL(deleteFilterSignal(DBFilter*)), &ATSDB::instance().filterManager(), SLOT(deleteFilterSlot(DBFilter*)), Qt::QueuedConnection);
    createMenu ();

    update();
}

/**
 * Tells the DBFilter that the widget has already been deleted.
 */
DBFilterWidget::~DBFilterWidget()
{

}

/**
 * Adds possible actions for a generic filter
 */
void DBFilterWidget::createMenu ()
{
    if (!filter_.isGeneric())
        return;

    QAction *reset_action = menu_.addAction(tr("Reset"));
    connect(reset_action, SIGNAL(triggered()), this, SLOT(reset()));

    QAction *edit_action = menu_.addAction(tr("Edit"));
    connect(edit_action, SIGNAL(triggered()), this, SLOT(filterEditSlot()));

    QAction *delete_action = menu_.addAction(tr("Delete"));
    connect(delete_action, SIGNAL(triggered()), this, SLOT(deleteFilter()));
}

/**
 * Adds a widget to the child layout
 */
void DBFilterWidget::addChildWidget (QWidget *widget)
{
    assert (widget);
    child_layout_->addWidget (widget);
}

/**
 * Removes all contents of the child layout, and adds all condition widgets of the filter
 */
void DBFilterWidget::updateChildWidget ()
{
    QLayoutItem *child;
    while ((child = child_layout_->takeAt(0)) != 0)
    {
        if (child->widget())
            child_layout_->removeWidget(child->widget());;
        child_layout_->removeItem(child);
        delete child;
    }

    std::vector <DBFilterCondition *> &conditions = filter_.getConditions();
    for (unsigned int cnt=0; cnt < conditions.size(); cnt++)
    {
        QWidget *child_widget = conditions.at(cnt)->getWidget();
        assert (child_widget);
        child_layout_->addWidget(child_widget);
        connect(conditions.at(cnt), SIGNAL( possibleFilterChange() ), this, SLOT( possibleSubFilterChange() ), Qt::UniqueConnection);
    }
}


void DBFilterWidget::toggleVisible()
{
    logdbg  << "DBFilterWidget: toggleVisible";
    visible_ = !visible_;
    child_->setVisible(visible_);
}

void DBFilterWidget::toggleAnd()
{
    logdbg << "DBFilterWidget: toggleAnd";

    //checked is or is false
    //unchecked is and is true
    /*
  filter_.setAnd(and_checkbox_->checkState() == Qt::Unchecked);
     */
    emit possibleFilterChange();
}

void DBFilterWidget::toggleActive()
{
    logdbg << "DBFilterWidget: toggleActive";
    filter_.setActive(active_checkbox_->checkState() == Qt::Checked);

    emit possibleFilterChange();
}

void DBFilterWidget::invert()
{
    logdbg << "DBFilterWidget: invert";
    filter_.invert();

    emit possibleFilterChange();
}

void DBFilterWidget::update (void)
{
    logdbg << "DBFilterWidget: update";

    visible_checkbox_->setText (filter_.getName().c_str());

    if (filter_.getActive())
        active_checkbox_->setChecked(true);
    else
        active_checkbox_->setChecked(false);

    if (visible_)
        visible_checkbox_->setChecked(true);
    else
        visible_checkbox_->setChecked(false);

    //  if (!filter_.getAnd())
    //    and_checkbox_->setChecked(Qt::Checked);
    //  else
    //    and_checkbox_->setChecked(Qt::Unchecked);

    //  std::vector <DBFilterCondition *> &conditions = filter_.getConditions();
    //  for (unsigned int cnt=0; cnt < conditions.size(); cnt++)
    //  {
    //    conditions.at(cnt)->update();
    //  }

}

void DBFilterWidget::possibleSubFilterChange ()
{
    logdbg  << "DBFilterWidget: possibleSubFilterChange";
    emit possibleFilterChange();
}

void DBFilterWidget::reset ()
{
    filter_.reset();
    updateChildWidget ();
    emit possibleFilterChange();
}

void DBFilterWidget::showMenuSlot()
{
    menu_.exec( QCursor::pos() );
}

void DBFilterWidget::deleteFilter()
{
    emit deleteFilterSignal (&filter_);
}

void DBFilterWidget::filterEditSlot ()
{
    logdbg  << "DBFilterWidget: filterEditSlot";
    emit filterEdit(&filter_);
}
