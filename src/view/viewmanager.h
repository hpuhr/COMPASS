/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VIEWMANAGER_H_
#define VIEWMANAGER_H_

#include <QObject>

#include "configurable.h"
#include "dbovariableset.h"

class ATSDB;
class Buffer;
class ViewContainer;
class ViewContainerWidget;
class ViewManagerWidget;
class View;
class ViewPoint;
class ViewPointsWidget;

class QWidget;
class QTabWidget;

class ViewManager : public QObject, public Configurable
{
    Q_OBJECT

  signals:
    void selectionChangedSignal();
    void unshowViewPointSignal (ViewPoint* vp);
    void showViewPointSignal (ViewPoint* vp);

  public slots:
    void selectionChangedSlot();

  public:
    ViewManager(const std::string& class_id, const std::string& instance_id, ATSDB* atsdb);
    virtual ~ViewManager();

    void init(QTabWidget* tab_widget);
    void close();

    void registerView(View* view);
    void unregisterView(View* view);
    bool isRegistered(View* view);

    ViewContainerWidget* addNewContainerWidget();

    // void deleteContainer (std::string instance_id);
    void removeContainer(std::string instance_id);
    void deleteContainerWidget(std::string instance_id);
    void removeContainerWidget(std::string instance_id);

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    void viewShutdown(View* view, const std::string& err = "");

    std::map<std::string, ViewContainer*> getContainers() { return containers_; }
    std::map<std::string, View*> getViews() { return views_; }
    DBOVariableSet getReadSet(const std::string& dbo_name);

    ViewManagerWidget* widget();

    unsigned int saveNewViewPoint(bool update=true);
    ViewPoint& saveNewViewPoint(unsigned int id, bool update=true);
    bool existsViewPoint(unsigned int id);
    ViewPoint& viewPoint(unsigned int id);
    void removeViewPoint(unsigned int id);
    void deleteAllViewPoints ();

    std::map<unsigned int, ViewPoint>& viewPoints() { return view_points_; }
    void printViewPoints();
    void saveViewPoints();

    ViewPointsWidget* viewPointsWidget() const;
    void importViewPoints (const std::string& filename);
    void exportViewPoints (const std::string& filename);

    void setCurrentViewPoint (unsigned int id);
    void unsetCurrentViewPoint ();

protected:
    ATSDB& atsdb_;

    ViewManagerWidget* widget_{nullptr};
    ViewPointsWidget* view_points_widget_{nullptr};

    bool initialized_{false};

    QTabWidget* main_tab_widget_{nullptr};

    std::map<std::string, ViewContainer*> containers_;
    std::map<std::string, ViewContainerWidget*> container_widgets_;
    std::map<std::string, View*> views_;
    std::map<unsigned int, ViewPoint> view_points_;

    bool current_view_point_set_ {false};
    unsigned int current_view_point_ {0};

    unsigned int container_count_{0};

    virtual void checkSubConfigurables();
};

#endif /* VIEWMANAGER_H_ */
