/*
 * ViewManager.h
 *
 *  Created on: Mar 26, 2012
 *      Author: sk
 */

#ifndef VIEWMANAGER_H_
#define VIEWMANAGER_H_

#include <QObject>

#include "dbovariableset.h"
#include "configurable.h"

class ATSDB;
class Buffer;
class ViewContainerWidget;
class ViewManagerWidget;
class View;
class QWidget;
class QTabWidget;

class ViewManager : public QObject, public Configurable
{
    Q_OBJECT
public:
    ViewManager(const std::string &class_id, const std::string &instance_id, ATSDB *atsdb);
    virtual ~ViewManager();

    void init (QTabWidget *tab_widget);
    void close ();

    void registerView (View *view);
    void unregisterView (View *view);
    bool isRegistered (View *view);

    //  void distributeData (Buffer *buffer);
    //  void clearData ();

    //  void addContainerWithGeographicView ();
    //  void addContainerWithHistogramView ();
    //  void addContainerWithListBoxView ();
    //  void addContainerWithMosaicView ();
    //  void addContainerWithScatterPlotView ();
    //  void addContainerWithTemplateView (std::string template_name);

      void deleteContainer (std::string instance_id);
      void removeContainer (std::string instance_id);

    //void updateReadSet ();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    void viewShutdown( View* view, const std::string& err = "" );
    //  void saveViewAsTemplate (View *view, std::string template_name);

    //  void setViewsWidget (ViewsWidget *views_widget);

    std::map <std::string, ViewContainerWidget *> getContainers () {return containers_;}
    std::map <std::string, View *> getViews () {return views_;}
    DBOVariableSet &getReadSet () { return read_set_; }

    ViewManagerWidget *widget();
    //  QWidget *getCentralWidget () { assert(central_widget_); return central_widget_; }

protected:
    ATSDB &atsdb_;

    ViewManagerWidget *widget_;
    bool initialized_;
    QTabWidget *tab_widget_;

    std::map <std::string, ViewContainerWidget *> containers_;
    std::map<std::string, View*> views_;
    //  ViewsWidget *views_widget_;

    DBOVariableSet read_set_;

    unsigned int container_count_;

    virtual void checkSubConfigurables ();
};

#endif /* VIEWMANAGER_H_ */
