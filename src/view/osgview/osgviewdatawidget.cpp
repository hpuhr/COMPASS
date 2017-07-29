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
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/Array>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Image>
#include <osg/Notify>
#include <osg/BlendFunc>
#include <osg/BlendEquation>
#include <osg/PointSprite>
#include <osg/Point>
#include <osg/LineWidth>
#include <osgDB/ReadFile>
#include <osg/ShapeDrawable>
#include <osgEarthAnnotation/AnnotationUtils>
#include <osg/Material>

#include <osgEarth/GeoTransform>
#include <osgEarth/Map>
#include <osgEarth/MapNode>
#include <osgEarth/Registry>
#include <osgEarthDrivers/tms/TMSOptions>
#include <osgEarthDrivers/gdal/GDALOptions>
#include <osgEarthUtil/LogarithmicDepthBuffer>
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
      viewer_(new osgViewer::Viewer), root_node_(nullptr), scale_x_(scaleX), scale_y_(scaleY)
{
    setup();
}

OSGViewDataWidget::~OSGViewDataWidget()
{
    assert (root_node_);
    root_node_->releaseGLObjects();
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
    this->getEventQueue()->mouseMotion(static_cast<float>(event->x())*scale_x_, static_cast<float>(event->y())*scale_y_);
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
    event->accept();
    int delta = event->delta();
    osgGA::GUIEventAdapter::ScrollingMotion motion = delta > 0 ?
                osgGA::GUIEventAdapter::SCROLL_UP : osgGA::GUIEventAdapter::SCROLL_DOWN;
    this->getEventQueue()->mouseScroll(motion);

}

bool OSGViewDataWidget::event(QEvent* event)
{
    bool handled = QOpenGLWidget::event(event);

    switch (event->type())
    {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
    case QEvent::Wheel:
        update();
        break;

    default:
        break;
    }

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
    dbo_sizes_.clear();

//    Vec3d eye(47.5, 14.0, 10.0 );
//    Vec3d center(47.5, 14.0, 0.0 );
//    Vec3d up( 0.0, 1.0, 0.0 );
//    assert (manipulator_);
//    manipulator_->setHomePosition(eye, center, up);
//    viewer_->setCameraManipulator(manipulator_);

}

void OSGViewDataWidget::setup ()
{
    assert (!root_node_);
    root_node_ = new osg::Group();

//    osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;
//    // set up alpha blending
//    stateset->setMode(GL_BLEND, osg::StateAttribute::ON);

//    osg::ref_ptr<osg::BlendFunc> blendFunction = new osg::BlendFunc;
//    blendFunction->setFunction(osg::BlendFunc::SRC_ALPHA,
//                               osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
//    stateset->setAttributeAndModes(blendFunction, osg::StateAttribute::ON);

//    osg::ref_ptr<osg::BlendEquation> blendEquation = new osg::BlendEquation;

//    blendEquation->setEquation(osg::BlendEquation::FUNC_ADD);
//    stateset->setAttribute(blendEquation);

//    stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
//    stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
//    root_node_->setStateSet(stateset);


    //osg::Node* loadedModel = osgDB::readNodeFile("data/maps/openstreetmap_flat.earth");
    osg::Node* loadedModel = osgDB::readNodeFile("data/maps/openstreetmap.earth");

//    osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;
//    osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
//    alphaFunc->setFunction(osg::AlphaFunc::GEQUAL,0.05f);
//    stateset->setAttributeAndModes (alphaFunc, osg::StateAttribute::ON );
//    loadedModel->setStateSet(stateset);

    //osg::Node* loadedModel = osgDB::readNodeFile("data/maps/lod_blending.earth");
    // Find the MapNode
    assert (!map_node_);
    map_node_ = MapNode::get( loadedModel );
    map_node_->getMap()->getImageLayerAt(0)->setOpacity(0.9);
    root_node_->addChild(map_node_);

    osg::Camera* camera = new osg::Camera;

    LogarithmicDepthBuffer logdepth;
    logdepth.install(camera);

    //camera->setViewport( 0.0, 0.0, this->width(), this->height() );
    camera->setClearColor( osg::Vec4( 0.f, 0.f, 0.f, 0.f ) );
    float aspectRatio = static_cast<float>( this->width()) / static_cast<float>( this->height() );
    //camera->setProjectionMatrixAsPerspective( 30.f, aspectRatio, 0.001f, 1000.f );
    camera->setProjectionMatrixAsPerspective(60.f, aspectRatio, 0.001f, 1000.f);

    camera->setGraphicsContext( graphics_window_ );

    //camera->setViewMatrixAsLookAt( eye, center, up );

    viewer_->setCamera(camera);

    viewer_->setSceneData(root_node_);
    manipulator_ = new osgGA::TrackballManipulator;
    manipulator_->setAllowThrow( false );
    this->setMouseTracking(true);
    viewer_->setCameraManipulator(manipulator_);
    viewer_->setThreadingModel(osgViewer::Viewer::SingleThreaded);
    viewer_->realize();

    setFocusPolicy(Qt::StrongFocus);
}

