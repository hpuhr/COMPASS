/*
 * OSGView.cpp
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */

#include "osgview.h"
#include "osgviewwidget.h"
#include "osgviewdatasource.h"
#include "osgviewdatawidget.h"
#include "logger.h"
#include "viewselection.h"

OSGView::OSGView(const std::string& class_id, const std::string& instance_id, ViewContainer *w, ViewManager &view_manager)
: View (class_id, instance_id, w, view_manager), widget_(0), data_source_ (0)
{
    registerParameter("map_name", &map_name_, "lod_blending.earth");
    registerParameter("map_opacity", &map_opacity_, 1.0);
    registerParameter("data_opacity", &data_opacity_, 1.0);
    registerParameter("use_height", &use_height_, false);
    registerParameter("use_height_scale", &use_height_scale_, false);
    registerParameter("height_scale_factor", &height_scale_factor_, 10.0);
    registerParameter("clamp_height", &clamp_height_, true);
}

OSGView::~OSGView()
{
  if (data_source_)
    delete data_source_;
  data_source_=0;
}

void OSGView::update( bool atOnce )
{

}

void OSGView::clearData()
{

}

bool OSGView::init()
{
  View::init();

  createSubConfigurables ();

  assert (data_source_);

  //connect( &ViewSelection::getInstance(), SIGNAL(selectionChanged()), this, SLOT(selectionChanged()) );
  //connect( &ViewSelection::getInstance(), SIGNAL(selectionToBeCleared()), this, SLOT(selectionToBeCleared()) );

  connect (data_source_, SIGNAL(loadingStartedSignal ()), widget_->getDataWidget (), SLOT(loadingStartedSlot()));
  connect (data_source_, SIGNAL(updateData (DBObject&, std::shared_ptr<Buffer>)), widget_->getDataWidget (), SLOT(updateData (DBObject&, std::shared_ptr<Buffer>)) );
  connect (this, SIGNAL(mapNameChangedSignal(std::string)),  widget_->getDataWidget (), SLOT(mapNameChangedSlot(std::string)));
  connect (this, SIGNAL(mapOpacityChangedSignal(float)), widget_->getDataWidget (), SLOT(mapOpacityChangedSlot(float)));
  connect (this, SIGNAL(dataOpacityChangedSignal(float)), widget_->getDataWidget (), SLOT(dataOpacityChangedSlot(float)));


  connect (this, SIGNAL(useHeightChangedSignal(bool)), widget_->getDataWidget (), SLOT(useHeightChangedSlot(bool)));
  connect (this, SIGNAL(useHeightScaleChangedSignal(bool)), widget_->getDataWidget (), SLOT(useHeightScaleChangedSlot(bool)));
  connect (this, SIGNAL(heightScaleFactorChangedSignal(float)), widget_->getDataWidget (), SLOT(heightScaleFactorChangedSlot(float)));
  connect (this, SIGNAL(clampHeightChangedSignal(bool)), widget_->getDataWidget (), SLOT(clampHeightChangedSlot(bool)));

  return true;
}

void OSGView::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
  logdbg  << "OSGView: generateSubConfigurable: class_id " << class_id << " instance_id " << instance_id;
  if ( class_id == "OSGViewDataSource" )
  {
    assert (!data_source_);
    data_source_ = new OSGViewDataSource( class_id, instance_id, this );
  }
  else if( class_id == "OSGViewWidget" )
  {
      widget_ = new OSGViewWidget( class_id, instance_id, this, this, central_widget_ );
      setWidget( widget_ );
  }
  else
    throw std::runtime_error ("OSGView: generateSubConfigurable: unknown class_id "+class_id );
}

void OSGView::checkSubConfigurables ()
{
  if (!data_source_)
  {
    generateSubConfigurable ("OSGViewDataSource", "OSGViewDataSource0");
  }

  if( !widget_ )
  {
      generateSubConfigurable ("OSGViewWidget", "OSGViewWidget0");
  }
}

DBOVariableSet OSGView::getSet (const std::string &dbo_name)
{
    assert (data_source_);
    return data_source_->getSet()->getFor(dbo_name);
}

float OSGView::mapOpacity() const
{
    return map_opacity_;
}

void OSGView::mapOpacity(float opacity)
{
    map_opacity_ = opacity;
    assert (map_opacity_ >= 0 && map_opacity_ <= 1.0);

    emit mapOpacityChangedSignal(map_opacity_);
}

float OSGView::dataOpacity() const
{
    return data_opacity_;
}

void OSGView::dataOpacity(float opacity)
{
    data_opacity_ = opacity;
    assert (data_opacity_ >= 0 && data_opacity_ <= 1.0);

    emit dataOpacityChangedSignal(data_opacity_);
}

std::string OSGView::mapName() const
{
    return map_name_;
}

void OSGView::mapName(const std::string &map_name)
{
    map_name_ = map_name;

    emit mapNameChangedSignal (map_name_);
}

float OSGView::heightScaleFactor() const
{
    return height_scale_factor_;
}

void OSGView::heightScaleFactor(float height_scale_factor)
{
    height_scale_factor_ = height_scale_factor;

    emit heightScaleFactorChangedSignal (height_scale_factor_);
}

bool OSGView::useHeight() const
{
    return use_height_;
}

void OSGView::useHeight(bool use_height)
{
    use_height_ = use_height;
    emit useHeightChangedSignal(use_height_);
}

bool OSGView::clampHeight() const
{
    return clamp_height_;
}

void OSGView::clampHeight(bool clamp_height_on_ground)
{
    clamp_height_ = clamp_height_on_ground;
    emit clampHeightChangedSignal(clamp_height_);
}

bool OSGView::useHeightScale() const
{
    return use_height_scale_;
}

void OSGView::useHeightScale(bool use_height_scale)
{
    use_height_scale_ = use_height_scale;
    emit useHeightChangedSignal(use_height_scale_);
}

//void OSGView::updateData ()
//{
//  logdbg  << "OSGView: updateData";
//  assert (data_source_);
//  assert (widget_);

//  //boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();
//  loginf  << "OSGView: " << getName().c_str() << ": loading ";

//  data_source_->updateData();
//  widget_->getDataWidget()->clearTables();
//  //boost::posix_time::ptime stop_time = boost::posix_time::microsec_clock::local_time();

////  boost::posix_time::time_duration diff = stop_time - start_time;
////  double load_time= diff.total_milliseconds()/1000.0;
////
////  loginf  << "OSGView: " << getName().c_str() << ": loading done after " << load_time << " seconds";
//}

void OSGView::selectionChanged()
{
  //  assert (data_source_);
  //  data_source_->updateSelection();
}
void OSGView::selectionToBeCleared()
{

}


