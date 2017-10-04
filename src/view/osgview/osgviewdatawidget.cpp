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

    camera->setViewport( 0.0, 0.0, this->width(), this->height() );
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
    dbo_line_nodes_.clear();

    assert (root_node_);

    for (auto object_it : dbo_sprite_nodes_)
    {
        for (auto node_it : object_it.second)
        {
            root_node_->removeChild(node_it);
        }
        object_it.second.clear();
    }

    for (auto object_it : dbo_line_nodes_)
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
    logdbg << "OSGViewDataWidget: updateData: dbo " << object.name() << " size " << buffer->size();

    assert (root_node_);

    createSpriteGeometry(object, buffer);
    createLineGeometry(object, buffer);

    dbo_sizes_[object.name()] = buffer->size();

    update();
}

void OSGViewDataWidget::createSpriteGeometry(DBObject &object, std::shared_ptr<Buffer> buffer)
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    size_t previous_size = 0;
    if (dbo_sizes_.count(object.name()) > 0)
        previous_size = dbo_sizes_.at(object.name());

    size_t buffer_size = buffer->size();
    if (buffer_size <= previous_size)
    {
        logerr << "OSGViewDataWidget: createSpriteGeometry: obj " << object.name() << " buf " << buffer_size << " prev " << previous_size;
        return;
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
        return;
    }

    if (!object_manager.metaVariable("pos_lat_deg").existsIn(object.name()) ||
            !object_manager.metaVariable("pos_long_deg").existsIn(object.name()) ||
            !object_manager.metaVariable("modec_code_ft").existsIn(object.name()))
    {
        logwrn << "OSGViewDataWidget::createSpriteGeometry: required variables exist but not for object, quitting";
        return;
    }
    const DBOVariable &latitude_var = object_manager.metaVariable("pos_lat_deg").getFor(object.name());
    const DBOVariable &longitude_var = object_manager.metaVariable("pos_long_deg").getFor(object.name());
    const DBOVariable &altitude_var = object_manager.metaVariable("modec_code_ft").getFor(object.name());

    ArrayListTemplate<double> &latitudes = buffer->getDouble (latitude_var.name());
    ArrayListTemplate<double> &longitudes = buffer->getDouble (longitude_var.name());
    ArrayListTemplate<int> &mode_c_height = buffer->getInt (altitude_var.name());

    const SpatialReference* wgs84 = SpatialReference::get("wgs84");
    const SpatialReference* srs = map_node_->getTerrain()->getSRS();

    GeoPoint wgsPoint;
    GeoPoint srsPoint;
    osg::Vec3d world_point;

    double latitude, longitude,mode_c;
    size_t current_size;
    bool ret;

    for (size_t i = 0; i < size_to_read; ++i)
    {
        current_size = previous_size+i;
        if (current_size > buffer_size)
        {
            logwrn << "UGA read size";
            break;
        }

        mode_c = 1000.0;

        if (!latitudes.isNone(current_size) && !longitudes.isNone(current_size))
        {
            latitude = latitudes.get(current_size);
            longitude = longitudes.get(current_size);

            if (!mode_c_height.isNone(current_size))
                mode_c = 5.0*0.3048 * static_cast<float> (mode_c_height.get(current_size));

            wgsPoint.set(wgs84, longitude, latitude, mode_c, osgEarth::ALTMODE_ABSOLUTE);
            srsPoint = wgsPoint.transform(srs);
            assert (srsPoint.isValid());

            ret = srsPoint.toWorld(world_point);
            assert (ret);

            (*instanceCoords)[i] = world_point;
        }
    }

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
            return;
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
        point->setSize(3); //sprite.getSize()
        stateset->setAttribute(point);

        stateset->setAttributeAndModes( alphaFunc, osg::StateAttribute::ON );
        stateset->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON);

        geom->setStateSet(stateset);
    }
    geode->addDrawable(geom);
    //geode->setName(getGeometryName(modelGeometry).toStdString());

    root_node_->addChild(geode);
    dbo_sprite_nodes_[object.name()].push_back(geode);
}

