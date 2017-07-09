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

#include <osgViewer/CompositeViewer>
#include <osgQt/GraphicsWindowQt>

#include "global.h"

class OSGViewDataSource;
class Buffer;
class DBObject;


class ViewerWidget : public QWidget
{
public:

protected:


};

/**
 * @brief Widget with tab containing BufferTableWidgets in OSGView
 *
 */
class OSGViewDataWidget : public QWidget, public osgViewer::CompositeViewer
{
    Q_OBJECT
public slots:
    void loadingStartedSlot();
    /// @brief Called when new result Buffer was delivered
    void updateData (DBObject &object, std::shared_ptr<Buffer> buffer);

public:
    /// @brief Constructor
    OSGViewDataWidget(OSGViewDataSource *data_source, osgViewer::ViewerBase::ThreadingModel threadingModel=osgViewer::CompositeViewer::SingleThreaded,
                      QWidget* parent=nullptr, Qt::WindowFlags f=0);
    /// @brief Destructor
    virtual ~OSGViewDataWidget();

    QWidget *addViewWidget (osgQt::GraphicsWindowQt *gw, osg::Node *scene);
    osgQt::GraphicsWindowQt* createGraphicsWindow (int x, int y, int w, int h, const std::string &name="", bool windowDecoration=false);

    void paintEvent (QPaintEvent* event);

protected:
    /// Data source
    OSGViewDataSource *data_source_;

    QTimer timer_;
};

#endif /* OSGVIEWDATAWIDGET_H_ */
