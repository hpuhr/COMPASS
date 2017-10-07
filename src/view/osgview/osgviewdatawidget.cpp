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
#include "osgview.h"
#include "osgviewdatawidget.h"
#include "osgviewdatasource.h"
#include "buffer.h"
#include "logger.h"
#include "atsdb.h"

using namespace osg;
using namespace osgEarth;
using namespace osgEarth::Util;
using namespace osgEarth::Drivers;

OSGViewDataWidget::OSGViewDataWidget(OSGView* osg_view, OSGViewDataSource *data_source, qreal scaleX, qreal scaleY, QWidget* parent)
    : QOpenGLWidget(parent), osg_view_(osg_view), data_source_ (data_source), graphics_window_ (new osgViewer::GraphicsWindowEmbedded( this->x(), this->y(), this->width(), this->height())),
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
    deleteGeometry ();

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
    this->getEventQueue()->mouseScroll(motion, 5);
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

    loadMapFile (osg_view_->mapName());

    //osg::Node* loadedModel = osgDB::readNodeFile("data/maps/openstreetmap_flat.earth");
    //osg::Node* loadedModel = osgDB::readNodeFile("data/maps/openstreetmap.earth");


    // Find the MapNode


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
    deleteGeometry();

    update();
}

void OSGViewDataWidget::updateData (DBObject &object, std::shared_ptr<Buffer> buffer)
{
    logdbg << "OSGViewDataWidget: updateData: dbo " << object.name() << " size " << buffer->size() << " previous " << dbo_sizes_[object.name()];

    assert (root_node_);

    createSpriteGeometry(object, buffer);
    createLineGeometry(object, buffer);

    dbo_sizes_[object.name()] = buffer->size();

    update();
}

