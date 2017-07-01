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

#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>

#include "atsdb.h"
#include "dbinterfacewidget.h"
#include "dbinterface.h"
#include "logger.h"
#include "stringconv.h"
#include "global.h"

using namespace Utils;


DBInterfaceWidget::DBInterfaceWidget(DBInterface &interface, QWidget* parent, Qt::WindowFlags f)
    : interface_(interface), connection_layout_ (nullptr)
{
    unsigned int frame_width = FRAME_SIZE;
    QFont font_bold;
    font_bold.setBold(true);

    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(frame_width);

    QVBoxLayout *layout = new QVBoxLayout ();

    QGroupBox *groupBox = new QGroupBox(tr("Database System"));
    QVBoxLayout *grplayout = new QVBoxLayout ();

    const std::map<std::string, DBConnection*> &types = interface_.connections();
    for (auto it : types)
    {
        QRadioButton *radio = new QRadioButton(it.first.c_str(), this);
        connect(radio, SIGNAL(pressed()), this, SLOT(databaseTypeSelectSlot()));
        if (types.size() == 1)
            radio->setChecked (true);
        grplayout->addWidget (radio);
    }
    groupBox->setLayout(grplayout);
    layout->addWidget(groupBox);

   layout->addStretch();

    connection_layout_ = new QVBoxLayout ();
    layout->addLayout(connection_layout_);

    setLayout (layout);

    if (types.size() == 1)
        useConnection (types.begin()->first);
}

DBInterfaceWidget::~DBInterfaceWidget()
{
    connection_layout_ = nullptr;
}

void DBInterfaceWidget::databaseTypeSelectSlot ()
{
    QRadioButton *radio = dynamic_cast <QRadioButton *> (QObject::sender());
    useConnection(radio->text().toStdString());
}

void DBInterfaceWidget::useConnection (std::string connection_type)
{
    interface_.useConnection(connection_type);

    assert (connection_layout_);

    QObject::connect(interface_.connectionWidget(), SIGNAL(databaseOpenedSignal()), this, SLOT(databaseOpenedSlot()), static_cast<Qt::ConnectionType>(Qt::UniqueConnection));

    connection_layout_->addWidget(interface_.connectionWidget());
}

void DBInterfaceWidget::databaseOpenedSlot ()
{
    logdbg << "DBInterfaceWidget: databaseOpenedSlot";
    emit databaseOpenedSignal();
}
