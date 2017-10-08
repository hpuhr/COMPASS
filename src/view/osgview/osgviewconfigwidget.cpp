/*
 * OSGViewConfigWidget.cpp
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */

#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QSlider>
#include <QGroupBox>
#include <QRadioButton>

#include "dbobjectmanager.h"
#include "osgview.h"
#include "osgviewconfigwidget.h"
#include "osgviewdatasource.h"
#include "logger.h"
#include "stringconv.h"

using namespace Utils;

static const float OPACITY_SLIDE_VALUES = 20;

OSGViewConfigWidget::OSGViewConfigWidget( OSGView* view, QWidget* parent )
    :   QWidget( parent ), view_( view )
{
    assert (view_);
    QVBoxLayout *vlayout = new QVBoxLayout;

    QGroupBox *map_box = new QGroupBox(tr("Map Layer"));

    std::string map_name = view_->mapName();

    //QRadioButton *radio1 = new QRadioButton(tr("None"));
    QRadioButton *radio1 = new QRadioButton(tr("Globe Satellite (ReadyMap)"));
    map_names_[radio1] = "lod_blending.earth";

    QRadioButton *radio2 = new QRadioButton(tr("Globe Map (OSM)"));
    map_names_[radio2] = "openstreetmap.earth";

    QRadioButton *radio3 = new QRadioButton(tr("Flat Map (OSM)"));
    map_names_[radio3] = "openstreetmap_flat.earth";

    for (auto it : map_names_)
    {
        if (map_name == it.second)
            it.first->setChecked (true);
    }

    connect (radio1, SIGNAL(toggled(bool)), this, SLOT(mapSelectedSlot(bool)));
    connect (radio3, SIGNAL(toggled(bool)), this, SLOT(mapSelectedSlot(bool)));
    connect (radio2, SIGNAL(toggled(bool)), this, SLOT(mapSelectedSlot(bool)));


    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(radio1);
    vbox->addWidget(radio2);
    vbox->addWidget(radio3);
    //vbox->addWidget(radio4);
    //vbox->addStretch(1);
    map_box->setLayout(vbox);
    vlayout->addWidget(map_box);

    vlayout->addWidget(new QLabel ("Map Opacity"));

    int opacity = OPACITY_SLIDE_VALUES*view_->mapOpacity()/OPACITY_SLIDE_VALUES;

    map_opacity_slider_ = new QSlider (Qt::Horizontal);
    map_opacity_slider_->setMinimum(0);
    map_opacity_slider_->setSingleStep(2);
    map_opacity_slider_->setMaximum(OPACITY_SLIDE_VALUES);
    map_opacity_slider_->setValue(opacity);
    connect (map_opacity_slider_, SIGNAL(valueChanged(int)), SLOT(mapOpacityChangedSlot()));
    vlayout->addWidget(map_opacity_slider_);

    vlayout->addWidget(new QLabel ("Data Opacity"));

    opacity = OPACITY_SLIDE_VALUES*view_->dataOpacity();
    data_opacity_slider_ = new QSlider (Qt::Horizontal);
    data_opacity_slider_->setMinimum(0);
    data_opacity_slider_->setSingleStep(2);
    data_opacity_slider_->setMaximum(OPACITY_SLIDE_VALUES);
    data_opacity_slider_->setValue(opacity);
    connect (data_opacity_slider_, SIGNAL(valueChanged(int)), SLOT(dataOpacityChangedSlot()));
    vlayout->addWidget(data_opacity_slider_);

    use_height_check_ = new QCheckBox ("Use Height");
    use_height_check_->setChecked(view->useHeight());
    connect (use_height_check_, SIGNAL(toggled(bool)), this, SLOT(useHeightSlot(bool)));
    vlayout->addWidget(use_height_check_);

    height_scale_check_ = new QCheckBox ("Scale Height");
    height_scale_check_->setChecked(view->useHeightScale());
    connect (height_scale_check_, SIGNAL(toggled(bool)), this, SLOT(useHeightScaleSlot(bool)));
    vlayout->addWidget(height_scale_check_);

    vlayout->addWidget(new QLabel ("Height Scale Factor"));

    float height_factor = view_->heightScaleFactor()/10.0;
    height_scale_slider_ = new QSlider (Qt::Horizontal);
    height_scale_slider_->setMinimum(1);
    height_scale_slider_->setSingleStep(1);
    height_scale_slider_->setMaximum(10);
    height_scale_slider_->setValue(height_factor);
    connect (height_scale_slider_, SIGNAL(valueChanged(int)), SLOT(heightScaleFactorChangedSlot()));
    vlayout->addWidget(height_scale_slider_);

    height_clamp_check_ = new QCheckBox ("Clamp Height on Ground");
    height_clamp_check_->setChecked(view->clampHeight());
    connect (height_clamp_check_, SIGNAL(toggled(bool)), this, SLOT(heightClampChangedSlot(bool)));
    vlayout->addWidget(height_clamp_check_);

    vlayout->addStretch();
    setLayout (vlayout);
}

OSGViewConfigWidget::~OSGViewConfigWidget()
{
}

void OSGViewConfigWidget::mapSelectedSlot (bool selected)
{
    if (selected)
    {
        QRadioButton* radio = dynamic_cast<QRadioButton*> (QObject::sender());
        assert (radio);
        assert (map_names_.count(radio) == 1);
        view_->mapName(map_names_.at(radio));
    }
}

void OSGViewConfigWidget::mapOpacityChangedSlot ()
{
    assert (map_opacity_slider_);
    int opacity = map_opacity_slider_->value();
    assert (opacity >= 0 && opacity <= OPACITY_SLIDE_VALUES);
    view_->mapOpacity(static_cast<float> (opacity/OPACITY_SLIDE_VALUES));
}

void OSGViewConfigWidget::dataOpacityChangedSlot ()
{
    assert (data_opacity_slider_);
    int opacity = data_opacity_slider_->value();
    assert (opacity >= 0 && opacity <= OPACITY_SLIDE_VALUES);
    view_->dataOpacity(static_cast<float> (opacity/OPACITY_SLIDE_VALUES));
}

void OSGViewConfigWidget::useHeightSlot (bool checked)
{
    view_->useHeight(checked);
}

void OSGViewConfigWidget::useHeightScaleSlot (bool checked)
{
    view_->useHeightScale(checked);
}

void OSGViewConfigWidget::heightScaleFactorChangedSlot ()
{
    assert (height_scale_slider_);
    int factor = height_scale_slider_->value()*10;
    assert (factor >= 0 && factor <= 100.0);
    view_->heightScaleFactor(static_cast<float> (factor));
}

void OSGViewConfigWidget::heightClampChangedSlot(bool checked)
{
    view_->clampHeight(checked);
}