void OSGViewDataWidget::loadMapFile (const std::string earth_file)
{
    if (map_node_)
    {
        root_node_->removeChild(map_node_);
        map_node_ = nullptr;
    }

    osg::Node* loadedModel = osgDB::readNodeFile("data/maps/"+earth_file);

    assert (!map_node_);
    map_node_ = MapNode::get( loadedModel );
    mapOpacityChangedSlot (osg_view_->mapOpacity());
    root_node_->addChild(map_node_);
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

    const Terrain* terrain = map_node_->getTerrain();
    assert (terrain);

    bool use_height = osg_view_->useHeight();
    bool use_height_scale = osg_view_->useHeightScale();
    float height_scale_factor = osg_view_->heightScaleFactor();
    bool clamp_height = osg_view_->clampHeight();
    double terrain_height;

    for (size_t i = 0; i < size_to_read; ++i)
    {
        current_size = previous_size+i;
        assert (current_size <= buffer_size);

        mode_c = 0.0;

        if (!latitudes.isNone(current_size) && !longitudes.isNone(current_size))
        {
            latitude = latitudes.get(current_size);
            longitude = longitudes.get(current_size);

            if (use_height && !mode_c_height.isNone(current_size))
            {
                mode_c = 0.3048 * static_cast<float> (mode_c_height.get(current_size));

                if (use_height_scale)
                    mode_c *= height_scale_factor;
            }

            if (clamp_height)
            {
                ret = terrain->getHeight (srs,latitude, longitude, &terrain_height);
                assert (ret);
                terrain_height += 200;
                if (terrain_height > mode_c)
                    mode_c = terrain_height;
            }

            wgsPoint.set(wgs84, longitude, latitude, mode_c, osgEarth::ALTMODE_ABSOLUTE);
            srsPoint = wgsPoint.transform(srs);
            assert (srsPoint.isValid());

            ret = srsPoint.toWorld(world_point);
            assert (ret);

            (*instanceCoords)[i] = world_point;
        }
    }

    float opacity = 1.0-osg_view_->dataOpacity();

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(1);
    (*colors)[0].r() = sprite.getColor().redF();
    (*colors)[0].g() = sprite.getColor().greenF();
    (*colors)[0].b() = sprite.getColor().blueF();
    (*colors)[0].a() = opacity;
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

    logdbg << "OSGViewDataWidget: createLineGeometry: obj " << object.name() << " buf " << buffer_size << " prev " << previous_size << " to read " << size_to_read;

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

    std::map <std::string, LineContainer*>& line_container_groups = dbo_line_containers_[object.name()];
    logdbg << "OSGViewDataWidget: createLineGeometry: obj " << object.name() << " with " << line_container_groups.size() << " groups";

    int track_num;
    double latitude, longitude,mode_c;
    float tod;
    std::string group_id;
    LinePoint line_point;

    size_t current_size;
    bool ret;

    const Terrain* terrain = map_node_->getTerrain();
    assert (terrain);

    bool use_height = osg_view_->useHeight();
    bool use_height_scale = osg_view_->useHeightScale();
    float height_scale_factor = osg_view_->heightScaleFactor();
    bool clamp_height = osg_view_->clampHeight();
    double terrain_height;

    logdbg << "OSGViewDataWidget: createLineGeometry: obj " << object.name() << " creating points";

    for (size_t i = 0; i < size_to_read; ++i)
    {
        current_size = previous_size+i;
        assert (current_size < buffer_size);

        mode_c = 0.0;

        if (!latitudes.isNone(current_size) && !longitudes.isNone(current_size) && !track_nums.isNone(current_size))
        {
            track_num = track_nums.get(current_size);
            latitude = latitudes.get(current_size);
            longitude = longitudes.get(current_size);
            assert (!tods.isNone(current_size));
            tod = tods.get(current_size);

            if (use_height && !mode_c_height.isNone(current_size))
            {
                mode_c = 0.3048 * static_cast<float> (mode_c_height.get(current_size));

                if (use_height_scale)
                    mode_c *= height_scale_factor;
            }

            if (clamp_height)
            {
                ret = terrain->getHeight (srs,latitude, longitude, &terrain_height);
                terrain_height += 200;
                assert (ret);
                if (terrain_height > mode_c)
                    mode_c = terrain_height;
            }

            wgsPoint.set(wgs84, longitude, latitude, mode_c, osgEarth::ALTMODE_ABSOLUTE);
            srsPoint = wgsPoint.transform(srs);
            assert (srsPoint.isValid());

            ret = srsPoint.toWorld(world_point);
            assert (ret);

            group_id = std::to_string(track_num);

            logdbg << "OSGViewDataWidget: createLineGeometry: obj " << object.name() << " group " << group_id << " lat " << latitude << " lon " << longitude << " modec " << mode_c;

            if (line_container_groups.count(group_id) == 0)
            {
                logdbg << "OSGViewDataWidget: createLineGeometry: obj " << object.name() << " creating new container for " << group_id;
                line_container_groups.insert(std::pair<std::string,LineContainer*> (group_id, new LineContainer(group_id)) );
                line_container_groups.at(group_id)->geode_ = new osg::Geode;

                root_node_->addChild(line_container_groups.at(group_id)->geode_);
                dbo_line_nodes_[object.name()].push_back(line_container_groups.at(group_id)->geode_);
            }

            LineContainer* line_container = line_container_groups.at(group_id);

            //loginf << "OSGViewDataWidget: createLineGeometry: obj " << object.name() << " group " << group_id << " prev " << line_container->previous_size_ << " points "
            //       << line_container->points_.size();

            line_point.point_ = world_point;
            line_point.tod_ = tod;

            line_container->points_.push_back(line_point);
        }
    }

    logdbg << "OSGViewDataWidget: createLineGeometry: obj " << object.name() << " creating lines, line containers " << line_container_groups.size();

    float opacity = 1.0-osg_view_->dataOpacity();

    for (auto line_group_it : line_container_groups)
    {
        LineContainer* line_container = line_group_it.second;

        if (line_container->points_.size() != line_container->previous_size_) // draw me like 'em french girls
        {
            size_t begin_index;
            size_t num_lines;
            size_t max_index;

            assert (line_container->points_.size() > line_container->previous_size_);

            logdbg << "OSGViewDataWidget: createLineGeometry: obj " << object.name() << " line container " << line_container->identifier_ << " prev " << line_container->previous_size_
                   << " points " << line_container->points_.size();

            if (line_container->previous_size_ == 0)
            {
                begin_index = 0;
                num_lines = line_container->points_.size();
                max_index = line_container->points_.size();
            }
            else
            {
                begin_index = line_container->previous_size_-1;
                num_lines = line_container->points_.size() - line_container->previous_size_ + 1; // line to last one
                max_index = line_container->points_.size();
            }

            logdbg << "OSGViewDataWidget: createLineGeometry: obj " << object.name() << " line container " << line_container->identifier_ << " with num_lines " << num_lines;

            // per-instance data
            osg::ref_ptr<osg::Vec3Array> instanceCoords = new osg::Vec3Array(num_lines);

            unsigned int line_cnt=0;
            for (size_t i = begin_index; i < max_index; ++i)
            {
                (*instanceCoords)[line_cnt] = line_container->points_.at(i).point_;

//                if (line_container->identifier_ == "67")
//                    loginf << "UGA line " << line_cnt << " i " << i << " point " << (*instanceCoords)[line_cnt].x() << "," << (*instanceCoords)[line_cnt].y()
//                           << "," << (*instanceCoords)[line_cnt].z();

                line_cnt++;
            }

            QColor color = object_colors_[object.name()];

            osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
            geom->setVertexArray(instanceCoords);
            osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(1);
            (*colors)[0].r() = color.redF();
            (*colors)[0].g() = color.greenF();
            (*colors)[0].b() = color.blueF();
            (*colors)[0].a() = opacity;
            geom->setColorArray(colors, osg::Array::Binding::BIND_OVERALL);
            geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, num_lines));

            {
                osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;
                stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::ON);
                stateset->setAttribute(new osg::LineWidth(1));
                geom->setStateSet(stateset);
            }
            line_container->geode_->addDrawable(geom);
            //geode->setName(getGeometryName(modelGeometry).toStdString());

            line_container->previous_size_=line_container->points_.size();
            logdbg << "OSGViewDataWidget: createLineGeometry: obj " << object.name() << " line container " << line_container->identifier_ << " end, set prev to " << line_container->previous_size_;
        }
        else
            logdbg << "OSGViewDataWidget: createLineGeometry: obj " << object.name() << " line container " << line_container->identifier_ << " requires no update";
    }

    return;
}

