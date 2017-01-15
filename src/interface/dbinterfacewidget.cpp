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
#include "logger.h"
#include "stringconv.h"

using namespace Utils;


DBInterfaceWidget::DBInterfaceWidget(DBInterface &interface, QWidget* parent, Qt::WindowFlags f)
 : interface_(interface), mysqlpp_radio_(nullptr)
{
  createElements();
}

DBInterfaceWidget::~DBInterfaceWidget()
{

}

//void DBSelectionWidget::checkSubConfigurables ()
//{

//}

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

void DBInterfaceWidget::selectDBType()
{
  // 0 undefined, 1 file, 2 mysqlpp, 3 mysqlcon
//  if (file_radio_->isDown ())
//    db_type_selection_ = 1;
  if (mysqlpp_radio_->isDown())
    db_type_selection_ = 2;
//  else if (mysqlcon_radio_->isDown())
//    db_type_selection_ = 3;
  else
    db_type_selection_ = 0;
}


//bool DBInterfaceWidget::hasDefinedDatabase ()
//{
////  if (db_type_selection_ == 1)
////  {
////    return filename_.size() != 0;
////  }
////  else
//  if (db_type_selection_ == 2) //  || db_type_selection_ == 3
//    return true;
//  else
//    return false;
//}


void DBInterfaceWidget::createElements ()
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

//  mysqlcon_radio_ = new QRadioButton("MySQL connector database", this);
//  connect(mysqlcon_radio_, SIGNAL(pressed()), this, SLOT(selectDBType()));
//  if (db_type_selection_ == 3)
//      mysqlcon_radio_->setChecked (true);
//  mysqlcon_radio_->setFont (font_bold);
//  layout->addWidget (mysqlcon_radio_);

  layout->addStretch();


  setLayout (layout);
}

//void DBInterfaceWidget::setDBType (std::string value)
//{
////  assert (file_radio_);
////  assert (mysqlpp_radio_);
////  assert (mysqlcon_radio_);

////  if (value.compare ("sqlite") == 0)
////  {
////    file_radio_->click();
////    db_type_selection_ = 1;
////  }
//  if (value.compare ("mysqlpp") == 0)
//  {
//    mysqlpp_radio_->click();
//    db_type_selection_ = 2;
//  }
////  if (value.compare ("mysqlcon") == 0)
////  {
////    mysqlcon_radio_->click();
////    db_type_selection_ = 3;
////  }
//  else
//  {
//    logerr  << "DBSelectionWidget: setDBType: unknown value '" << value << "'";
////    mysqlcon_radio_->click();
//    db_type_selection_ = 0;

//  }
//}
//void DBSelectionWidget::setDBFilename (std::string value)
//{
//  assert (filename_edit_);
//  filename_edit_->setText(value.c_str());
//  filename_=value;
//}

