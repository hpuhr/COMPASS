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

#ifndef VIEW_H
#define VIEW_H

#include <QObject>

#include "configurable.h"
#include "dbovariableset.h"
#include "viewcontainerwidget.h"

class ViewContainer;
class ViewModel;
class ViewWidget;
class QQWidget;
class Workflow;

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

  signals:
    /// @brief Signals that loading has started in the view
    void loadingStarted();
    /// @brief Signals that loading has finished in the view
    void loadingFinished();
    /// @brief Signals the current loading time
    void loadingTime(double s);

    void selectionChangedSignal();  // do not emit manually, call emitSelectionChange()

  public slots:
    void selectionChangedSlot();

  public:
    View(const std::string& class_id, const std::string& instance_id, ViewContainer* container,
         ViewManager& view_manager);
    virtual ~View();

    /// @brief Starts immediate (e.g. through generators) or deferred (e.g. buffer driven) redraw
    virtual void update(bool atOnce = false) = 0;
    /// @brief Clears the views data
    virtual void clearData() = 0;
    virtual bool init();
    /// @brief Returns the view type as a string
    virtual std::string viewType() const { return "View"; }

    unsigned int getKey();
    const std::string& getName() const;

    /// @brief Returns the view's central widget
    QWidget* getCentralWidget() { return central_widget_; }

    /// @brief Returns the view's widget, override this method in derived classes.
    ViewWidget* getWidget() { return widget_; }
    /// @brief Returns the view's model, override this method in derived classes.
    ViewModel* getModel() { return model_; }

    virtual DBOVariableSet getSet(const std::string& dbo_name) = 0;

    void viewShutdown(const std::string& err);
    void emitSelectionChange();

  protected:
    ViewManager& view_manager_;

    /// The view's model
    ViewModel* model_;
    /// The view's widget
    ViewWidget* widget_;
    /// The ViewContainerWidget the view is currently embedded in
    ViewContainer* container_;
    /// The widget containing the view's widget
    QWidget* central_widget_;

    // bool selection_change_emitted_ {false};

    void constructWidget();
    void setModel(ViewModel* model);
    void setWidget(ViewWidget* widget);

    virtual void updateSelection() = 0;

  private:
    unsigned int getInstanceKey();

    /// Static member counter
    static unsigned int cnt_;
};

#endif  // VIEW_H
