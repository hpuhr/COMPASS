/*
 * OSGViewDataWidget.h
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */

#ifndef OSGVIEWDATAWIDGET_H_
#define OSGVIEWDATAWIDGET_H_

#include <QWidget>
#include <QTimer>
#include <QOpenGLWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QColor>

#include <memory>
#include <iostream>

#include <osgViewer/GraphicsWindow>
#include <osgViewer/Viewer>

#include "texturefactory.h"
#include "global.h"
#include "logger.h"

class OSGView;
class OSGViewDataSource;
class Buffer;
class DBObject;

namespace osgEarth
{
class MapNode;
class GeoTransform;
}

namespace osgGA
{
class TrackballManipulator;
}

class LinePoint
{
public:
    osg::Vec3d point_;
    double tod_;
};

class LineContainer
{
public:
    LineContainer (const std::string& identifier):identifier_(identifier) { logdbg << "LineContainer: ctor: " << identifier_; }
    std::string identifier_;
    osg::ref_ptr<osg::Geode> geode_;
    std::vector <LinePoint> points_;
    unsigned int previous_size_{0};
};

class OSGViewDataWidget : public QOpenGLWidget
{
    Q_OBJECT
public slots:
    void loadingStartedSlot();
    /// @brief Called when new result Buffer was delivered
    void updateData (DBObject &object, std::shared_ptr<Buffer> buffer);

    void mapNameChangedSlot (const std::string& map_name);
    void mapOpacityChangedSlot (float opacity);
    void dataOpacityChangedSlot (float opacity);

    void useHeightChangedSlot (bool use);
    void useHeightScaleChangedSlot (bool use);
    void heightScaleFactorChangedSlot (float factor);
    void clampHeightChangedSlot (bool use);

public:
  OSGViewDataWidget(OSGView* osg_view, OSGViewDataSource* data_source, qreal scaleX, qreal scaleY, QWidget* parent = 0);

  virtual ~OSGViewDataWidget();

  void setScale(qreal X, qreal Y);

protected:

  virtual void paintGL();
  virtual void resizeGL( int width, int height );
  virtual void mouseMoveEvent(QMouseEvent* event);
  virtual void mousePressEvent(QMouseEvent* event);
  virtual void mouseReleaseEvent(QMouseEvent* event);
  virtual void wheelEvent(QWheelEvent* event);
  virtual bool event(QEvent* event);

private:
  OSGView* osg_view_;
  /// Data source
  OSGViewDataSource *data_source_;

  osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> graphics_window_;
  osg::ref_ptr<osgViewer::Viewer> viewer_;

  osg::Group* root_node_;
  osgEarth::MapNode* map_node_{nullptr};

  qreal scale_x_, scale_y_;

  std::map <std::string, size_t> dbo_sizes_;
  std::map <std::string, std::vector <osg::ref_ptr<osg::Geode>>> dbo_sprite_nodes_;
  std::map <std::string, std::vector <osg::ref_ptr<osg::Geode>>> dbo_line_nodes_;

  std::map <std::string, std::map <std::string, LineContainer*>> dbo_line_containers_;

  void setup ();
  TextureFactory textureFactory;
  std::map <std::string, QColor> object_colors_;

  osgGA::EventQueue* getEventQueue() const;

  void loadMapFile (const std::string earth_file);
  void createSpriteGeometry(DBObject &object, std::shared_ptr<Buffer> buffer);
  void createLineGeometry(DBObject &object, std::shared_ptr<Buffer> buffer);

  void deleteGeometry ();
  void redrawGeometry ();

};


#endif /* OSGVIEWDATAWIDGET_H_ */
