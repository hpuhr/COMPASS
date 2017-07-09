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

#include <memory>
#include <iostream>

#include "global.h"

class OSGViewDataSource;
class Buffer;
class DBObject;


#include <osgGA/TrackballManipulator>

#include <osgDB/ReadFile>

#include <osgQt/GraphicsWindowQt>

#include <iostream>

#include <QOpenGLWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDesktopWidget>
#include <QScreen>
#include <QtGlobal>
#include <QWindow>

#include <osg/ref_ptr>
#include <osgViewer/GraphicsWindow>
#include <osgViewer/Viewer>
#include <osg/Camera>
#include <osg/ShapeDrawable>
#include <osg/StateSet>
#include <osg/Material>
#include <osgGA/EventQueue>
#include <osgGA/TrackballManipulator>
//#include <osgEarthQt/ViewerWidget>

#include <osgEarth/Map>
#include <osgEarth/MapNode>
#include <osgEarthDrivers/tms/TMSOptions>
#include <osgEarthDrivers/gdal/GDALOptions>

using namespace osgEarth;
using namespace osgEarth::Drivers;


#include <iostream>
#include <stdio.h>

class OSGViewDataSource;

class OSGViewDataWidget : public QOpenGLWidget
{
    Q_OBJECT
public slots:
    void loadingStartedSlot();
    /// @brief Called when new result Buffer was delivered
    void updateData (DBObject &object, std::shared_ptr<Buffer> buffer);

public:
  OSGViewDataWidget(OSGViewDataSource *data_source, qreal scaleX, qreal scaleY, QWidget* parent = 0);

  virtual ~OSGViewDataWidget();

  void setScale(qreal X, qreal Y);

protected:

  virtual void paintGL();

  virtual void resizeGL( int width, int height );

//  virtual void initializeGL(){}

  virtual void mouseMoveEvent(QMouseEvent* event);

  virtual void mousePressEvent(QMouseEvent* event);

  virtual void mouseReleaseEvent(QMouseEvent* event);

  virtual void wheelEvent(QWheelEvent* event);

  virtual bool event(QEvent* event);

private:
  /// Data source
  OSGViewDataSource *data_source_;

  osgGA::EventQueue* getEventQueue() const;

  osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> _mGraphicsWindow;
  osg::ref_ptr<osgViewer::Viewer> _mViewer;
  qreal m_scaleX, m_scaleY;
};

/**
 * @brief Widget with tab containing BufferTableWidgets in OSGView
 *
 */
//class OSGViewDataWidget : public QWidget, public osgViewer::CompositeViewer
//{
//    Q_OBJECT
//public slots:
//    void loadingStartedSlot();
//    /// @brief Called when new result Buffer was delivered
//    void updateData (DBObject &object, std::shared_ptr<Buffer> buffer);

//public:
//    /// @brief Constructor
//    OSGViewDataWidget(OSGViewDataSource *data_source, osgViewer::ViewerBase::ThreadingModel threadingModel=osgViewer::CompositeViewer::SingleThreaded,
//                      QWidget* parent=nullptr, Qt::WindowFlags f=0);
//    /// @brief Destructor
//    virtual ~OSGViewDataWidget();

//    QWidget *addViewWidget (osgQt::GraphicsWindowQt *gw, osg::Node *scene);
//    osgQt::GraphicsWindowQt* createGraphicsWindow (int x, int y, int w, int h, const std::string &name="", bool windowDecoration=false);

//    void paintEvent (QPaintEvent* event);

//protected:
//    /// Data source
//    OSGViewDataSource *data_source_;

//    QTimer timer_;
//};

#endif /* OSGVIEWDATAWIDGET_H_ */
