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
#include <osgEarthDrivers/cache_filesystem/FileSystemCache>
#include <osgEarthUtil/LogarithmicDepthBuffer>
#include <osgEarthUtil/EarthManipulator>
#include <osg/PositionAttitudeTransform>

#include "buffertablewidget.h"
#include "dbobject.h"
#include "dbovariable.h"
#include "metadbovariable.h"
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
    object_colors_["Radar"] = QColor("#50FF50");
    object_colors_["MLAT"] = QColor("#FF5050");
    object_colors_["ADSB"] = QColor("#5050FF");
    object_colors_["Tracker"] = QColor("#FFFFFF");

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
    this->getEventQueue()->mouseScroll(motion, 10);
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

void OSGViewDataWidget::setup ()
{
    assert (!root_node_);
    root_node_ = new osg::Group();

    //    osgEarth::Drivers::FileSystemCacheOptions cacheOptions;
    //    cacheOptions.path("/home/sk/workspace_cdt/atsb/osgearth_cache/";

    //    MapOptions mapOptions;
    //    mapOptions.cache() = cacheOptions;


    //    osgEarth::FileSystemCache *cache = new osgEarth::FileSystemCache ();
    //    osgEarth::Registry::instance()->setCache(cache);
    //    osgEarth::Registry::instance()->setDefaultCachePolicy(osgEarth::CachePolicy::USAGE_READ_WRITE);

    //osg::Node* loadedModel = osgDB::readNodeFile("data/maps/openstreetmap_flat.earth");
    //osg::Node* loadedModel = osgDB::readNodeFile("data/maps/openstreetmap.earth");
    osg::Node* loadedModel = osgDB::readNodeFile("data/maps/lod_blending.earth");

    // Find the MapNode
    assert (!map_node_);
    map_node_ = MapNode::get( loadedModel );
    root_node_->addChild(map_node_);

    osg::Camera* camera = new osg::Camera;

    LogarithmicDepthBuffer logdepth;
    logdepth.install(camera);

    //camera->setViewport( 0.0, 0.0, this->width(), this->height() );
    camera->setClearColor( osg::Vec4( 0.f, 0.f, 0.f, 0.f ) );
    float aspectRatio = static_cast<float>( this->width()) / static_cast<float>( this->height() );
    camera->setProjectionMatrixAsPerspective(60.f, aspectRatio, 0.0001f, 1000.f);

    camera->setGraphicsContext( graphics_window_ );

    viewer_->setCamera(camera);

    viewer_->setSceneData(root_node_);
//    osgGA::TrackballManipulator* manipulator = new osgGA::TrackballManipulator;
//    manipulator->setAllowThrow( false );
//    manipulator_>setWheelZoomFactor(-0.005);

    osgEarth::Util::EarthManipulator* manipulator = new osgEarth::Util::EarthManipulator();
    this->setMouseTracking(true);
    viewer_->setCameraManipulator(manipulator);
    viewer_->setThreadingModel(osgViewer::Viewer::SingleThreaded);
    viewer_->realize();

    setFocusPolicy(Qt::StrongFocus);
}

void OSGViewDataWidget::loadingStartedSlot()
{
    loginf << "OSGViewDataWidget: loadingStartedSlot";
    dbo_sizes_.clear();

    assert (root_node_);

    for (auto object_it : dbo_nodes_)
    {
        for (auto node_it : object_it.second)
        {
            root_node_->removeChild(node_it);
        }
        object_it.second.clear();
    }

    update();
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
    dbo_nodes_[object.name()].push_back(geode);

    update();
}

