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

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QRadioButton>
#include <QGroupBox>

#include "stringconv.h"
#include "projectionmanager.h"
#include "projectionmanagerwidget.h"

ProjectionManagerWidget::ProjectionManagerWidget(ProjectionManager& proj_man, QWidget* parent, Qt::WindowFlags f)
 : QWidget (parent, f), projection_manager_(proj_man)
{
    QVBoxLayout *main_layout = new QVBoxLayout ();

//    QGroupBox *groupBox = new QGroupBox(tr("Projection Selection"));

//    QVBoxLayout *layout = new QVBoxLayout ();

//    ogr_radio_ = new QRadioButton ("OGR Projection");
//    ogr_radio_->setChecked(projection_manager_.useOGRProjection());
//    connect (ogr_radio_, &QRadioButton::toggled, this, &ProjectionManagerWidget::projectionChangedSlot);
//    layout->addWidget(ogr_radio_);

//    QGridLayout *grid = new QGridLayout ();

//    QLabel *world_label = new QLabel ("World Coordinates (WGS84) Info");
//    grid->addWidget (world_label, 0, 0);

//    world_proj_info_label_ = new QLabel (projection_manager_.getWorldPROJ4Info().c_str());
//    world_proj_info_label_->setWordWrap(true);
//    grid->addWidget (world_proj_info_label_, 0, 1);

//    QLabel *cart_label = new QLabel ("Cartesian Coordinates EPSG");
//    grid->addWidget (cart_label, 1, 0);

//    epsg_edit_ = new QLineEdit ();
//    epsg_edit_->setText(std::to_string(projection_manager_.getEPSG()).c_str());
//    connect (epsg_edit_, SIGNAL(returnPressed()), this, SLOT(changedEPSGSlot()));
//    epsg_edit_->setToolTip("Please refer to the EPSG number appropriate to your country \n"
//                           "under http://spatialreference.org/ref/epsg/");
//    grid->addWidget (epsg_edit_, 1, 1);

//    QLabel *cart_info_label = new QLabel ("Cartesian Coordinates Info");
//    grid->addWidget (cart_info_label, 2, 0);

//    cart_proj_info_label_ = new QLabel (projection_manager_.getCartesianPROJ4Info().c_str());
//    cart_proj_info_label_->setWordWrap(true);
//    grid->addWidget (cart_proj_info_label_, 2, 1);

//    layout->addLayout (grid);

//    sdl_radio_ = new QRadioButton ("SDL Projection");
//    //ogr_radio_->setChecked(projection_manager_.useSDLProjection());
//    //connect (sdl_radio_, &QRadioButton::toggled, this, &ProjectionManagerWidget::projectionChangedSlot);
//    //sdl_radio_->setChecked(projection_manager_.useSDLProjection());
//    sdl_radio_->setDisabled(true);
//    //layout->addWidget(sdl_radio_);

//    rs2g_radio_  = new QRadioButton ("RS2G Projection");
//    connect (rs2g_radio_, &QRadioButton::toggled, this, &ProjectionManagerWidget::projectionChangedSlot);
//    rs2g_radio_->setChecked(projection_manager_.useRS2GProjection());
//    layout->addWidget(rs2g_radio_);

//    groupBox->setLayout(layout);

//    main_layout->addWidget(groupBox);

    setLayout (main_layout);
}

ProjectionManagerWidget::~ProjectionManagerWidget()
{
}

//void ProjectionManagerWidget::changedEPSGSlot ()
//{
//    assert (epsg_edit_);
//    assert (cart_proj_info_label_);

//    std::string value_str = epsg_edit_->text().toStdString();

//    try
//    {
//        unsigned int value = std::stoul(value_str);
//        ProjectionManager::instance().setNewCartesianEPSG(value);
//        cart_proj_info_label_->setText(projection_manager_.getCartesianPROJ4Info().c_str());
//    }
//    catch (...)
//    {
//        std::string msg = "Forbidden value '"+value_str
//                +"'\n Please refer to http://spatialreference.org/ref/epsg/ for possible numbers.";

//        QMessageBox::warning ( this, "Change EPSG Value", msg.c_str());
//        epsg_edit_->setText(std::to_string(projection_manager_.getEPSG()).c_str());
//        return;
//    }
//}

void ProjectionManagerWidget::projectionChangedSlot()
{
//    assert (ogr_radio_);
//    assert (sdl_radio_);
//    assert (rs2g_radio_);

//    if (ogr_radio_->isChecked())
//        projection_manager_.useOGRProjection(true);

////    if (sdl_radio_->isChecked())
////        projection_manager_.useSDLProjection(true);

//    if (rs2g_radio_->isChecked())
//        projection_manager_.useRS2GProjection(true);
}
