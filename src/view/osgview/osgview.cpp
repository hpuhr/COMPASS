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

  connect( data_source_, SIGNAL(loadingStartedSignal ()), widget_->getDataWidget (), SLOT(loadingStartedSlot()));
  connect( data_source_, SIGNAL(updateData (DBObject&, std::shared_ptr<Buffer>)), widget_->getDataWidget (), SLOT(updateData (DBObject&, std::shared_ptr<Buffer>)) );

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


