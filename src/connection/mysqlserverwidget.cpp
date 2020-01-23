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
#include <QInputDialog>
#include <QMessageBox>
#include <QApplication>

MySQLServerWidget::MySQLServerWidget(MySQLppConnection& connection, MySQLServer& server, QWidget* parent)
    : QWidget (parent), connection_(connection), server_(server)
{
    QVBoxLayout *layout = new QVBoxLayout ();

    layout->addWidget (new QLabel("Server Host"));

    host_edit_ = new QLineEdit (server_.host().c_str());
    connect(host_edit_, &QLineEdit::textChanged, this, &MySQLServerWidget::updateHostSlot);
    layout->addWidget (host_edit_);

    layout->addWidget (new QLabel("Username"));

    user_edit_ = new QLineEdit (server_.user().c_str());
    connect(user_edit_, &QLineEdit::textChanged, this, &MySQLServerWidget::updateUserSlot);
    layout->addWidget (user_edit_);

    layout->addWidget (new QLabel("Password"));

    password_edit_ = new QLineEdit (server.password().c_str());
    password_edit_->setEchoMode(QLineEdit::Password);
    connect(password_edit_, &QLineEdit::textChanged, this, &MySQLServerWidget::updatePasswordSlot);
    layout->addWidget (password_edit_);

    layout->addWidget (new QLabel("Port number"));

    port_edit_ = new QLineEdit (QString::number(server.port()));
    connect(port_edit_, &QLineEdit::textChanged, this, &MySQLServerWidget::updatePortSlot);
    layout->addWidget (port_edit_);

    connect_button_ = new QPushButton ("Connect");
    connect (connect_button_, &QPushButton::clicked, this, &MySQLServerWidget::connectSlot);
    layout->addWidget(connect_button_);

    layout->addStretch();

    layout->addWidget (new QLabel("Database name"));

    db_name_box_ = new QComboBox ();
    connect (db_name_box_, SIGNAL(currentIndexChanged(const QString &)),
             this, SLOT(updateDatabaseSlot(const QString &)));
    db_name_box_->setDisabled(true);
    layout->addWidget (db_name_box_);

    QHBoxLayout* op_layout = new QHBoxLayout;

    new_button_ = new QPushButton ("New");
    connect (new_button_, &QPushButton::clicked, this, &MySQLServerWidget::newDatabaseSlot);
    new_button_->setDisabled(true);
    op_layout->addWidget(new_button_);

    clear_button_ = new QPushButton ("Clear");
    connect (clear_button_, &QPushButton::clicked, this, &MySQLServerWidget::clearDatabaseSlot);
    clear_button_->setDisabled (true);
    op_layout->addWidget(clear_button_);

    delete_button_ = new QPushButton ("Delete");
    connect (delete_button_, &QPushButton::clicked, this, &MySQLServerWidget::deleteDatabaseSlot);
    delete_button_->setDisabled(true);
    op_layout->addWidget(delete_button_);

    layout->addLayout(op_layout);

    open_button_ = new QPushButton ("Open");
    connect (open_button_, &QPushButton::clicked, this, &MySQLServerWidget::openDatabaseSlot);
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
    server_.port(value.toUInt());
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

    try
    {
        connection_.connectServer();
    }
    catch (std::runtime_error& e)
    {
        logerr << "MySQLServerWidget: connectSlot: connecting failed";

        QMessageBox msgBox;
        msgBox.setText(e.what());
        msgBox.exec();
        return;
    }

    host_edit_->setDisabled(true);
    user_edit_->setDisabled(true);
    password_edit_->setDisabled(true);
    port_edit_->setDisabled(true);
    connect_button_->setDisabled(true);

    updateDatabases ();
    db_name_box_->setDisabled(false);

    new_button_->setDisabled(false);
    clear_button_->setDisabled(false);
    delete_button_->setDisabled(false);
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
}

void MySQLServerWidget::updateDatabaseSlot (const QString &value)
{
    logdbg << "MySQLServerWidget: updateDatabaseSlot: value '" << value.toStdString() << "'";
    server_.database(value.toStdString());
}

void MySQLServerWidget::newDatabaseSlot ()
{
    logdbg << "MySQLServerWidget: newDatabaseSlot";

    bool ok;
    QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"), tr("Database name:"), QLineEdit::Normal,
                                         "example", &ok);
    if (ok && !text.isEmpty())
    {
        connection_.createDatabase(text.toStdString());
        server_.database(text.toStdString());
        updateDatabases();
    }
}

void MySQLServerWidget::clearDatabaseSlot ()
{
    logdbg << "MySQLServerWidget: clearDatabaseSlot";

    QMessageBox::StandardButton reply;
    std::string question = "Please confirm clearing all data from database '";
    question += server_.database().c_str();
    question += "'.";
    reply = QMessageBox::question(this, "Clear Database", question.c_str(), QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        connection_.deleteDatabase(server_.database());
        connection_.createDatabase(server_.database());
        updateDatabases();

        QApplication::restoreOverrideCursor();
    }
}


void MySQLServerWidget::deleteDatabaseSlot ()
{
    logdbg << "MySQLServerWidget: deleteDatabaseSlot";

    QMessageBox::StandardButton reply;
    std::string question = "Please confirm deletion of database '";
    question += server_.database().c_str();
    question += "'.";
    reply = QMessageBox::question(this, "Delete Database", question.c_str(), QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        connection_.deleteDatabase(server_.database());
        updateDatabases();

        QApplication::restoreOverrideCursor();
    }
}

void MySQLServerWidget::openDatabaseSlot ()
{
    loginf << "MySQLServerWidget: openDatabaseSlot: database '" << server_.database() << "'";

    if (server_.database() == "")
    {
        QMessageBox m_warning (QMessageBox::Warning, "No Database Selected",
                                 "Please select a valid database. New ones can be created using the 'New' button.",
                                 QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    connection_.openDatabase(server_.database());

    assert (open_button_);
    assert (db_name_box_);

    new_button_->setDisabled(true);
    clear_button_->setDisabled(true);
    delete_button_->setDisabled(true);

    open_button_->setDisabled(true);
    db_name_box_->setDisabled(true);

    emit databaseOpenedSignal ();
}
