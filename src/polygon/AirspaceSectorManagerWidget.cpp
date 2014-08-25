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
 * AirspaceSectorManagerWidget.cpp
 *
 *  Created on: Nov 25, 2013
 *      Author: sk
 */

#include "AirspaceSectorManagerWidget.h"
#include "AirspaceSectorTreeWidget.h"
#include "AirspaceSectorWidget.h"
#include "AirspaceSectorManager.h"
#include "Logger.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QInputDialog>
#include <QPushButton>
#include <QFileDialog>

AirspaceSectorManagerWidget::AirspaceSectorManagerWidget(QWidget *parent, Qt::WindowFlags f)
 : QWidget (parent, f), yggdrasil_(0), sector_widget_(0)
{
    setMinimumSize(1200, 800);
    createElements();

}

AirspaceSectorManagerWidget::~AirspaceSectorManagerWidget()
{
}

void AirspaceSectorManagerWidget::createElements ()
{
    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    QVBoxLayout *vlayout = new QVBoxLayout;

    QLabel *label = new QLabel ("Airspace Sector Configuration");
    label->setFont (font_big);
    vlayout->addWidget (label);

    QHBoxLayout *blayout = new QHBoxLayout ();

    QPushButton *new_button = new QPushButton("Add Sector");
    connect(new_button, SIGNAL( clicked() ), this, SLOT( addNewSector() ));
    blayout->addWidget (new_button);

    QPushButton *acg_button = new QPushButton("Add Sector By ACG XML File");
    connect(acg_button, SIGNAL( clicked() ), this, SLOT( addSectorsByACGXMLFile() ));
    blayout->addWidget (acg_button);

    QPushButton *shp_button = new QPushButton("Add Sector By Shapefile");
    connect(shp_button, SIGNAL( clicked() ), this, SLOT( addSectorsByShapeFile() ));
    blayout->addWidget (shp_button);

    vlayout->addLayout (blayout);

    QHBoxLayout *hlayout = new QHBoxLayout;

    yggdrasil_ = new AirspaceSectorTreeWidget ();
    hlayout->addWidget(yggdrasil_);

    sector_widget_ = new AirspaceSectorWidget ();
    hlayout->addWidget(sector_widget_);

    connect (yggdrasil_, SIGNAL (showSector (AirspaceSector *)), sector_widget_, SLOT (showSector (AirspaceSector *)));

    connect (sector_widget_, SIGNAL (sectorChanged ()), yggdrasil_, SLOT (updateLayerListSlot ()));


    vlayout->addLayout (hlayout);

    setLayout( vlayout );
}

void AirspaceSectorManagerWidget::addNewSector ()
{
    bool ok;
    std::string text = QInputDialog::getText ( (QWidget*) this, tr("Add Airspace Sector"), tr("Please enter name (without spaces)"),
            QLineEdit::Normal, "", &ok).toStdString();

    if (ok)
    {
        loginf << "AirspaceSectorManagerWidget: addNewSector: adding new sector '" << text << "'";
        AirspaceSectorManager::getInstance().addNewSector(text);
        yggdrasil_->updateLayerListSlot ();
    }
}

void AirspaceSectorManagerWidget::addSectorsByACGXMLFile ()
{
    ACGXMLImportDialog import;

    import.exec();

    if (import.result() == QDialog::Accepted)
    {
        AirspaceSectorManager::getInstance().createNewSectorFromACGXMLFile(import.getFilename(), import.getSectorName());
    }

    yggdrasil_->updateLayerListSlot();
    sector_widget_->disableWidgets();
}

void AirspaceSectorManagerWidget::addSectorsByShapeFile ()
{
    QString filename = QFileDialog::getOpenFileName (this, tr("Please select a shapefile"), tr("/home/sk"),tr("Shapefiles (*.shp)"));
    AirspaceSectorManager::getInstance().createNewSectorFromShapefile(filename.toStdString());

    yggdrasil_->updateLayerListSlot();
    sector_widget_->disableWidgets();
}


