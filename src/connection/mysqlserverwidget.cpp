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

}

void MySQLServerWidget::updateUserSlot (const QString &value)
{

}

void MySQLServerWidget::updatePasswortSlot (const QString &value)
{

}

void MySQLServerWidget::updatePortSlot (const QString &value)
{

}

void MySQLServerWidget::connectSlot ()
{

}

void MySQLServerWidget::updateDatabaseSlot (const QString &value)
{

}

void MySQLServerWidget::openDatabaseSlot ()
{

}