void OSGViewDataWidget::updateData (DBObject &object, std::shared_ptr<Buffer> buffer)
{
    loginf << "OSGViewDataWidget: updateData: dbo " << object.name() << " size " << buffer->size();

    assert (root_node_);
    osg::ref_ptr<osg::Geode> geode = createSpriteGeometry(object, buffer);
    //Registry::shaderGenerator().run(geode);

//    GeoTransform* xform = new GeoTransform();

//    const SpatialReference* srs = map_node_->getTerrain()->getSRS();
//    xform->setTerrain( map_node_->getTerrain() );
//    GeoPoint point(srs, -0.0, 0.0);
//    xform->setPosition(point);
//    xform->addChild(geode);
//    root_node_->addChild(xform);


    root_node_->addChild(geode);
    update();
}

osg::ref_ptr<osg::Geode> OSGViewDataWidget::createSpriteGeometry(DBObject &object, std::shared_ptr<Buffer> buffer)
{
    //  auto* sprite = dynamic_cast<const Sprite*>(modelGeometry.getPrimitive());
    //  if (!sprite)
    //  {
    //    std::cout << "Error: primitive with type sprite was not a sprite"
    //              << std::endl;
    //    return nullptr;
    //  }
    // per-instance data

//    osg::Geode* geode = new osg::Geode();
//    geode->setComputeBoundingSphereCallback( new MyComputeBoundCallback(RADIUS) );
//    geode->addDrawable( new osg::ShapeDrawable( new osg::Sphere(osg::Vec3f(0,0,0), RADIUS) ) );
//    osg::Vec3f center = geode->getBound().center();
//    GeoTransform* xform = new GeoTransform();
//    xform->setPosition( GeoPoint(srs, 0.0, 0.0, 0.0, ALTMODE_ABSOLUTE) );
//    xform->addChild( geode );
//return xform;

    //const SpatialReference* srs = map_node_->getTerrain()->getSRS();

    //GeoPoint point;

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    size_t previous_size = 0;
    if (dbo_sizes_.count(object.name()) > 0)
        previous_size = dbo_sizes_.at(object.name());

    size_t buffer_size = buffer->size();
    if (buffer_size <= previous_size)
    {
        logerr << "UGA bufer size fixme";
        return geode;
    }
    size_t size_to_read = buffer_size-previous_size;

    Sprite sprite (QColor("#FFFFFF"),Sprite::Style::CIRCLE, 2.0);

    osg::ref_ptr<osg::Vec3Array> instanceCoords = new osg::Vec3Array(size_to_read);

    ArrayListTemplate<double> &latitudes = buffer->getDouble ("POS_LAT_DEG");
    ArrayListTemplate<double> &longitudes = buffer->getDouble ("POS_LONG_DEG");

    //const SpatialReference* wgs84 = SpatialReference::get("wgs84");
    //const SpatialReference* srs = map_node_->getTerrain()->getSRS();

    //GeoPoint wgsPoint (srs, 0.0, 0.0, 0.0);
    //GeoPoint srsPoint;
    //osg::Vec3d world_point;


    double x,y,z;
    osg::EllipsoidModel elipsModelObj;


    for (size_t i = 0; i < size_to_read; ++i)
    {
        if (!latitudes.isNone(i) && !longitudes.isNone(i))
        {
//            point.set(srs, latitudes.get(previous_size+i), longitudes.get(previous_size+i), 2000.0, osgEarth::ALTMODE_ABSOLUTE);
//            xform_->setPosition(point);
//            assert (point.isValid());

            //world_point.set(latitudes.get(previous_size+i), longitudes.get(previous_size+i), 1.0);

            //wgsPoint.fromWorld(wgs84, world_point);
            //.set(srs, , osgEarth::ALTMODE_ABSOLUTE);
            //wgsPoint.toWorld(world_point);
            //srsPoint = wgsPoint.transform( srs );

            elipsModelObj.convertLatLongHeightToXYZ(latitudes.get(previous_size+i),longitudes.get(previous_size+i),1000.0,x,y,z);

            (*instanceCoords)[i].x() = x;
            (*instanceCoords)[i].y() = y;
            (*instanceCoords)[i].z() = z;

//            (*instanceCoords)[i].x() = latitudes.get(previous_size+i);
//            (*instanceCoords)[i].y() = longitudes.get(previous_size+i);
//            (*instanceCoords)[i].z() = 0;

        }
    }
    loginf << "last point x " << x << " y " << y << " z " << z;

    dbo_sizes_[object.name()] = buffer_size;

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(1);
    (*colors)[0].r() = 1.0f;
    (*colors)[0].g() = 0.0f;
    (*colors)[0].b() = 0.0f;
    (*colors)[0].a() = 1.0f;
    geom->setColorArray(colors, osg::Array::Binding::BIND_OVERALL);
    geom->setVertexArray(instanceCoords);
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, size_to_read));


      {
        osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;
        stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::ON);
        //stateset->setRenderBinDetails( 2, "RenderBin");

        // Use point sprites
        osg::ref_ptr<osg::PointSprite> pointSprite = new osg::PointSprite;
        stateset->setTextureAttributeAndModes(0, pointSprite, osg::StateAttribute::ON);

        osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
        alphaFunc->setFunction(osg::AlphaFunc::ALWAYS,0.05f);

