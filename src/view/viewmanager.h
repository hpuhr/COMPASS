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
class ViewContainer;
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
    //  void saveViewAsTemplate (View *view, std::string template_name);
    //void updateReadSet ();

    void deleteContainer (std::string instance_id);
    void removeContainer (std::string instance_id);

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    void viewShutdown( View* view, const std::string& err = "" );

    std::map <std::string, ViewContainer*> getContainers () {return containers_;}
    std::map <std::string, View *> getViews () {return views_;}
    DBOVariableSet getReadSet (const std::string &dbo_name);

    ViewManagerWidget *widget();

protected:
    ATSDB &atsdb_;

    ViewManagerWidget *widget_;
    bool initialized_;
    QTabWidget *tab_widget_;

    std::map <std::string, ViewContainer*> containers_;
    std::map<std::string, View*> views_;

    unsigned int container_count_;

    virtual void checkSubConfigurables ();
};

#endif /* VIEWMANAGER_H_ */
