/*
 * OSGViewDataWidget.cpp
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */

#include <QTabWidget>
#include <QHBoxLayout>

#include <osg/ref_ptr>
#include <osg/Camera>
#include <osg/ShapeDrawable>
#include <osg/StateSet>
#include <osg/Material>
#include <osgGA/EventQueue>
#include <osgGA/TrackballManipulator>
//#include <osgEarthQt/ViewerWidget>

#include <osgEarth/GeoTransform>
#include <osgEarth/Map>
#include <osgEarth/MapNode>
#include <osgEarth/Registry>
#include <osgEarthDrivers/tms/TMSOptions>
#include <osgEarthDrivers/gdal/GDALOptions>
#include <osgEarthUtil/LogarithmicDepthBuffer>

#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Billboard>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
//#include <osgDB/Registry>
//#include <osgDB/ReadFile>
#include <osg/PositionAttitudeTransform>

#include "buffertablewidget.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "osgviewdatawidget.h"
#include "osgviewdatasource.h"
#include "buffer.h"
#include "logger.h"
#include "atsdb.h"

using namespace osg;
using namespace osgEarth;
using namespace osgEarth::Util;
using namespace osgEarth::Drivers;

OSGViewDataWidget::OSGViewDataWidget(OSGViewDataSource *data_source, qreal scaleX, qreal scaleY, QWidget* parent)
    : QOpenGLWidget(parent), data_source_ (data_source), graphics_window_ (new osgViewer::GraphicsWindowEmbedded( this->x(), this->y(), this->width(), this->height())),
      viewer_(new osgViewer::Viewer), scale_x_(scaleX), scale_y_(scaleY)
{
    setup();
}

OSGViewDataWidget::~OSGViewDataWidget()
{

}

void OSGViewDataWidget::setScale(qreal X, qreal Y)
{
    scale_x_ = X;
    scale_y_ = Y;
    this->resizeGL(this->width(), this->height());
}

void OSGViewDataWidget::paintGL()
{
  viewer_->frame();
}

void OSGViewDataWidget::resizeGL( int width, int height )
{
    this->getEventQueue()->windowResize(this->x()*scale_x_, this->y() * scale_y_, width*scale_x_, height*scale_y_);
    graphics_window_->resized(this->x()*scale_x_, this->y() * scale_y_, width*scale_x_, height*scale_y_);
    osg::Camera* camera = viewer_->getCamera();
    camera->setViewport(0, 0, this->width()*scale_x_, this->height()* scale_y_);
}