//            osg::StateSet* billBoardStateSet = new osg::StateSet;

//            billBoardStateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
//            billBoardStateSet->setTextureAttributeAndModes
//                  (0, ocotilloTexture, osg::StateAttribute::ON );
//            billBoardStateSet->setAttributeAndModes
//                  (new osg::BlendFunc, osg::StateAttribute::ON );
//            osg::AlphaFunc* alphaFunction = new osg::AlphaFunc;
//            alphaFunction->setFunction(osg::AlphaFunc::GEQUAL,0.05f);
//            billBoardStateSet->setAttributeAndModes( alphaFunc, osg::StateAttribute::ON );

        // set GL_POINT_SIZE
        osg::ref_ptr<osg::Point> point = new osg::Point;
        point->setSize(sprite.getSize());
        stateset->setAttribute(point);

        // set texture for points
        auto* tex = textureFactory.getTextureForStyle(sprite.getStyle());
        if (!tex)
        {
          std::cout << "Error: got null texture" << std::endl;
          return nullptr;
        }
        stateset->setAttributeAndModes( alphaFunc, osg::StateAttribute::ON );
        stateset->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON);

        geom->setStateSet(stateset);
      }
    geode->addDrawable(geom);
    //geode->setName(getGeometryName(modelGeometry).toStdString());

    return geode;
}


