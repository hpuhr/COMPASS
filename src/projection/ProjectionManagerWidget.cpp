/*
 * ProjectionManagerWidget.cpp
 *
 *  Created on: Nov 21, 2014
 *      Author: sk
 */

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>

#include "String.h"
#include "ProjectionManager.h"
#include "ProjectionManagerWidget.h"

ProjectionManagerWidget::ProjectionManagerWidget(QWidget *parent, Qt::WindowFlags f)
 : QWidget (parent, f), world_proj_info_label_ (0), epsg_edit_ (0), cart_proj_info_label_(0)
{
    createGUIElements();
}

ProjectionManagerWidget::~ProjectionManagerWidget()
{
}

void ProjectionManagerWidget::createGUIElements ()
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

    world_proj_info_label_ = new QLabel (ProjectionManager::getInstance().getWorldPROJ4Info().c_str());
    world_proj_info_label_->setWordWrap(true);
    grid->addWidget (world_proj_info_label_, 0, 1);

    QLabel *cart_label = new QLabel ("Cartesian Coordinates EPSG");
    grid->addWidget (cart_label, 1, 0);

    epsg_edit_ = new QLineEdit ();
    epsg_edit_->setText(Utils::String::intToString(ProjectionManager::getInstance().getEPSG()).c_str());
    connect (epsg_edit_, SIGNAL(returnPressed()), this, SLOT(changeEPSG()));
    epsg_edit_->setToolTip("Please refer to the EPSG number appropriate to your country \n under http://spatialreference.org/ref/epsg/");
    grid->addWidget (epsg_edit_, 1, 1);

    QLabel *cart_info_label = new QLabel ("Cartesian Coordinates Info");
    grid->addWidget (cart_info_label, 2, 0);

    cart_proj_info_label_ = new QLabel (ProjectionManager::getInstance().getCartesianPROJ4Info().c_str());
    cart_proj_info_label_->setWordWrap(true);
    grid->addWidget (cart_proj_info_label_, 2, 1);

    layout->addLayout (grid);

    setLayout (layout);
}

void ProjectionManagerWidget::changeEPSG ()
{
    assert (epsg_edit_);
    assert (cart_proj_info_label_);

    std::string value_str = epsg_edit_->text().toStdString();

    bool ok;
    unsigned int value = Utils::String::intFromString(value_str, &ok);

    if (!ok)
    {
        std::string msg = "Forbidden value '"+value_str+"\n Please refer to http://spatialreference.org/ref/epsg/ for possible numbers";
        QMessageBox::warning ( this, "Change Cartesian Coordinate", msg.c_str());
        epsg_edit_->setText(Utils::String::intToString(ProjectionManager::getInstance().getEPSG()).c_str());
        return;
    }

    ProjectionManager::getInstance().setNewCartesianEPSG(value);
    cart_proj_info_label_->setText(ProjectionManager::getInstance().getCartesianPROJ4Info().c_str());
}
