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
 * QCollapsableWidget.cpp
 *
 *  Created on: Sep 28, 2011
 *      Author: sk
 */

#include <QWidget>
#include <QCheckBox>
#include <QVBoxLayout>
#include "QCollapsableWidget.h"
#include "Logger.h"

using namespace std;

QCollapsableWidget::QCollapsableWidget (string name, QWidget * parent, Qt::WindowFlags f)
:QFrame (parent, f)
{
	logdbg  << "QCollapsableWidget: constructor";
	name_ = name;
	visible_ = false;

	setFrameStyle(QFrame::Panel | QFrame::Raised);

	layout_ = new QVBoxLayout();
	layout_->setContentsMargins (5, 0, 0, 0);
	layout_->setSpacing (0);
	layout_->setAlignment (Qt::AlignTop);

	QHBoxLayout *hlayout = new QHBoxLayout ();
  hlayout->setContentsMargins (0, 0, 0, 0);
  hlayout->setSpacing (0);

//  active_checkbox_ = new QCheckBox();
//  connect( active_checkbox_, SIGNAL( clicked() ), this, SLOT( toggleActive() ) );
//  hlayout->addWidget (active_checkbox_);

  //hlayout->addSpacing(10);

	visible_checkbox_ = new QCheckBox(tr(name_.c_str()));
	connect( visible_checkbox_, SIGNAL( clicked() ), this, SLOT( toggleVisible() ) );

	visible_checkbox_->setStyleSheet (" QCheckBox::indicator {  width: 12px; height: 12px; }  "
	    "QCheckBox::indicator:checked   {     image: url(./Data/icons/collapse.png);   }"
	    "QCheckBox::indicator:unchecked   {     image: url(./Data/icons/expand.png); }");

  hlayout->addWidget (visible_checkbox_);
  hlayout->addStretch();

//  and_checkbox_ = new QCheckBox();
//  connect( and_checkbox_, SIGNAL( clicked() ), this, SLOT( toggleAnd() ) );
//
//  and_checkbox_->setStyleSheet (" QCheckBox::indicator {  width: 12px; height: 12px; }  "
//      "QCheckBox::indicator:checked   {     image: url(./Data/icons/or.png);   }"
//      "QCheckBox::indicator:unchecked   {     image: url(./Data/icons/and.png); }");
//
//  hlayout->addWidget (and_checkbox_);
//
//  invert_checkbox_ = new QCheckBox();
//  connect( invert_checkbox_, SIGNAL( clicked() ), this, SLOT( invert() ) );
//
//  invert_checkbox_->setStyleSheet (" QCheckBox::indicator {  width: 12px; height: 12px; }  "
//      "QCheckBox::indicator:checked   {     image: url(./Data/icons/invert.png);   }"
//      "QCheckBox::indicator:unchecked   {     image: url(./Data/icons/invert.png); }");
//
//  hlayout->addWidget (invert_checkbox_);


  layout_->addLayout (hlayout);

	child_ = new QWidget ();
	child_->setVisible (visible_);

	layout_->addWidget (child_);

	setLayout (layout_);
}

QCollapsableWidget::~QCollapsableWidget()
{
	logdbg  << "QCollapsableWidget: destructor";
}

void QCollapsableWidget::toggleVisible()
{
	logdbg  << "QCollapsableWidget: toggleVisible";
	visible_ = !visible_;
	child_->setVisible(visible_);
}

//void QCollapsableWidget::toggleAnd()
//{
//  loginf << "QCollapsableWidget: toggleAnd";
////  visible_ = !visible_;
////  child_->setVisible(visible_);
//}
//
//void QCollapsableWidget::toggleActive()
//{
//  loginf << "QCollapsableWidget: toggleActive";
////  visible_ = !visible_;
////  child_->setVisible(visible_);
//}
//
//void QCollapsableWidget::invert()
//{
//  loginf << "QCollapsableWidget: invert";
////  visible_ = !visible_;
////  child_->setVisible(visible_);
//}