void OSGViewDataWidget::mapNameChangedSlot (const std::string& map_name)
{
    loadMapFile(map_name);

    redrawGeometry ();
}

void OSGViewDataWidget::mapOpacityChangedSlot (float opacity)
{
    assert (map_node_);
    map_node_->getMap()->getImageLayerAt(0)->setOpacity(1.0-opacity);
    update();
}

void OSGViewDataWidget::dataOpacityChangedSlot (float opacity)
{
    loginf << "OSGViewDataWidget: dataOpacityChangedSlot: " << opacity;

    redrawGeometry ();
    update();
}

void OSGViewDataWidget::useHeightChangedSlot (bool use)
{
    loginf << "OSGViewDataWidget: useHeightChangedSlot: " << use;

    redrawGeometry ();
    update();
}

void OSGViewDataWidget::useHeightScaleChangedSlot (bool use)
{
    loginf << "OSGViewDataWidget: useHeightScaleChangedSlot: " << use;

    redrawGeometry ();
    update();
}

void OSGViewDataWidget::heightScaleFactorChangedSlot (float factor)
{
    loginf << "OSGViewDataWidget: heightScaleFactorChangedSlot: " << factor;

    redrawGeometry ();
    update();
}

void OSGViewDataWidget::clampHeightChangedSlot (bool use)
{
    loginf << "OSGViewDataWidget: clampHeightChangedSlot: " << use;

    redrawGeometry ();
    update();
}

void OSGViewDataWidget::deleteGeometry ()
{
    dbo_sizes_.clear();

    for (auto it : dbo_line_containers_)
        for (auto it2 : it.second)
            delete it2.second;

    dbo_line_containers_.clear();

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
            //node_it->removeDrawables (0, node_it->getNumDrawables());
            root_node_->removeChild(node_it);
        }
        object_it.second.clear();
    }
    dbo_line_nodes_.clear();
}

void OSGViewDataWidget::redrawGeometry ()
{
    deleteGeometry ();

    for (auto object_it : ATSDB::instance().objectManager().objects())
    {
        if (object_it.second->data())
            updateData (*object_it.second, object_it.second->data());
    }
}
