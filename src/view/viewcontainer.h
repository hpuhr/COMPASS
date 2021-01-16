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

#ifndef VIEWCONTAINER_H
#define VIEWCONTAINER_H

#include <QMenu>
#include <QObject>

#include "configurable.h"

class MainLoadWidget;
class View;
class ViewManager;
class QHBoxLayout;
class QPushButton;
class QTabWidget;

class ViewManager;

class ViewContainer : public QObject, public Configurable
{
    Q_OBJECT

  public slots:
    void showAddViewMenuSlot();
    void showViewMenuSlot();
    // void saveViewTemplate ();
    void deleteViewSlot();
    void addNewViewSlot();

  public:
    ViewContainer(const std::string& class_id, const std::string& instance_id, Configurable* parent,
                  ViewManager* view_manager, QTabWidget* tab_widget, int window_cnt);
    virtual ~ViewContainer();

    const std::vector<std::unique_ptr<View>>& getViews() const;

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    virtual std::string getWindowName();
    //static unsigned int getViewCount() { return view_count_; }

    void addView(const std::string& class_name);
    void showView(QWidget* widget);

  protected:
    ViewManager& view_manager_;

    QTabWidget* tab_widget_{nullptr};

    int window_cnt_{0};

    std::vector<std::unique_ptr<View>> views_;

    //static unsigned int view_count_;

    virtual void checkSubConfigurables();
    void addView(View* view);
};

#endif  // VIEWCONTAINER_H
