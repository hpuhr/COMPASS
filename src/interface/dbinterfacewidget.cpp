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

    QLabel *db_type_label = new QLabel ("Database System");
    db_type_label->setFont (font_big);
    layout->addWidget (db_type_label);

    std::vector<std::string> types = interface_.getDatabaseConnectionTypes();

    for (auto type : types)
    {
        QRadioButton *radio = new QRadioButton(type.c_str(), this);
        connect(radio, SIGNAL(pressed()), this, SLOT(databaseTypeSelectSlot()));
        if (types.size() == 1)
            radio->setChecked (true);
        radio->setFont (font_bold);
        layout->addWidget (radio);
    }

    connection_layout_ = new QVBoxLayout ();
    layout->addLayout(connection_layout_);

    layout->addStretch();

    setLayout (layout);

    if (types.size() == 1)
        initConnection (types.at(0));
}

DBInterfaceWidget::~DBInterfaceWidget()
{
    connection_layout_ = nullptr;
}

void DBInterfaceWidget::databaseTypeSelectSlot ()
{
    QRadioButton *radio = dynamic_cast <QRadioButton *> (QObject::sender());
    initConnection(radio->text().toStdString());
}

void DBInterfaceWidget::initConnection (std::string connection_type)
{
    interface_.initConnection(connection_type);

    assert (connection_layout_);

    connection_layout_->addWidget(interface_.connectionWidget());
}

void DBInterfaceWidget::databaseOpenedSlot ()
{

}
