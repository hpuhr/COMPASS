/*
 * ListBoxView.cpp
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */

#include "listboxview.h"
#include "listboxviewwidget.h"
#include "listboxviewdatasource.h"
#include "listboxviewdatawidget.h"
#include "logger.h"
#include "viewselection.h"

ListBoxView::ListBoxView(const std::string& class_id, const std::string& instance_id, ViewContainer *w, ViewManager &view_manager)
: View (class_id, instance_id, w, view_manager), widget_(0), data_source_ (0)
{
}

ListBoxView::~ListBoxView()
{
  if (data_source_)
    delete data_source_;
  data_source_=0;
}

void ListBoxView::update( bool atOnce )
{

}

void ListBoxView::clearData()
{

}

bool ListBoxView::init()
{
  View::init();

  createSubConfigurables ();

  assert (data_source_);

  //connect( &ViewSelection::getInstance(), SIGNAL(selectionChanged()), this, SLOT(selectionChanged()) );
  //connect( &ViewSelection::getInstance(), SIGNAL(selectionToBeCleared()), this, SLOT(selectionToBeCleared()) );

  connect( data_source_, SIGNAL(updateData (DBObject&, std::shared_ptr<Buffer>)), widget_->getDataWidget (), SLOT(updateData (DBObject&, std::shared_ptr<Buffer>)) );

  return true;
}

void ListBoxView::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
  logdbg  << "ListBoxView: generateSubConfigurable: class_id " << class_id << " instance_id " << instance_id;
  if ( class_id == "ListBoxViewDataSource" )
  {
    assert (!data_source_);
    data_source_ = new ListBoxViewDataSource( class_id, instance_id, this );
  }
  else if( class_id == "ListBoxViewWidget" )
  {
      widget_ = new ListBoxViewWidget( class_id, instance_id, this, this, central_widget_ );
      setWidget( widget_ );
  }
  else
    throw std::runtime_error ("ListBoxView: generateSubConfigurable: unknown class_id "+class_id );
}

void ListBoxView::checkSubConfigurables ()
{
  if (!data_source_)
  {
    generateSubConfigurable ("ListBoxViewDataSource", "ListBoxViewDataSource0");
  }

  if( !widget_ )
  {
      generateSubConfigurable ("ListBoxViewWidget", "ListBoxViewWidget0");
  }
}

//void ListBoxView::updateData ()
//{
//  logdbg  << "ListBoxView: updateData";
//  assert (data_source_);
//  assert (widget_);

//  //boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();
//  loginf  << "ListBoxView: " << getName().c_str() << ": loading ";

//  data_source_->updateData();
//  widget_->getDataWidget()->clearTables();
//  //boost::posix_time::ptime stop_time = boost::posix_time::microsec_clock::local_time();

////  boost::posix_time::time_duration diff = stop_time - start_time;
////  double load_time= diff.total_milliseconds()/1000.0;
////
////  loginf  << "ListBoxView: " << getName().c_str() << ": loading done after " << load_time << " seconds";
//}

void ListBoxView::selectionChanged()
{
  //  assert (data_source_);
  //  data_source_->updateSelection();
}
void ListBoxView::selectionToBeCleared()
{

}


