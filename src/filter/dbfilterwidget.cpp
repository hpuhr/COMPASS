/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "dbfilterwidget.h"

#include <QAction>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "compass.h"
#include "dbfilter.h"
#include "dbfiltercondition.h"
#include "files.h"
#include "filtermanager.h"
#include "global.h"
#include "logger.h"

using namespace Utils;

DBFilterWidget::DBFilterWidget(DBFilter& filter)
    : QFrame(), filter_(filter)
{
    logdbg << "DBFilterWidget: constructor";

    QVBoxLayout* main_layout = new QVBoxLayout();
    main_layout->setContentsMargins(1, 1, 1, 1);
    main_layout->setSpacing(1);

    setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QHBoxLayout* config_layout = new QHBoxLayout();
    config_layout->setContentsMargins(1, 1, 1, 1);
    config_layout->setSpacing(1);

    active_checkbox_ = new QCheckBox();
    connect(active_checkbox_, SIGNAL(clicked()), this, SLOT(toggleActive()));
    config_layout->addWidget(active_checkbox_);

    visible_checkbox_ = new QCheckBox(tr(filter_.getName().c_str()));
    connect(visible_checkbox_, SIGNAL(clicked()), this, SLOT(toggleVisible()));

    std::string style_str =
        " QCheckBox::indicator {  width: 12px; height: 12px; }  "
        "QCheckBox::indicator:checked   {     image: url(" +
        Files::getIconFilepath("collapse.png") +
        ");   }"
        "QCheckBox::indicator:unchecked   {     image: url(" +
        Files::getIconFilepath("expand.png") + "); }";
    visible_checkbox_->setStyleSheet(style_str.c_str());

    config_layout->addWidget(visible_checkbox_);
    config_layout->addStretch();

    manage_button_ = new QPushButton();
    manage_button_->setIcon(QIcon(Files::getIconFilepath("edit.png").c_str()));
    manage_button_->setFixedSize(UI_ICON_SIZE);
    manage_button_->setFlat(UI_ICON_BUTTON_FLAT);
    manage_button_->setToolTip(tr("Manage filter"));
    connect(manage_button_, SIGNAL(clicked()), this, SLOT(showMenuSlot()));
    config_layout->addWidget(manage_button_);
    main_layout->addLayout(config_layout);

    child_ = new QWidget();
    child_->setVisible(filter_.widgetVisible());

    child_layout_ = new QVBoxLayout();
    child_layout_->setContentsMargins(5, 1, 1, 1);
    child_layout_->setSpacing(1);

    updateChildWidget();

    child_->setLayout(child_layout_);

    main_layout->addWidget(child_);

    main_layout->addStretch();
    setLayout(main_layout);

//    connect(this, SIGNAL(deleteFilterSignal(DBFilter*)), &COMPASS::instance().filterManager(),
//            SLOT(deleteFilterSlot(DBFilter*)), Qt::QueuedConnection);
    createMenu();

    update();
}

DBFilterWidget::~DBFilterWidget() {}

void DBFilterWidget::createMenu()
{
    if (!filter_.isGeneric())
        return;

    QAction* reset_action = menu_.addAction(tr("Reset"));
    connect(reset_action, SIGNAL(triggered()), this, SLOT(reset()));

    QAction* edit_action = menu_.addAction(tr("Edit"));
    connect(edit_action, SIGNAL(triggered()), this, SLOT(filterEditSlot()));

    QAction* delete_action = menu_.addAction(tr("Delete"));
    connect(delete_action, SIGNAL(triggered()), this, SLOT(deleteFilter()));
}

void DBFilterWidget::addChildWidget(QWidget* widget)
{
    assert(widget);
    child_layout_->addWidget(widget);
}

void DBFilterWidget::updateChildWidget()
{
    QLayoutItem* child;
    while ((child = child_layout_->takeAt(0)) != 0)
    {
        if (child->widget())
            child_layout_->removeWidget(child->widget());
        ;
        child_layout_->removeItem(child);
        delete child;
    }

    std::vector<DBFilterCondition*>& conditions = filter_.getConditions();
    for (unsigned int cnt = 0; cnt < conditions.size(); cnt++)
    {
        QWidget* child_widget = conditions.at(cnt)->getWidget();
        assert(child_widget);
        child_layout_->addWidget(child_widget);
        connect(conditions.at(cnt), SIGNAL(possibleFilterChange()), this,
                SLOT(possibleSubFilterChange()), Qt::UniqueConnection);
    }
}

void DBFilterWidget::toggleVisible()
{
    logdbg << "DBFilterWidget: toggleVisible";
    filter_.widgetVisible(!filter_.widgetVisible());
    child_->setVisible(filter_.widgetVisible());
}

void DBFilterWidget::toggleAnd()
{
    logdbg << "DBFilterWidget: toggleAnd";

    // checked is or is false
    // unchecked is and is true
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

void DBFilterWidget::update(void)
{
    logdbg << "DBFilterWidget: update";

    visible_checkbox_->setText(filter_.getName().c_str());

    active_checkbox_->setChecked(filter_.getActive());
    visible_checkbox_->setChecked(filter_.widgetVisible());

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

void DBFilterWidget::setInvisible()
{
    filter_.widgetVisible(false);
    child_->setVisible(false);
}

void DBFilterWidget::possibleSubFilterChange()
{
    logdbg << "DBFilterWidget: possibleSubFilterChange";
    emit possibleFilterChange();
}

void DBFilterWidget::reset()
{
    filter_.reset();
    updateChildWidget();
    emit possibleFilterChange();
}

void DBFilterWidget::showMenuSlot() { menu_.exec(QCursor::pos()); }

void DBFilterWidget::deleteFilter() { emit deleteFilterSignal(&filter_); }

void DBFilterWidget::filterEditSlot()
{
    logdbg << "DBFilterWidget: filterEditSlot";
    emit filterEdit(&filter_);
}
