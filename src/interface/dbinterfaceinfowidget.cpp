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
 * DBSelectionWidget.cpp
 *
 *  Created on: Aug 19, 2012
 *      Author: sk
 */

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>

#include "atsdb.h"
#include "dbconnection.h"
#include "dbinterfaceinfowidget.h"
#include "dbinterface.h"
#include "logger.h"
#include "stringconv.h"

using namespace Utils;


DBInterfaceInfoWidget::DBInterfaceInfoWidget(DBInterface &interface, QWidget* parent, Qt::WindowFlags f)
    : interface_(interface), layout_(nullptr)
{
    unsigned int frame_width = 2;
    QFont font_bold;
    font_bold.setBold(true);

    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(frame_width);

    layout_ = new QVBoxLayout ();
    setLayout (layout_);

    connect (&interface_, SIGNAL(databaseOpenedSignal()), this, SLOT(databaseOpenedSlot()));
}

DBInterfaceInfoWidget::~DBInterfaceInfoWidget()
{
}


void DBInterfaceInfoWidget::databaseOpenedSlot ()
{
    assert (layout_);
    layout_->addWidget(interface_.connection().infoWidget());
}
