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

#include "propertylist.h"
#include "mysqlserverwidget.h"
#include "mysqlppconnection.h"
#include "mysqlserver.h"

#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

MySQLServerWidget::MySQLServerWidget(MySQLppConnection &connection, MySQLServer &server, QWidget *parent)
    : QWidget (parent), connection_(connection), server_(server)
{
    QVBoxLayout *layout = new QVBoxLayout ();

//    QLabel *main = new QLabel (server.getInstanceId().c_str());
//    layout->addWidget(main);

    layout->addWidget (new QLabel("Server Host"));

    host_edit_ = new QLineEdit (server_.host().c_str());
    connect(host_edit_, SIGNAL(textChanged(const QString &)), this, SLOT(updateHostSlot(const QString &)));
    layout->addWidget (host_edit_);

    layout->addWidget (new QLabel("Username"));

    user_edit_ = new QLineEdit (server_.user().c_str());
    connect(user_edit_, SIGNAL(textChanged(const QString &)), this, SLOT(updateUserSlot(const QString &)));
    layout->addWidget (user_edit_);

    layout->addWidget (new QLabel("Password"));

    password_edit_ = new QLineEdit (server.password().c_str());
    connect(password_edit_, SIGNAL(textChanged(const QString &)), this, SLOT(updatePasswordSlot(const QString &)));
    layout->addWidget (password_edit_);

    layout->addWidget (new QLabel("Port number"));

    port_edit_ = new QLineEdit (QString::number(server.port()));
    connect(port_edit_, SIGNAL(textChanged(const QString &)), this, SLOT(updatePortSlot(const QString &)));
    layout->addWidget (port_edit_);

    connect_button_ = new QPushButton ("Connect");
    connect (connect_button_, SIGNAL(clicked()), this, SLOT(connectSlot()));
    layout->addWidget(connect_button_);

    layout->addStretch();

    layout->addWidget (new QLabel("Database name"));

    db_name_box_ = new QComboBox ();
    connect (db_name_box_, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(updateDatabaseSlot(const QString &)));
    db_name_box_->setDisabled(true);
    layout->addWidget (db_name_box_);

    open_button_ = new QPushButton ("Open");
    //open_button_->setFont(font_bold);
    connect (open_button_, SIGNAL(clicked()), this, SLOT(openDatabaseSlot()));
    open_button_->setDisabled(true);
    layout->addWidget(open_button_);

    setLayout (layout);
}

MySQLServerWidget::~MySQLServerWidget ()
{

}

void MySQLServerWidget::updateHostSlot (const QString &value)
{
    logdbg << "MySQLServerWidget: updateHostSlot: value '" << value.toStdString() << "'";
    server_.host(value.toStdString());
}

void MySQLServerWidget::updateUserSlot (const QString &value)
{
    logdbg << "MySQLServerWidget: updateUserSlot: value '" << value.toStdString() << "'";
    server_.user(value.toStdString());
}

void MySQLServerWidget::updatePasswordSlot (const QString &value)
{
    logdbg << "MySQLServerWidget: updatePasswortSlot: value '" << value.toStdString() << "'";
    server_.password(value.toStdString());
}

void MySQLServerWidget::updatePortSlot (const QString &value)
{
    logdbg << "MySQLServerWidget: updatePortSlot: value '" << value.toStdString() << "'";
    server_.port(value.toInt());
}

void MySQLServerWidget::connectSlot ()
{
    logdbg << "MySQLServerWidget: connectSlot";

    assert (host_edit_);
    assert (user_edit_);
    assert (password_edit_);
    assert (port_edit_);

    assert (connect_button_);
    assert (open_button_);

    host_edit_->setDisabled(true);
    user_edit_->setDisabled(true);
    password_edit_->setDisabled(true);
    port_edit_->setDisabled(true);
    connect_button_->setDisabled(true);

    connection_.connectServer();

    updateDatabases ();
    db_name_box_->setDisabled(false);
    open_button_->setDisabled(false);

    emit serverConnectedSignal ();
}

void MySQLServerWidget::updateDatabases ()
{
    std::vector <std::string> databases = connection_.getDatabases();
    std::string used_database = server_.database();

    logdbg << "MySQLServerWidget: updateDatabases: got used database '" << used_database << "'";

    db_name_box_->clear();

    for (auto it : databases)
    {
        db_name_box_->addItem(it.c_str());
    }

    int index = db_name_box_->findText(used_database.c_str());
    if (index != -1) // -1 for not found
    {
       db_name_box_->setCurrentIndex(index);
    }

//    index = db_name_box_->currentIndex();
//    if (index != -1)
//    {
//        QString selected = db_name_box_->itemText(index); //get selected
//        logdbg << "MySQLppConnectionWidget: updateDatabases: got database '" << selected.toStdString() << "'";
//        updateDatabaseSlot (selected);
//    }

}

void MySQLServerWidget::updateDatabaseSlot (const QString &value)
{
    logdbg << "MySQLServerWidget: updateDatabaseSlot: value '" << value.toStdString() << "'";
    server_.database(value.toStdString());
}

void MySQLServerWidget::openDatabaseSlot ()
{
    logdbg << "MySQLServerWidget: openDatabaseSlot";
    connection_.openDatabase(server_.database());

    assert (open_button_);
    assert (db_name_box_);
    open_button_->setDisabled(true);
    db_name_box_->setDisabled(true);

    emit databaseOpenedSignal ();
}
