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

#include "ATSDB.h"
#include "DBSelectionWidget.h"
#include "DBConnectionInfo.h"
#include "Logger.h"
#include "String.h"

using namespace Utils;


DBSelectionWidget::DBSelectionWidget(std::string class_id, std::string instance_id, Configurable *parent)
 : Configurable (class_id, instance_id, parent), /*filename_edit_(0), file_radio_(0),*/ mysqlpp_radio_(0), mysqlcon_radio_(0),
  mysql_db_ip_edit_(0), mysql_db_port_edit_ (0), mysql_db_username_edit_ (0), mysql_db_password_edit_(0),
  connect_button_(0),  mysql_db_name_box_ (0), open_button_(0)
{
  registerParameter ("filename", &filename_, (std::string)"");

  registerParameter ("mysql_db_name", &mysql_db_name_, (std::string) "test");
  registerParameter ("mysql_db_ip", &mysql_db_ip_, (std::string) "localhost");
  registerParameter ("mysql_db_port", &mysql_db_port_, (std::string) "3306");
  registerParameter ("mysql_db_username", &mysql_db_username_, (std::string) "root");
  registerParameter ("mysql_db_password", &mysql_db_password_, (std::string) "");

  registerParameter ("db_type_selection", &db_type_selection_, 0);

  createElements();

  createSubConfigurables();
}

DBSelectionWidget::~DBSelectionWidget()
{

}

void DBSelectionWidget::checkSubConfigurables ()
{

}

//void DBSelectionWidget::selectFile()
//{
//  logdbg  << "DBSelectionWidget: selectFile";
//
//  QString filename = QFileDialog::getOpenFileName(this, QObject::tr("Open RDB File"), tr("../data/"),
//      QObject::tr("RDB files (*.rdb)"), 0, QFileDialog::DontUseNativeDialog);
//
//  filename_ = filename.toStdString ();
//  filename_edit_->setText (filename);
//}

void DBSelectionWidget::selectDBType()
{
  // 0 undefined, 1 file, 2 mysqlpp, 3 mysqlcon
//  if (file_radio_->isDown ())
//    db_type_selection_ = 1;
  if (mysqlpp_radio_->isDown())
    db_type_selection_ = 2;
  else if (mysqlcon_radio_->isDown())
    db_type_selection_ = 3;
  else
    db_type_selection_ = 0;
}

void DBSelectionWidget::updateMySQLConnectInfo ()
{
  mysql_db_ip_ = mysql_db_ip_edit_->text().toStdString();
  mysql_db_port_ = mysql_db_port_edit_->text().toStdString();
  mysql_db_username_ = mysql_db_username_edit_->text().toStdString();
  mysql_db_password_ = mysql_db_password_edit_->text().toStdString();
}

void DBSelectionWidget::updateMySQLDatabaseInfo ()
{
    mysql_db_name_ = mysql_db_name_box_->getDatabaseName();
}


bool DBSelectionWidget::hasDefinedDatabase ()
{
  if (db_type_selection_ == 1)
  {
    return filename_.size() != 0;
  }
  else if (db_type_selection_ == 2 || db_type_selection_ == 3)
    return true;
  else
    return false;
}

DBConnectionInfo *DBSelectionWidget::getConnectionInfo ()
{
  assert (hasDefinedDatabase());

  if (db_type_selection_ == 1)
  {
      throw std::runtime_error ("DBSelectionWidget: getConnectionInfo: SQLite3 connection not supported at the moment");
    //return new SQLite3ConnectionInfo (filename_);
  }
  else if (db_type_selection_ == 2)
  {
//      throw std::runtime_error ("DBSelectionWidget: getConnectionInfo: MySql++ connection not supported at the moment");
    return new MySQLConnectionInfo (DB_TYPE_MYSQLpp, mysql_db_name_, mysql_db_ip_, mysql_db_username_, mysql_db_password_,
            String::intFromString(mysql_db_port_));
  }
  else if (db_type_selection_ == 3)
  {
    return new MySQLConnectionInfo (DB_TYPE_MYSQLCon, mysql_db_name_, mysql_db_ip_, mysql_db_username_, mysql_db_password_,
            String::intFromString(mysql_db_port_));
  }
  else
    throw std::runtime_error ("DBSelectionWidget: getConnectionInfo: undefined connection");
}

void DBSelectionWidget::createElements ()
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

