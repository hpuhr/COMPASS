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
 * QCollapsableWidget.h
 *
 *  Created on: Sep 28, 2011
 *      Author: sk
 */

#ifndef QCOLLAPSABLEWIDGET_H_
#define QCOLLAPSABLEWIDGET_H_

#include <QFrame>

class QWidget;
class QCheckBox;
class QVBoxLayout;

/**
 * @brief Earlier version of DBFilterWidget. Not in use at the moment.
 *
 * Embeds a child widget which can be shown or hidden based on a checkbox.
 */
class QCollapsableWidget: public QFrame
{
	Q_OBJECT
private slots:
  void toggleVisible();

private:
	std::string name_;
	bool visible_;
	QWidget *child_;
	QCheckBox *visible_checkbox_;
//  QCheckBox *active_checkbox_;
//	QCheckBox *and_checkbox_;
//	QCheckBox *invert_checkbox_;

	QVBoxLayout *layout_;
public:
	QCollapsableWidget (std::string name, QWidget * parent = 0, Qt::WindowFlags f = 0);
	virtual ~QCollapsableWidget();

	QWidget *getChildWidget () { return child_; };
};

#endif /* QCOLLAPSABLEWIDGET_H_ */