void OSGViewDataWidget::createLineGeometry(DBObject &object, std::shared_ptr<Buffer> buffer)
{
    assert (buffer);

    //osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    DBObjectManager &object_manager = ATSDB::instance().objectManager();

    if (!object_manager.existsMetaVariable("track_num") || !object_manager.existsMetaVariable("pos_lat_deg") ||
            !object_manager.existsMetaVariable("pos_long_deg") || !object_manager.existsMetaVariable("modec_code_ft") || !object_manager.existsMetaVariable("tod"))
    {
        logwrn << "OSGViewDataWidget::createLineGeometry: required variables missing, quitting";
        return;
    }

    if (!object_manager.metaVariable("track_num").existsIn(object.name()) || !object_manager.metaVariable("pos_lat_deg").existsIn(object.name()) ||
            !object_manager.metaVariable("pos_long_deg").existsIn(object.name()) || !object_manager.metaVariable("modec_code_ft").existsIn(object.name()) ||
            !object_manager.metaVariable("tod").existsIn(object.name()))
    {
        logwrn << "OSGViewDataWidget::createLineGeometry: required variables exist but not for object, quitting";
        return;
    }

    size_t previous_size = 0;
    if (dbo_sizes_.count(object.name()) > 0)
        previous_size = dbo_sizes_.at(object.name());

    size_t buffer_size = buffer->size();
    if (buffer_size <= previous_size)
    {
        logerr << "OSGViewDataWidget: createLineGeometry: obj " << object.name() << " buf " << buffer_size << " prev " << previous_size;
        return;
    }

    size_t size_to_read = buffer_size-previous_size;

    const DBOVariable &track_num_var = object_manager.metaVariable("track_num").getFor(object.name());
    const DBOVariable &latitude_var = object_manager.metaVariable("pos_lat_deg").getFor(object.name());
    const DBOVariable &longitude_var = object_manager.metaVariable("pos_long_deg").getFor(object.name());
    const DBOVariable &altitude_var = object_manager.metaVariable("modec_code_ft").getFor(object.name());
    const DBOVariable &tod_var = object_manager.metaVariable("tod").getFor(object.name());

    ArrayListTemplate<int> &track_nums = buffer->getInt (track_num_var.name());
    ArrayListTemplate<double> &latitudes = buffer->getDouble (latitude_var.name());
    ArrayListTemplate<double> &longitudes = buffer->getDouble (longitude_var.name());
    ArrayListTemplate<int> &mode_c_height = buffer->getInt (altitude_var.name());
    ArrayListTemplate<float> &tods = buffer->getFloat (tod_var.name());

    const SpatialReference* wgs84 = SpatialReference::get("wgs84");
    const SpatialReference* srs = map_node_->getTerrain()->getSRS();

    GeoPoint wgsPoint;
    GeoPoint srsPoint;
    osg::Vec3d world_point;

    std::map <std::string, LineContainer>& line_container_groups = dbo_line_containers_[object.name()];

    bool has_track_num;
    int track_num;
    double latitude, longitude,mode_c;
    float tod;
    std::string group_id;
    LinePoint line_point;

    size_t current_size;
    bool ret;

    for (size_t i = 0; i < size_to_read; ++i)
    {
        current_size = previous_size+i;
        assert (current_size < buffer_size);

        mode_c = 1000.0;

        if (!latitudes.isNone(current_size) && !longitudes.isNone(current_size))
        {
            has_track_num = !track_nums.isNone(current_size);

            if (has_track_num)
                track_num = track_nums.get(current_size);
            else
                continue;

            latitude = latitudes.get(current_size);
            longitude = longitudes.get(current_size);
            assert (!tods.isNone(current_size));
            tod = tods.get(current_size);

            if (!mode_c_height.isNone(current_size))
                mode_c = 5.0*0.3048 * static_cast<float> (mode_c_height.get(current_size));

            wgsPoint.set(wgs84, longitude, latitude, mode_c, osgEarth::ALTMODE_ABSOLUTE);
            srsPoint = wgsPoint.transform(srs);
            assert (srsPoint.isValid());

            ret = srsPoint.toWorld(world_point);
            assert (ret);

            group_id = std::to_string(track_num);

            LineContainer &line_container = line_container_groups[group_id];

            if (!line_container.geode_)
            {
                line_container.identifier_ = group_id;
                line_container.geode_ = new osg::Geode;

                root_node_->addChild(line_container.geode_);
                dbo_line_nodes_[object.name()].push_back(line_container.geode_);
            }

//            std::string identifier_;
//            osg::ref_ptr<osg::Geode> geode_;
//            std::vector <LinePoint> points_;
//            unsigned int previous_size_{0};

//            line point
//            osg::Vec3d point_;
//            double tod_;

            line_point.point_ = world_point;
            line_point.tod_ = tod;

            line_container.points_.push_back(line_point);
        }
    }

    for (auto line_group_it : line_container_groups)
    {
        LineContainer &line_container = line_group_it.second;

        if (line_container.points_.size() != line_container.previous_size_) // draw me like 'em french girls
        {
            unsigned int num_lines;

            assert (line_container.points_.size() > line_container.previous_size_);

            if (line_container.previous_size_ == 0)
                num_lines = line_container.points_.size();
            else
                num_lines = line_container.points_.size() - line_container.previous_size_ + 1; // line to last one

            // per-instance data
            osg::ref_ptr<osg::Vec3Array> instanceCoords = new osg::Vec3Array(num_lines);

//            size_t begin, end;

//            if (line_group.previous_size_ == 0)
//            {
//                begin = 0;
//                end = num_lines-1;
//            }
//            else // line to last one
//            {
//                begin = line_group.previous_size_;
//                end = num_lines-1;
//                (*instanceCoords)[0] =
//            }

            for (size_t i = line_container.previous_size_; i < num_lines; ++i)
            {
                (*instanceCoords)[i] = line_container.points_.at(i).point_;
            }

            QColor color = object_colors_[object.name()];

            osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
            geom->setVertexArray(instanceCoords);
            osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(1);
            (*colors)[0].r() = color.redF();
            (*colors)[0].g() = color.greenF();
            (*colors)[0].b() = color.blueF();
            (*colors)[0].a() = 0.9f;
            geom->setColorArray(colors, osg::Array::Binding::BIND_OVERALL);
            geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, num_lines));

            {
                osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;
                stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::ON);
                stateset->setAttribute(new osg::LineWidth(1));
                geom->setStateSet(stateset);
            }
            line_container.geode_->addDrawable(geom);
            //geode->setName(getGeometryName(modelGeometry).toStdString());
            line_container.previous_size_=line_container.points_.size();
        }
    }

    // per-instance data
//    osg::ref_ptr<osg::Vec2Array> instanceCoords =
//            new osg::Vec2Array(modelGeometry.getPositions().size());
//    for (size_t i = 0; i < modelGeometry.getPositions().size(); ++i)
//    {
//        (*instanceCoords)[i].x() = modelGeometry.getPositions()[i].x();
//        (*instanceCoords)[i].y() = modelGeometry.getPositions()[i].y();
//    }

//    QColor color = object_colors_[object.name()];

//    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
//    geom->setVertexArray(instanceCoords);
//    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(1);
//    (*colors)[0].r() = color.redF();
//    (*colors)[0].g() = color.greenF();
//    (*colors)[0].b() = color.blueF();
//    (*colors)[0].a() = 0.9f;
//    geom->setColorArray(colors, osg::Array::Binding::BIND_OVERALL);
//    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, modelGeometry.getPositions().size()));

//    {
//        osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;
//        stateset->setAttribute(new osg::LineWidth(2));
//        geom->setStateSet(stateset);
//    }
//    geode->addDrawable(geom);
//    //geode->setName(getGeometryName(modelGeometry).toStdString());

    return;
}
