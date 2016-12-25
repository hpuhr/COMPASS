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
 * AirspaceSectorWidget.cpp
 *
 *  Created on: Nov 25, 2013
 *      Author: sk
 */

#include "AirspaceSectorWidget.h"
#include "Logger.h"
#include "AirspaceSector.h"
#include "AirspaceSectorManager.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QApplication>
#include <QClipboard>

#include "String.h"

using namespace Utils;

AirspaceSectorWidget::AirspaceSectorWidget(QWidget *parent, Qt::WindowFlags f)
: QWidget (parent, f), current_(0), table_ (0)
{

    createElements();

    disableWidgets ();
}

AirspaceSectorWidget::~AirspaceSectorWidget()
{

}

void AirspaceSectorWidget::createElements ()
{
    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    QVBoxLayout *vlayout = new QVBoxLayout;

    QLabel *label = new QLabel ("Airspace Sector");
    label->setFont (font_big);
    vlayout->addWidget (label);

    QGridLayout *grid_layout = new QGridLayout ();

    QLabel *name_label = new QLabel ("Name");
    grid_layout->addWidget (name_label, 0, 0);

    name_edit_ = new QLineEdit ();
    connect(name_edit_, SIGNAL( returnPressed() ), this, SLOT( setName() ));
    grid_layout->addWidget (name_edit_, 0, 1);

    QLabel *volume_label = new QLabel ("Has Own Volume");
    grid_layout->addWidget (volume_label, 1, 0);

    has_own_volume_ = new QCheckBox ();
    has_own_volume_->setChecked (false);
    connect(has_own_volume_, SIGNAL( clicked() ), this, SLOT( setHasOwnVolume() ));
    grid_layout->addWidget (has_own_volume_, 1, 1);

    QLabel *height_min_label = new QLabel ("Height Minimum [ft]");
    grid_layout->addWidget (height_min_label, 2, 0);

    height_min_edit_ = new QLineEdit ();
    connect(height_min_edit_, SIGNAL( returnPressed() ), this, SLOT( setHeightMin() ));
    grid_layout->addWidget (height_min_edit_, 2, 1);

    QLabel *height_max_label = new QLabel ("Height Maximum [ft]");
    grid_layout->addWidget (height_max_label, 3, 0);

    height_max_edit_ = new QLineEdit ();
    connect(height_max_edit_, SIGNAL( returnPressed() ), this, SLOT( setHeightMax() ));
    grid_layout->addWidget (height_max_edit_, 3, 1);

    QLabel *check_label = new QLabel ("Used for Checking");
    grid_layout->addWidget (check_label, 4, 0);

    used_for_checking_ = new QCheckBox ();
    used_for_checking_->setChecked (false);
    connect(used_for_checking_, SIGNAL( clicked() ), this, SLOT( setUseForChecking() ));
    grid_layout->addWidget (used_for_checking_, 4, 1);

    vlayout->addLayout (grid_layout);

    //QVBoxLayout *blayout = new QHBoxLayout;

    delete_button_= new QPushButton ("Delete Sector");
    connect(delete_button_, SIGNAL( clicked() ), this, SLOT( deleteSector() ));
    vlayout->addWidget (delete_button_);

   // vlayout->addStretch();

    clear_button_ = new QPushButton ("Clear Points");
    connect(clear_button_, SIGNAL( clicked() ), this, SLOT( clearPoints() ));
    vlayout->addWidget (clear_button_);

    add_button_ = new QPushButton ("Add Points From Clipboard");
    connect(add_button_, SIGNAL( clicked() ), this, SLOT( addPoints() ));
    vlayout->addWidget (add_button_);

    copy_button_ = new QPushButton ("Copy Points To Clipboard");
    connect(copy_button_, SIGNAL( clicked() ), this, SLOT( copyPoints() ));
    vlayout->addWidget (copy_button_);

    table_ = new QTableWidget ();
    table_->setAlternatingRowColors(true);

    table_->setColumnCount(2);

    QStringList header_list;
    header_list.append (tr("Latitude"));
    header_list.append (tr("Longitude"));
    table_->setHorizontalHeaderLabels (header_list);

    //connect( table_, SIGNAL( itemClicked( QTableWidgetItem * )), this, SLOT( itemChanged ( QTableWidgetItem * )));

    vlayout->addWidget (table_);



    setLayout( vlayout );
}

void AirspaceSectorWidget::showSector (AirspaceSector *sector)
{
    assert (sector);
    loginf << "AirspaceSectorWidget: showSector: " << sector->getName();

    current_=sector;

    enableWidgets ();

    assert (name_edit_);
    name_edit_->setText(sector->getName().c_str());

    has_own_volume_->setChecked(current_->hasOwnVolume());

    if (!current_->hasOwnVolume())
    {
        height_min_edit_->setDisabled(true);
        height_max_edit_->setDisabled(true);

        clear_button_->setDisabled(true);
        add_button_->setDisabled(true);
        copy_button_->setDisabled(true);

        table_->setDisabled(true);
    }
    else
    {
        height_min_edit_->setText(String::doubleToString(current_->getHeightMin()).c_str());
        height_max_edit_->setText(String::doubleToString(current_->getHeightMax()).c_str());

        updatePointsTable ();
    }

    used_for_checking_->setChecked(current_->getUsedForChecking());
}

