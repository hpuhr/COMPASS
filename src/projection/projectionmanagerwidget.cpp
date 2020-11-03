/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "projectionmanagerwidget.h"

#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>
#include <cassert>

#include "logger.h"
#include "projection.h"
#include "projectionmanager.h"
#include "stringconv.h"

ProjectionManagerWidget::ProjectionManagerWidget(ProjectionManager& proj_man, QWidget* parent,
                                                 Qt::WindowFlags f)
    : QWidget(parent, f), projection_manager_(proj_man)
{
    setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* main_layout = new QVBoxLayout();
    main_layout->setContentsMargins(0, 0, 0, 0);

    QGridLayout* grid = new QGridLayout();
    grid->setContentsMargins(0, 0, 0, 0);

    grid->addWidget(new QLabel("Projection Method"), 0, 0);

    projection_box_ = new QComboBox();

    for (auto& proj_it : projection_manager_.projections())
        projection_box_->addItem(proj_it.first.c_str());

    if (projection_manager_.hasCurrentProjection())
        projection_box_->setCurrentText(projection_manager_.currentProjectionName().c_str());

    connect(projection_box_, SIGNAL(currentIndexChanged(const QString&)), this,
            SLOT(selectedObjectParserSlot(const QString&)));

    grid->addWidget(projection_box_, 0, 1);

    main_layout->addLayout(grid);

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

    setLayout(main_layout);
}

ProjectionManagerWidget::~ProjectionManagerWidget() {}

// void ProjectionManagerWidget::changedEPSGSlot ()
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
//                +"'\n Please refer to http://spatialreference.org/ref/epsg/ for possible
//                numbers.";

//        QMessageBox::warning ( this, "Change EPSG Value", msg.c_str());
//        epsg_edit_->setText(std::to_string(projection_manager_.getEPSG()).c_str());
//        return;
//    }
//}

void ProjectionManagerWidget::selectedObjectParserSlot(const QString& name)
{
    loginf << "ProjectionManagerWidget: selectedObjectParserSlot: name " << name.toStdString();

    assert(projection_manager_.hasProjection(name.toStdString()));
    projection_manager_.currentProjectionName(name.toStdString());
}