//  file_radio_ = new QRadioButton("File container", this);
//  connect(file_radio_, SIGNAL(pressed()), this, SLOT(selectDBType()));
//  if (db_type_selection_ == 1)
//    file_radio_->setChecked (true);
//  file_radio_->setFont (font_bold);
//  db_type_layout->addWidget (file_radio_);
//
//  filename_edit_ = new QTextEdit (tr(filename_.c_str()));
//  filename_edit_->setReadOnly (true);
//  filename_edit_->setMaximumHeight (50);
//  db_type_layout->addWidget (filename_edit_);
//
//  QPushButton *select_file = new QPushButton(tr("Select"));
//  connect(select_file, SIGNAL( clicked() ), this, SLOT( selectFile() ));
//  db_type_layout->addWidget(select_file);
//
  mysqlpp_radio_ = new QRadioButton("MySQLpp database", this);
  connect(mysqlpp_radio_, SIGNAL(pressed()), this, SLOT(selectDBType()));
  if (db_type_selection_ == 2)
      mysqlpp_radio_->setChecked (true);
  mysqlpp_radio_->setFont (font_bold);
  layout->addWidget (mysqlpp_radio_);

  mysqlcon_radio_ = new QRadioButton("MySQL connector database", this);
  connect(mysqlcon_radio_, SIGNAL(pressed()), this, SLOT(selectDBType()));
  if (db_type_selection_ == 3)
      mysqlcon_radio_->setChecked (true);
  mysqlcon_radio_->setFont (font_bold);
  layout->addWidget (mysqlcon_radio_);

  layout->addStretch();

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

void DBSelectionWidget::setDBType (std::string value)
{
//  assert (file_radio_);
//  assert (mysqlpp_radio_);
  assert (mysqlcon_radio_);

//  if (value.compare ("sqlite") == 0)
//  {
//    file_radio_->click();
//    db_type_selection_ = 1;
//  }
  if (value.compare ("mysqlpp") == 0)
  {
    mysqlpp_radio_->click();
    db_type_selection_ = 2;
  }
  if (value.compare ("mysqlcon") == 0)
  {
    mysqlcon_radio_->click();
    db_type_selection_ = 3;
  }
  else
  {
    logerr  << "DBSelectionWidget: setDBType: unknown value '" << value << "'";
    mysqlcon_radio_->click();
    db_type_selection_ = 3;

  }
}
//void DBSelectionWidget::setDBFilename (std::string value)
//{
//  assert (filename_edit_);
//  filename_edit_->setText(value.c_str());
//  filename_=value;
//}
void DBSelectionWidget::setDBServer (std::string value)
{
  assert (mysql_db_ip_edit_);
  mysql_db_ip_edit_->setText(value.c_str());
  mysql_db_ip_=value;
}
void DBSelectionWidget::setDBName (std::string value)
{
  assert (mysql_db_name_box_);

  if (mysql_db_name_box_->hasDatabaseName(value))
      mysql_db_name_box_->setDatabaseName(value);
  else
      value = mysql_db_name_box_->getDatabaseName();
  mysql_db_name_=value;
}
void DBSelectionWidget::setDBPort (std::string value)
{
  assert (mysql_db_port_edit_);
  mysql_db_port_edit_->setText(value.c_str());
  mysql_db_port_=value;

}
void DBSelectionWidget::setDBUser (std::string value)
{
  assert (mysql_db_username_edit_);
  mysql_db_username_edit_->setText(value.c_str());
  mysql_db_username_=value;

}
void DBSelectionWidget::setDBPassword (std::string value)
{
  assert (mysql_db_password_edit_);
  mysql_db_password_edit_->setText(value.c_str());
  mysql_db_password_=value;
}
void DBSelectionWidget::setDBNoPassword ()
{
  assert (mysql_db_password_edit_);
  mysql_db_password_edit_->setText("");
  mysql_db_password_="";
}

void DBSelectionWidget::connectDB ()
{
    ATSDB::getInstance().connect(getConnectionInfo());

    assert (mysql_db_name_box_);
    mysql_db_name_box_->loadDatabaseNames();

    if (mysql_db_name_box_->hasDatabaseName(mysql_db_name_))
        mysql_db_name_box_->setDatabaseName (mysql_db_name_);
    else
        mysql_db_name_=mysql_db_name_box_->getDatabaseName();
    connect(mysql_db_name_box_, SIGNAL( currentIndexChanged (const QString &) ), this, SLOT( updateMySQLDatabaseInfo() ));

    connect_button_->setDisabled(true);
    open_button_->setDisabled(false);
}
void DBSelectionWidget::openDB ()
{
    ATSDB::getInstance().open(mysql_db_name_);
    open_button_->setDisabled(true);
    emit databaseOpened();
}
