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

#include <memory>
#include <iostream>

#include <osgViewer/GraphicsWindow>
#include <osgViewer/Viewer>

#include "global.h"

class OSGViewDataSource;
class Buffer;
class DBObject;

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

  osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> graphics_window_;
  osg::ref_ptr<osgViewer::Viewer> viewer_;
  qreal scale_x_, scale_y_;

  void setup ();
  osg::Billboard* createBillboards ();
  osg::Drawable* createShrub(const float & scale, osg::StateSet* bbState);
};


#endif /* OSGVIEWDATAWIDGET_H_ */
