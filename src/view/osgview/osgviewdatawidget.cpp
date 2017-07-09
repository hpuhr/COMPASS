/*
 * OSGViewDataWidget.cpp
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */

#include <QTabWidget>
#include <QHBoxLayout>



#include "buffertablewidget.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "osgviewdatawidget.h"
#include "osgviewdatasource.h"
#include "buffer.h"
#include "logger.h"
#include "atsdb.h"

OSGViewDataWidget::OSGViewDataWidget(OSGViewDataSource *data_source, qreal scaleX, qreal scaleY, QWidget* parent)
    : QOpenGLWidget(parent), data_source_ (data_source), _mGraphicsWindow (new osgViewer::GraphicsWindowEmbedded( this->x(), this->y(), this->width(), this->height())),
      _mViewer(new osgViewer::Viewer), m_scaleX(scaleX), m_scaleY(scaleY)
{
    //        osg::Cylinder* cylinder    = new osg::Cylinder( osg::Vec3( 0.f, 0.f, 0.f ), 0.25f, 0.5f );
    //        osg::ShapeDrawable* sd = new osg::ShapeDrawable( cylinder );
    //        sd->setColor( osg::Vec4( 0.8f, 0.5f, 0.2f, 1.f ) );

    //osg::Geode* geode = new osg::Geode;
    //osg::Node* globe = osgDB::readNodeFile("data/maps/openstreetmap_flat.earth");
    //geode->addDrawable(globe);

    osg::Node* loadedModel = osgDB::readNodeFile("/home/sk/workspace_cdt/atsdb/data/maps/openstreetmap_flat.earth");

    // Find the MapNode
    osgEarth::MapNode* mapNode = MapNode::get( loadedModel );

    //      Map* map = new Map();

    //      // Add an imagery layer (blue marble from a TMS source)
    //      {
    //          TMSOptions tms;
    //          tms.url() = "http://labs.metacarta.com/wms-c/Basic.py/1.0.0/satellite/";
    //          ImageLayer* layer = new ImageLayer( "NASA", tms );
    //          map->addImageLayer( layer );
    //      }
    //      MapNode* mapNode = new MapNode( map );

    osg::Camera* camera = new osg::Camera;
    camera->setViewport( 0, 0, this->width(), this->height() );
    camera->setClearColor( osg::Vec4( 0.9f, 0.9f, 1.f, 1.f ) );
    float aspectRatio = static_cast<float>( this->width()) / static_cast<float>( this->height() );
    camera->setProjectionMatrixAsPerspective( 30.f, aspectRatio, 1.f, 1000.f );
    camera->setGraphicsContext( _mGraphicsWindow );

    _mViewer->setCamera(camera);
    _mViewer->setSceneData(mapNode);
    osgGA::TrackballManipulator* manipulator = new osgGA::TrackballManipulator;
    manipulator->setAllowThrow( false );
    this->setMouseTracking(true);
    _mViewer->setCameraManipulator(manipulator);
    _mViewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
    _mViewer->realize();
}

OSGViewDataWidget::~OSGViewDataWidget()
{

}

void OSGViewDataWidget::setScale(qreal X, qreal Y)
{
    m_scaleX = X;
    m_scaleY = Y;
    this->resizeGL(this->width(), this->height());
}

void OSGViewDataWidget::paintGL()
{
  _mViewer->frame();
}

void OSGViewDataWidget::resizeGL( int width, int height )
{
    this->getEventQueue()->windowResize(this->x()*m_scaleX, this->y() * m_scaleY, width*m_scaleX, height*m_scaleY);
    _mGraphicsWindow->resized(this->x()*m_scaleX, this->y() * m_scaleY, width*m_scaleX, height*m_scaleY);
    osg::Camera* camera = _mViewer->getCamera();
    camera->setViewport(0, 0, this->width()*m_scaleX, this->height()* m_scaleY);
}

void OSGViewDataWidget::mouseMoveEvent(QMouseEvent* event)
{
    this->getEventQueue()->mouseMotion(event->x()*m_scaleX, event->y()*m_scaleY);
}

void OSGViewDataWidget::mousePressEvent(QMouseEvent* event)
{
    unsigned int button = 0;
    switch (event->button()){
    case Qt::LeftButton:
        button = 1;
        break;
    case Qt::MiddleButton:
        button = 2;
        break;
    case Qt::RightButton:
        button = 3;
        break;
    default:
        break;
    }
    this->getEventQueue()->mouseButtonPress(event->x()*m_scaleX, event->y()*m_scaleY, button);
}

void OSGViewDataWidget::mouseReleaseEvent(QMouseEvent* event)
{
    unsigned int button = 0;
    switch (event->button()){
    case Qt::LeftButton:
        button = 1;
        break;
    case Qt::MiddleButton:
        button = 2;
        break;
    case Qt::RightButton:
        button = 3;
        break;
    default:
        break;
    }
    this->getEventQueue()->mouseButtonRelease(event->x()*m_scaleX, event->y()*m_scaleY, button);
}

void OSGViewDataWidget::wheelEvent(QWheelEvent* event)
{
    int delta = event->delta();
    osgGA::GUIEventAdapter::ScrollingMotion motion = delta > 0 ?
                osgGA::GUIEventAdapter::SCROLL_UP : osgGA::GUIEventAdapter::SCROLL_DOWN;
    this->getEventQueue()->mouseScroll(motion);
}

bool OSGViewDataWidget::event(QEvent* event)
{
    bool handled = QOpenGLWidget::event(event);
    this->update();
    return handled;
}

osgGA::EventQueue* OSGViewDataWidget::getEventQueue() const
{
  osgGA::EventQueue* eventQueue = _mGraphicsWindow->getEventQueue();
  return eventQueue;
}

void OSGViewDataWidget::loadingStartedSlot()
{
    loginf << "OSGViewDataWidget: loadingStartedSlot";
}

void OSGViewDataWidget::updateData (DBObject &object, std::shared_ptr<Buffer> buffer)
{
loginf << "OSGViewDataWidget: updateData";
}
