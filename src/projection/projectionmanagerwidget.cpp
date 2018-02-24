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

#include "stringconv.h"
#include "projectionmanager.h"
#include "projectionmanagerwidget.h"

ProjectionManagerWidget::ProjectionManagerWidget(QWidget* parent, Qt::WindowFlags f)
 : QWidget (parent, f)
{
    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    QVBoxLayout *layout = new QVBoxLayout ();

    QLabel *label = new QLabel ("Projection Selection");
    label->setFont (font_big);
    layout->addWidget (label);

    QGridLayout *grid = new QGridLayout ();

    QLabel *world_label = new QLabel ("World Coordinates (WGS84) Info");
    grid->addWidget (world_label, 0, 0);

    world_proj_info_label_ = new QLabel (ProjectionManager::instance().getWorldPROJ4Info().c_str());
    world_proj_info_label_->setWordWrap(true);
    grid->addWidget (world_proj_info_label_, 0, 1);

    QLabel *cart_label = new QLabel ("Cartesian Coordinates EPSG");
    grid->addWidget (cart_label, 1, 0);

    epsg_edit_ = new QLineEdit ();
    epsg_edit_->setText(std::to_string(ProjectionManager::instance().getEPSG()).c_str());
    connect (epsg_edit_, SIGNAL(returnPressed()), this, SLOT(changeEPSG()));
    epsg_edit_->setToolTip("Please refer to the EPSG number appropriate to your country \n under http://spatialreference.org/ref/epsg/");
    grid->addWidget (epsg_edit_, 1, 1);

    QLabel *cart_info_label = new QLabel ("Cartesian Coordinates Info");
    grid->addWidget (cart_info_label, 2, 0);

    cart_proj_info_label_ = new QLabel (ProjectionManager::instance().getCartesianPROJ4Info().c_str());
    cart_proj_info_label_->setWordWrap(true);
    grid->addWidget (cart_proj_info_label_, 2, 1);

    layout->addLayout (grid);

    setLayout (layout);
}

ProjectionManagerWidget::~ProjectionManagerWidget()
{
}

void ProjectionManagerWidget::changeEPSG ()
{
    assert (epsg_edit_);
    assert (cart_proj_info_label_);

    std::string value_str = epsg_edit_->text().toStdString();

    try
    {
        unsigned int value = std::stoul(value_str);
        ProjectionManager::instance().setNewCartesianEPSG(value);
        cart_proj_info_label_->setText(ProjectionManager::instance().getCartesianPROJ4Info().c_str());
    }
    catch (...)
    {
        std::string msg = "Forbidden value '"+value_str
                +"'\n Please refer to http://spatialreference.org/ref/epsg/ for possible numbers.";

        QMessageBox::warning ( this, "Change EPSG Value", msg.c_str());
        epsg_edit_->setText(std::to_string(ProjectionManager::instance().getEPSG()).c_str());
        return;
    }


}