void OSGViewDataWidget::mouseMoveEvent(QMouseEvent* event)
{
    this->getEventQueue()->mouseMotion(event->x()*scale_x_, event->y()*scale_y_);
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
    this->getEventQueue()->mouseButtonPress(event->x()*scale_x_, event->y()*scale_y_, button);
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
    this->getEventQueue()->mouseButtonRelease(event->x()*scale_x_, event->y()*scale_y_, button);
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
  osgGA::EventQueue* eventQueue = graphics_window_->getEventQueue();
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

void OSGViewDataWidget::setup ()
{
    //osg::Group* rootNode = new osg::Group();
    //Registry::shaderGenerator().run(rootNode);

    osg::Node* loadedModel = osgDB::readNodeFile("data/maps/openstreetmap_flat.earth");
    //osg::Node* loadedModel = osgDB::readNodeFile("data/maps/openstreetmap.earth");
    //osg::Node* loadedModel = osgDB::readNodeFile("data/maps/lod_blending.earth");
    // Find the MapNode
    osgEarth::MapNode* mapNode = MapNode::get( loadedModel );
    //rootNode->addChild(mapNode);

    GeoTransform* xform = new GeoTransform();
    xform->setTerrain( mapNode->getTerrain() );

    const SpatialReference* srs =mapNode->getTerrain()->getSRS();

    GeoPoint point(srs, -121.0, 34.0);
    xform->setPosition(point);

//    Map* map = new Map();

//    TMSOptions driverOpt;
//    driverOpt.url() = "http://tile.openstreetmap.org/";
//    driverOpt.tmsType() = "google";

//    ImageLayerOptions layerOpt( "OSM", driverOpt );
//    //layerOpt.profile() = ProfileOptions( "global-mercator" );

//    ImageLayer* osmLayer = new ImageLayer( layerOpt );
//    osmLayer->setOpacity( 0.5 );

//    map->addImageLayer( osmLayer );

//    MapNode* mapNode = new MapNode( map );

    osg::Camera* camera = new osg::Camera;

    LogarithmicDepthBuffer logdepth;
    logdepth.install(camera);

//    Vec3d eye( 1000.0, 1000.0, 0.0 );
//    Vec3d center( 0.0, 0.0, 0.0 );
//    Vec3d up( 0.0, 0.0, 1.0 );

//    camera->setViewMatrixAsLookAt( eye, center, up );

    osg::Billboard *billy = createBillboards ();
    Registry::shaderGenerator().run(billy);

    mapNode->addChild(billy);


    camera->setViewport( 0, 0, this->width(), this->height() );
    camera->setClearColor( osg::Vec4( 0.f, 0.f, 0.f, 0.f ) );
    float aspectRatio = static_cast<float>( this->width()) / static_cast<float>( this->height() );
    camera->setProjectionMatrixAsPerspective( 30.f, aspectRatio, 0.001f, 1000.f );
    camera->setGraphicsContext( graphics_window_ );

    // setup
    viewer_->setCamera(camera);

    viewer_->setSceneData(mapNode);
    osgGA::TrackballManipulator* manipulator = new osgGA::TrackballManipulator;
    manipulator->setAllowThrow( false );
    this->setMouseTracking(true);
    viewer_->setCameraManipulator(manipulator);
    viewer_->setThreadingModel(osgViewer::Viewer::SingleThreaded);
    viewer_->realize();
}

osg::Billboard* OSGViewDataWidget::createBillboards ()
{
    osg::Billboard* shrubBillBoard = new osg::Billboard();

    shrubBillBoard->setMode(osg::Billboard::POINT_ROT_EYE);
    shrubBillBoard->setAxis(osg::Vec3(0.0f,0.0f,1.0f));
    shrubBillBoard->setNormal(osg::Vec3(0.0f,-1.0f,0.0f));

    osg::Texture2D *ocotilloTexture = new osg::Texture2D;
    ocotilloTexture->setImage(osgDB::readImageFile("data/icons/delete.png"));

    osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
    alphaFunc->setFunction(osg::AlphaFunc::GEQUAL,0.05f);

    osg::StateSet* billBoardStateSet = new osg::StateSet;

    billBoardStateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    billBoardStateSet->setTextureAttributeAndModes
          (0, ocotilloTexture, osg::StateAttribute::ON );
    billBoardStateSet->setAttributeAndModes
          (new osg::BlendFunc, osg::StateAttribute::ON );
    osg::AlphaFunc* alphaFunction = new osg::AlphaFunc;
    alphaFunction->setFunction(osg::AlphaFunc::GEQUAL,0.05f);
    billBoardStateSet->setAttributeAndModes( alphaFunc, osg::StateAttribute::ON );

    osg::Drawable* shrub1Drawable = createShrub( 1.0f, billBoardStateSet);
    osg::Drawable* shrub2Drawable = createShrub( 2.0f, billBoardStateSet);
    osg::Drawable* shrub3Drawable = createShrub( 1.2f, billBoardStateSet);

    // Add these drawables to our billboard at various positions
    shrubBillBoard->addDrawable( shrub1Drawable , osg::Vec3(120,30,80) );
    shrubBillBoard->addDrawable( shrub2Drawable , osg::Vec3(100,180,80));
    shrubBillBoard->addDrawable( shrub3Drawable , osg::Vec3(60,100,80) );

    return shrubBillBoard;
}

osg::Drawable* OSGViewDataWidget::createShrub(const float &scale, osg::StateSet* bbState)
{
   float width = 1.5f;
   float height = 3.0f;

   width *= scale;
   height *= scale;

   osg::Geometry* shrubQuad = new osg::Geometry;

   osg::Vec3Array* shrubVerts = new osg::Vec3Array(4);
   (*shrubVerts)[0] = osg::Vec3(-width/2.0f, 0, 0);
   (*shrubVerts)[1] = osg::Vec3( width/2.0f, 0, 0);
   (*shrubVerts)[2] = osg::Vec3( width/2.0f, 0, height);
   (*shrubVerts)[3] = osg::Vec3(-width/2.0f, 0, height);

   shrubQuad->setVertexArray(shrubVerts);

   osg::Vec2Array* shrubTexCoords = new osg::Vec2Array(4);
   (*shrubTexCoords)[0].set(0.0f,0.0f);
   (*shrubTexCoords)[1].set(1.0f,0.0f);
   (*shrubTexCoords)[2].set(1.0f,1.0f);
   (*shrubTexCoords)[3].set(0.0f,1.0f);
   shrubQuad->setTexCoordArray(0,shrubTexCoords);

   shrubQuad->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));

   // Need to assign a color to the underlying geometry, otherwise we'll get
   // whatever color is current applied to our geometry.
   // Create a color array, add a single color to use for all the vertices

   osg::Vec4Array* colorArray = new osg::Vec4Array;
   colorArray->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f) ); // white, fully opaque

   // An index array for assigning vertices to colors (based on index in the array)
   osg::TemplateIndexArray
      <unsigned int, osg::Array::UIntArrayType,4,1> *colorIndexArray;
   colorIndexArray =
      new osg::TemplateIndexArray<unsigned int, osg::Array::UIntArrayType,4,1>;
   colorIndexArray->push_back(0);

   // Use the index array to associate the first entry in our index array with all
   // of the vertices.
   shrubQuad->setColorArray( colorArray);
   //shrubQuad->setColorIndices(colorIndexArray);
   shrubQuad->setColorBinding(osg::Geometry::BIND_OVERALL);

   shrubQuad->setStateSet(bbState);

   return shrubQuad;
}
