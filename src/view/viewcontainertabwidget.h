/*
 * ViewContainerTabWidget.h
 *
 *  Created on: Apr 11, 2013
 *      Author: sk
 */

#ifndef VIEWCONTAINERTABWIDGET_H_
#define VIEWCONTAINERTABWIDGET_H_

#include <QTabWidget>

class ViewContainerTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    ViewContainerTabWidget (QWidget *parent=0) : QTabWidget (parent) {}
    virtual ~ViewContainerTabWidget () {}

    QTabBar *getTabBar() { return tabBar(); }

};



#endif /* VIEWCONTAINERTABWIDGET_H_ */