osg::ref_ptr<osg::Geode> OSGViewDataWidget::createSpriteGeometry(DBObject &object, std::shared_ptr<Buffer> buffer)
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    size_t previous_size = 0;
    if (dbo_sizes_.count(object.name()) > 0)
        previous_size = dbo_sizes_.at(object.name());

    size_t buffer_size = buffer->size();
    if (object.name() == "Radar" || buffer_size <= previous_size) //TODO FIXME PLZ
    {
        logerr << "UGA bufer size fixme";
        return geode;
    }
    size_t size_to_read = buffer_size-previous_size;

    QColor color = object_colors_[object.name()];

    Sprite sprite (color,Sprite::Style::CIRCLE, 2.0);

    osg::ref_ptr<osg::Vec3Array> instanceCoords = new osg::Vec3Array(size_to_read);

    DBObjectManager &object_manager = ATSDB::instance().objectManager();

    if (!object_manager.existsMetaVariable("pos_lat_deg") ||
            !object_manager.existsMetaVariable("pos_long_deg") ||
            !object_manager.existsMetaVariable("modec_code_ft"))
    {
        logwrn << "OSGViewDataWidget::createSpriteGeometry: required variables missing, quitting";
        return geode;
    }

    if (!object_manager.metaVariable("pos_lat_deg").existsIn(object.name()) ||
            !object_manager.metaVariable("pos_long_deg").existsIn(object.name()) ||
            !object_manager.metaVariable("modec_code_ft").existsIn(object.name()))
    {
        logwrn << "OSGViewDataWidget::createSpriteGeometry: required variables exist but not for object, quitting";
        return geode;
    }
    const DBOVariable &latitude_var = object_manager.metaVariable("pos_lat_deg").getFor(object.name());
    const DBOVariable &longitude_var = object_manager.metaVariable("pos_long_deg").getFor(object.name());
    const DBOVariable &altitude_var = object_manager.metaVariable("modec_code_ft").getFor(object.name());

    ArrayListTemplate<double> &latitudes = buffer->getDouble (latitude_var.name());
    ArrayListTemplate<double> &longitudes = buffer->getDouble (longitude_var.name());
    ArrayListTemplate<int> &mode_c_height = buffer->getInt (altitude_var.name());

    //    const SpatialReference* wgs84 = SpatialReference::get("wgs84");
    //    const SpatialReference* srs = map_node_->getTerrain()->getSRS();

    //    GeoPoint wgsPoint (srs, 0.0, 0.0, 0.0);
    //    GeoPoint srsPoint;
    //    osg::Vec3d world_point;


    double x,y,z;
    osg::EllipsoidModel elipsModelObj;
    float mode_c;


    for (size_t i = 0; i < size_to_read; ++i)
    {
        if (previous_size+i > buffer_size)
        {
            logwrn << "UGA read size";
            break;
        }

        mode_c = 1000.0;

        if (!latitudes.isNone(i) && !longitudes.isNone(i))
        {
            //            point.set(srs, latitudes.get(previous_size+i), longitudes.get(previous_size+i), 2000.0, osgEarth::ALTMODE_ABSOLUTE);
            //            xform_->setPosition(point);
            //            assert (point.isValid());

            //            world_point.set(latitudes.get(previous_size+i), longitudes.get(previous_size+i), 1.0);

            //            wgsPoint.fromWorld(wgs84, world_point);

            //            x = wgsPoint.x();
            //            y = wgsPoint.y();
            //            z = wgsPoint.z();

            if (!mode_c_height.isNone(i))
                mode_c = 5.0*0.3048 * static_cast<float> (mode_c_height.get(i));

            elipsModelObj.convertLatLongHeightToXYZ(osg::DegreesToRadians(latitudes.get(previous_size+i)),osg::DegreesToRadians(longitudes.get(previous_size+i)),mode_c,x,y,z);

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
    (*colors)[0].r() = sprite.getColor().redF();
    (*colors)[0].g() = sprite.getColor().greenF();
    (*colors)[0].b() = sprite.getColor().blueF();
    (*colors)[0].a() = 0.9f;
    geom->setColorArray(colors, osg::Array::Binding::BIND_OVERALL);
    geom->setVertexArray(instanceCoords);
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, size_to_read));


    {
        // set texture for points
        auto* tex = textureFactory.getTextureForStyle(sprite.getStyle());
        if (!tex)
        {
            std::cout << "Error: got null texture" << std::endl;
            return nullptr;
        }

        osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;
        stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::ON);

        // Use point sprites
        osg::ref_ptr<osg::PointSprite> pointSprite = new osg::PointSprite;
        stateset->setTextureAttributeAndModes(0, pointSprite, osg::StateAttribute::ON);

        osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
        alphaFunc->setFunction(osg::AlphaFunc::ALWAYS,0.05f);

        // set GL_POINT_SIZE
        osg::ref_ptr<osg::Point> point = new osg::Point;
        point->setSize(sprite.getSize());
        stateset->setAttribute(point);

        stateset->setAttributeAndModes( alphaFunc, osg::StateAttribute::ON );
        stateset->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON);

        geom->setStateSet(stateset);
    }
    geode->addDrawable(geom);

    return geode;
}


