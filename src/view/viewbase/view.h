/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VIEW_H
#define VIEW_H

#include "configurable.h"
#include "dbcontent/variable/variableset.h"
#include "viewcontainerwidget.h"
#include "buffer.h"
#include "appmode.h"

#include <QObject>

#include <memory>
#include <map>

class ViewContainer;
class ViewWidget;
class QQWidget;
class Workflow;

class ViewableDataConfig;
class LatexVisitor;

/**
@brief Serves as base class for all views. Subclasses can be embedded in a ViewContainerWidget.

A view consists of two main components, a widget to display its content (not specifically a qt
widget) and a model manage its data. To add a new view to palantir, one will typically have to
introduce three new classes: a view class, a view widget class and a view model class. These classes
should be named similarly, e.g. *View, *ViewModel, *ViewWidget. Often these classes have to be
derived from a similar group of coherent classes. For example a new view based on a DBView will need
a model based on a DBViewModel and a widget based on a DBViewWidget.

A view always has a center widget, in which the ViewWidget is embedded. The central widget will then
again always be embedded inside a ViewContainerWidget, which shows the specific views in tabs.
 */
class View : public QObject, public Configurable
{
    Q_OBJECT
public:
    View(const std::string& class_id, const std::string& instance_id, ViewContainer* container,
         ViewManager& view_manager);
    virtual ~View();

    bool init();

    virtual void databaseOpened();
    virtual void databaseClosed();

    virtual void loadingStarted();
    virtual void loadedData(const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset);
    virtual void loadingDone();
    virtual void clearData();
    virtual void appModeSwitch(AppMode app_mode_previous, AppMode app_mode_current);

    unsigned int getKey();
    const std::string& getName() const;

    /// @brief Returns the view's central widget
    QWidget* getCentralWidget() { return central_widget_; }

    /// @brief Returns the view's widget, override this method in derived classes.
    ViewWidget* getWidget() { assert (widget_); return widget_; }

    void enableInTabWidget(bool value);
    void showInTabWidget();

    virtual dbContent::VariableSet getSet(const std::string& dbcontent_name) = 0;

    void viewShutdown(const std::string& err);
    void emitSelectionChange();

    AppMode appMode() const { return app_mode_; }

    virtual void accept(LatexVisitor& v) = 0;

signals:
    void selectionChangedSignal();  // do not emit manually, call emitSelectionChange()

public slots:
    void selectionChangedSlot();
    virtual void unshowViewPointSlot (const ViewableDataConfig* vp)=0;
    virtual void showViewPointSlot (const ViewableDataConfig* vp)=0;

protected:
    virtual void updateSelection() = 0;
    virtual bool init_impl() { return true; }

    void constructWidget();
    //void setModel(ViewModel* model);
    void setWidget(ViewWidget* widget);

    ViewManager& view_manager_;

    /// The view's model
    //ViewModel* model_;
    /// The view's widget
    ViewWidget* widget_ {nullptr};
    /// The ViewContainerWidget the view is currently embedded in
    ViewContainer* container_ {nullptr};
    /// The widget containing the view's widget
    QWidget* central_widget_;

private:
    unsigned int getInstanceKey();

    AppMode app_mode_;

    /// Static member counter
    static unsigned int cnt_;
};

#endif  // VIEW_H
