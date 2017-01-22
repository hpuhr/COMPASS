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

using namespace Utils;


DBInterfaceWidget::DBInterfaceWidget(DBInterface &interface, QWidget* parent, Qt::WindowFlags f)
    : interface_(interface), connection_layout_ (nullptr)
{
    unsigned int frame_width = 2;
    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

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
        radio->setFont (font_bold);
        grplayout->addWidget (radio);
    }
    groupBox->setLayout(grplayout);
    layout->addWidget(groupBox);

    connection_layout_ = new QVBoxLayout ();
    layout->addLayout(connection_layout_);

    layout->addStretch();

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

    connection_layout_->addWidget(interface_.connectionWidget());
}

void DBInterfaceWidget::databaseOpenedSlot ()
{

}
