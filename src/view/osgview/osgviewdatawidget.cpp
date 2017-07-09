/*
 * OSGViewDataWidget.cpp
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */

#include <QTabWidget>
#include <QHBoxLayout>

#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>
#include <osgQt/GraphicsWindowQt>
#include <QGLWidget>

#include "buffertablewidget.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "osgviewdatawidget.h"
#include "osgviewdatasource.h"
#include "buffer.h"
#include "logger.h"
#include "atsdb.h"

OSGViewDataWidget::OSGViewDataWidget(OSGViewDataSource *data_source, osgViewer::ViewerBase::ThreadingModel threadingModel,
                                     QWidget * parent, Qt::WindowFlags f)
    : QWidget (parent, f), data_source_ (data_source)
{
    assert (data_source_);

    QHBoxLayout *layout = new QHBoxLayout ();

    setThreadingModel(threadingModel);

    // disable the default setting of viewer.done() by pressing Escape.
    setKeyEventSetsDone(0);

    QWidget* widget1 = addViewWidget( createGraphicsWindow(0,0,100,100), osgDB::readNodeFile("cow.osgt") );
//    QWidget* widget2 = addViewWidget( createGraphicsWindow(0,0,100,100), osgDB::readNodeFile("glider.osgt") );
//    QWidget* widget3 = addViewWidget( createGraphicsWindow(0,0,100,100), osgDB::readNodeFile("axes.osgt") );
//    QWidget* widget4 = addViewWidget( createGraphicsWindow(0,0,100,100), osgDB::readNodeFile("fountain.osgt") );
//    QWidget* popupWidget = addViewWidget( createGraphicsWindow(900,100,320,240,"Popup window",true), osgDB::readNodeFile("dumptruck.osgt") );
//    popupWidget->show();

//    QGridLayout* grid = new QGridLayout;
//    grid->addWidget( widget1, 0, 0 );
//    grid->addWidget( widget2, 0, 1 );
//    grid->addWidget( widget3, 1, 0 );
//    grid->addWidget( widget4, 1, 1 );
//    setLayout( grid );

    layout->addWidget(widget1);

    connect( &timer_, SIGNAL(timeout()), this, SLOT(update()) );
    timer_.start( 10 );

    setLayout (layout);
}

OSGViewDataWidget::~OSGViewDataWidget()
{
    // TODO
    //buffer_tables_.clear();
}

//void OSGViewDataWidget::clearTables ()
//{
//    logdbg  << "OSGViewDataWidget: updateTables: start";
//    // TODO
//    //  std::map <DB_OBJECT_TYPE, BufferTableWidget*>::iterator it;

//    //  for (it = buffer_tables_.begin(); it != buffer_tables_.end(); it++)
//    //  {
//    //    it->second->show (0, 0, false);
//    //  }

//    logdbg  << "OSGViewDataWidget: updateTables: end";
//}

void OSGViewDataWidget::loadingStartedSlot()
{
//    for (auto buffer_table : buffer_tables_)
//        buffer_table.second->clear();
}

void OSGViewDataWidget::updateData (DBObject &object, std::shared_ptr<Buffer> buffer)
{
    logdbg  << "OSGViewDataWidget: updateTables: start";
    //assert (buffer_tables_.count (object.name()) > 0);
    //buffer_tables_.at(object.name())->show(buffer); //, data_source_->getSet()->getFor(type), data_source_->getDatabaseView()

    logdbg  << "OSGViewDataWidget: updateTables: end";
}


QWidget* OSGViewDataWidget::addViewWidget( osgQt::GraphicsWindowQt* gw, osg::Node* scene )
{
    osgViewer::View* view = new osgViewer::View;
    addView( view );

    osg::Camera* camera = view->getCamera();
    camera->setGraphicsContext( gw );

    const osg::GraphicsContext::Traits* traits = gw->getTraits();

    camera->setClearColor( osg::Vec4(0.2, 0.2, 0.6, 1.0) );
    camera->setViewport( new osg::Viewport(0, 0, traits->width, traits->height) );
    camera->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(traits->width)/static_cast<double>(traits->height), 1.0f, 10000.0f );

    view->setSceneData( scene );
    view->addEventHandler( new osgViewer::StatsHandler );
    view->setCameraManipulator( new osgGA::TrackballManipulator );

    return gw->getGLWidget();
}

osgQt::GraphicsWindowQt* OSGViewDataWidget::createGraphicsWindow (int x, int y, int w, int h, const std::string& name, bool windowDecoration)
{
    osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->windowName = name;
    traits->windowDecoration = windowDecoration;
    traits->x = x;
    traits->y = y;
    traits->width = w;
    traits->height = h;
    traits->doubleBuffer = true;
    traits->alpha = ds->getMinimumNumAlphaBits();
    traits->stencil = ds->getMinimumNumStencilBits();
    traits->sampleBuffers = ds->getMultiSamples();
    traits->samples = ds->getNumMultiSamples();

    return new osgQt::GraphicsWindowQt(traits.get());
}

void OSGViewDataWidget::paintEvent( QPaintEvent* event )
{
    frame();
}