void AirspaceSectorWidget::clearContent ()
{
    assert (name_edit_);
    name_edit_->setText("");

    assert (table_);
    table_->clear();

    current_=0;

    disableWidgets ();
}

void AirspaceSectorWidget::disableWidgets ()
{
    name_edit_->setDisabled(true);
    has_own_volume_->setDisabled(true);
    height_min_edit_->setDisabled(true);
    height_max_edit_->setDisabled(true);
    used_for_checking_->setDisabled(true);

    delete_button_->setDisabled(true);
    clear_button_->setDisabled(true);
    add_button_->setDisabled(true);
    copy_button_->setDisabled(true);

    table_->setDisabled(true);
}
void AirspaceSectorWidget::enableWidgets ()
{
    name_edit_->setDisabled(false);
    has_own_volume_->setDisabled(false);
    height_min_edit_->setDisabled(false);
    height_max_edit_->setDisabled(false);
    used_for_checking_->setDisabled(false);

    delete_button_->setDisabled(false);
    clear_button_->setDisabled(false);
    add_button_->setDisabled(false);
    copy_button_->setDisabled(false);

    table_->setDisabled(false);
}

void AirspaceSectorWidget::setName ()
{
    assert (name_edit_);
    assert (current_);
    current_->setName(name_edit_->text().toStdString());

    emit sectorChanged ();
}

void AirspaceSectorWidget::deleteSector ()
{
    if (!AirspaceSectorManager::getInstance().deleteSectorIfPossible(current_))
    {
        assert (current_);
        delete current_;
    }
    clearContent();
    emit sectorChanged ();
}

void AirspaceSectorWidget::updatePointsTable ()
{
    assert (current_);

    std::vector <Vector2> &points = current_->getOwnPoints ();
    std::vector <Vector2>::iterator it;

    table_->setRowCount(points.size());

    unsigned int row = 0;
    for (it = points.begin(); it != points.end(); it++)
     {

         QTableWidgetItem *newItem;

         std::string latitude = String::doubleToStringPrecision(it->x_, 12);
         std::string longitude = String::doubleToStringPrecision(it->y_, 12);

         newItem = new QTableWidgetItem(latitude.c_str());
         newItem->setFlags(Qt::ItemIsEnabled);
         table_->setItem(row, 0, newItem);

         newItem = new QTableWidgetItem(longitude.c_str());
         newItem->setFlags(Qt::ItemIsEnabled);
         table_->setItem(row, 1, newItem);

         row++;
     }

     table_->resizeColumnsToContents();

}

void AirspaceSectorWidget::setHasOwnVolume ()
{
    bool checked = has_own_volume_->checkState() == Qt::Checked;
    assert (current_);

    current_->setHasOwnVolume(checked);

    if (!checked)
        clearPoints();
    else
        showSector(current_);
}
void AirspaceSectorWidget::setHeightMin()
{
    double value = String::doubleFromString(height_min_edit_->text().toStdString());
    assert (current_);
    assert (current_->hasOwnVolume());
    current_->setHeightMin(value);
}
void AirspaceSectorWidget::setHeightMax ()
{
    double value = String::doubleFromString(height_max_edit_->text().toStdString());
    assert (current_);
    assert (current_->hasOwnVolume());
    current_->setHeightMax(value);
}

void AirspaceSectorWidget::setUseForChecking ()
{
    assert (used_for_checking_);
    bool checked = used_for_checking_->checkState() == Qt::Checked;
    assert (current_);

    current_->setUsedForChecking(checked);
}

void AirspaceSectorWidget::addPoints ()
{
    QClipboard *clipboard = QApplication::clipboard();
    std::string text = clipboard->text().toStdString();

    assert (current_);
    current_->addPoints(text);
}

void AirspaceSectorWidget::clearPoints ()
{
    current_->clearPoints();
    showSector(current_);
}

void AirspaceSectorWidget::copyPoints ()
{
    loginf << "AirspaceSectorWidget: copyPoints: copying points";

    assert (current_);

    std::vector <Vector2> &points = current_->getOwnPoints ();
    std::vector <Vector2>::iterator it;
    std::stringstream ss;

    for (it = points.begin(); it != points.end(); it++)
    {
        ss << String::doubleToStringPrecision(it->x_, 12) << "," << String::doubleToStringPrecision(it->y_, 12) << std::endl;
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ss.str().c_str());
}
