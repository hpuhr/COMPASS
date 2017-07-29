/*
 * ViewContainerWidget.h
 *
 *  Created on: Jan 15, 2012
 *      Author: sk
 */

#ifndef VIEWCONTAINERWIDGET_H_
#define VIEWCONTAINERWIDGET_H_

#include <QWidget>

#include "configurable.h"

class QTabWidget;
class ViewManager;
class ViewContainer;

/**
 * Serves as container for different views. May be embedded in a parent widget, or in new window, which is decided by constructor.
 *
 * Includes DBConfigTab, therefore DB HAS to be loaded before a constructor is called.
 */
class ViewContainerWidget : public QWidget, public Configurable
{
public:
    ViewContainerWidget(const std::string &class_id, const std::string &instance_id, ViewManager *parent);
    virtual ~ViewContainerWidget();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    const std::string &name () { return name_; }
    ViewContainer &viewContainer() const;

protected:
    ViewManager &view_manager_;
    std::string name_;

    unsigned int pos_x_;
    unsigned int pos_y_;
    unsigned int width_;
    unsigned int height_;
    unsigned int min_width_;
    unsigned int min_height_;

    QTabWidget *tab_widget_{nullptr};
    ViewContainer *view_container_{nullptr};

    void closeEvent ( QCloseEvent * event );
    virtual void moveEvent (QMoveEvent *event);
    virtual void resizeEvent (QResizeEvent *event);

    virtual void checkSubConfigurables ();
};

#endif /* VIEWCONTAINERWIDGET_H_ */
