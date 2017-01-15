#include "mysqlppconnectionwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>

MySQLppConnectionWidget::MySQLppConnectionWidget(MySQLppConnection &connection, QWidget *parent)
    : QWidget(parent), connection_(connection), mysql_db_ip_edit_(0), mysql_db_port_edit_ (0), mysql_db_username_edit_ (0), mysql_db_password_edit_(0),
      connect_button_(0),  mysql_db_name_box_ (0), open_button_(0)
{
    createElements();
}

void MySQLppConnectionWidget::updateMySQLConnectInfo ()
{
    mysql_db_ip_ = mysql_db_ip_edit_->text().toStdString();
    mysql_db_port_ = mysql_db_port_edit_->text().toStdString();
    mysql_db_username_ = mysql_db_username_edit_->text().toStdString();
    mysql_db_password_ = mysql_db_password_edit_->text().toStdString();
}

void MySQLppConnectionWidget::updateMySQLDatabaseInfo ()
{
    mysql_db_name_ = mysql_db_name_box_->getDatabaseName();
}

void MySQLppConnectionWidget::createElements()
{
    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    QVBoxLayout *layout = new QVBoxLayout ();

    QLabel *db_system_label = new QLabel (tr("Database Server"));
    db_system_label->setFont (font_big);
    layout->addWidget (db_system_label);

    QLabel *db_ip_label = new QLabel("Server Name or IP address");
    layout->addWidget (db_ip_label);

    mysql_db_ip_edit_ = new QLineEdit ();
    mysql_db_ip_edit_->setText (mysql_db_ip_.c_str());
    connect(mysql_db_ip_edit_, SIGNAL( returnPressed() ), this, SLOT(updateMySQLConnectInfo()));
    layout->addWidget (mysql_db_ip_edit_);

    QLabel *db_port_label = new QLabel("Port number");
    layout->addWidget (db_port_label);

    mysql_db_port_edit_ = new QLineEdit ();
    mysql_db_port_edit_->setText (mysql_db_port_.c_str());
    connect(mysql_db_port_edit_, SIGNAL( returnPressed() ), this, SLOT(updateMySQLConnectInfo()));
    layout->addWidget (mysql_db_port_edit_);

    QLabel *db_username_label = new QLabel("Username");
    layout->addWidget (db_username_label);

    mysql_db_username_edit_ = new QLineEdit ();
    mysql_db_username_edit_->setText (mysql_db_username_.c_str());
    connect(mysql_db_username_edit_, SIGNAL( returnPressed() ), this, SLOT(updateMySQLConnectInfo()));
    layout->addWidget (mysql_db_username_edit_);

    QLabel *db_password_label = new QLabel("Password");
    layout->addWidget (db_password_label);

    mysql_db_password_edit_ = new QLineEdit ();
    mysql_db_password_edit_->setText (mysql_db_password_.c_str());
    connect(mysql_db_password_edit_, SIGNAL( returnPressed() ), this, SLOT(updateMySQLConnectInfo()));
    layout->addWidget (mysql_db_password_edit_);

    connect_button_ = new QPushButton ("Connect");
    connect_button_->setFont(font_bold);
    connect (connect_button_, SIGNAL(clicked()), this, SLOT(connectDB()));
    layout->addWidget(connect_button_);

    layout->addStretch();

    QLabel *db_name_label = new QLabel("Database name");
    db_name_label->setFont(font_big);
    layout->addWidget (db_name_label);

    mysql_db_name_box_ = new DatabaseNameComboBox ();
    layout->addWidget (mysql_db_name_box_);

    open_button_ = new QPushButton ("Open");
    open_button_->setFont(font_bold);
    connect (open_button_, SIGNAL(clicked()), this, SLOT(openDB()));
    open_button_->setDisabled(true);
    layout->addWidget(open_button_);

    setLayout (layout);
}

void MySQLppConnectionWidget::setDBServer (std::string value)
{
    assert (mysql_db_ip_edit_);
    mysql_db_ip_edit_->setText(value.c_str());
    mysql_db_ip_=value;
}
void MySQLppConnectionWidget::setDBName (std::string value)
{
    assert (mysql_db_name_box_);

    if (mysql_db_name_box_->hasDatabaseName(value))
        mysql_db_name_box_->setDatabaseName(value);
    else
        value = mysql_db_name_box_->getDatabaseName();
    mysql_db_name_=value;
}
void MySQLppConnectionWidget::setDBPort (std::string value)
{
    assert (mysql_db_port_edit_);
    mysql_db_port_edit_->setText(value.c_str());
    mysql_db_port_=value;

}
void MySQLppConnectionWidget::setDBUser (std::string value)
{
    assert (mysql_db_username_edit_);
    mysql_db_username_edit_->setText(value.c_str());
    mysql_db_username_=value;

}
void MySQLppConnectionWidget::setDBPassword (std::string value)
{
    assert (mysql_db_password_edit_);
    mysql_db_password_edit_->setText(value.c_str());
    mysql_db_password_=value;
}
void MySQLppConnectionWidget::setDBNoPassword ()
{
    assert (mysql_db_password_edit_);
    mysql_db_password_edit_->setText("");
    mysql_db_password_="";
}

void MySQLppConnectionWidget::connectDB ()
{
//    ATSDB::getInstance().connect(getConnectionInfo());

//    assert (mysql_db_name_box_);
//    mysql_db_name_box_->loadDatabaseNames();

//    if (mysql_db_name_box_->hasDatabaseName(mysql_db_name_))
//        mysql_db_name_box_->setDatabaseName (mysql_db_name_);
//    else
//        mysql_db_name_=mysql_db_name_box_->getDatabaseName();
//    connect(mysql_db_name_box_, SIGNAL( currentIndexChanged (const QString &) ), this, SLOT( updateMySQLDatabaseInfo() ));

//    connect_button_->setDisabled(true);
//    open_button_->setDisabled(false);
}
void MySQLppConnectionWidget::openDB ()
{
//    ATSDB::getInstance().open(mysql_db_name_);
//    open_button_->setDisabled(true);
//    emit databaseOpened();
}
