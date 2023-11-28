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

#pragma once

#include "appmode.h"
#include "json.h"

#include <QToolBar>
#include <QKeySequence>

#include <map>
#include <set>
#include <functional>

#include <boost/optional.hpp>

class ViewToolSwitcher;
class ViewWidget;

class QShortcut;
class QCursor;
class QAction;

/**
 * Toolbar for views. Implements adding of tools and actions to the view's toolbar, switching of tools, etc.
 * Keeps the toolbar generic for all views by giving the possibility to pass callbacks.
 */
class ViewToolWidget : public QToolBar
{
public:
    typedef std::function<void()>         Callback;       //callback triggered if an action is clicked
    typedef std::function<void(bool)>     ToggleCallback; //callback triggered if a checkable action is toggled
    typedef std::function<void(QAction*)> UpdateCallback; //callback for updating an action's state (e.g. check state, enabled state, icon, etc.)

    ViewToolWidget(ViewWidget* view_widget, ViewToolSwitcher* tool_switcher, QWidget* parent = nullptr);
    virtual ~ViewToolWidget() = default;

    void addTool(int id, const UpdateCallback& cb_update = UpdateCallback());
    void addActionCallback(const QString& name,
                           const Callback& cb,
                           const UpdateCallback& cb_update = UpdateCallback(),
                           const QIcon& icon = QIcon(),
                           const QKeySequence& key_combination = QKeySequence());
    void addActionCallback(int id,
                           const QString& name,
                           const Callback& cb,
                           const UpdateCallback& cb_update = UpdateCallback(),
                           const QIcon& icon = QIcon(),
                           const QKeySequence& key_combination = QKeySequence());
    void addActionCallback(const QString& name,
                           const ToggleCallback& cb,
                           const UpdateCallback& cb_update = UpdateCallback(),
                           const QIcon& icon = QIcon(),
                           const QKeySequence& key_combination = QKeySequence(),
                           bool checked = false);
    void addActionCallback(int id,
                           const QString& name,
                           const ToggleCallback& cb,
                           const UpdateCallback& cb_update = UpdateCallback(),
                           const QIcon& icon = QIcon(),
                           const QKeySequence& key_combination = QKeySequence(),
                           bool checked = false);
    void addSpacer();
    void addSeparatorIfValid();
    void addScreenshotButton();
    void addConfigWidgetToggle();

    void enableAction(int id, bool enable);

    void updateItems();

    void loadingStarted();
    void loadingDone();
    void redrawStarted();
    void redrawDone();
    void appModeSwitch(AppMode app_mode);
    void configChanged();

    virtual nlohmann::json viewInfo(const std::string& what) const { return {}; }

private:
    struct Action
    {
        QAction* action;
        bool     is_tool;
    };

    void addAction(int id, QAction* action, bool is_tool);

    void toolSwitched(int id, const QCursor& cursor);

    QAction* addActionCallback_internal(const QString& name,
                                        const Callback& cb,
                                        const UpdateCallback& cb_update,
                                        const QIcon& icon,
                                        const QKeySequence& key_combination);
    QAction* addActionCallback_internal(const QString& name,
                                        const ToggleCallback& cb,
                                        const UpdateCallback& cb_update,
                                        const QIcon& icon,
                                        const QKeySequence& key_combination,
                                        bool checked);

    bool actionIsSpacer(QAction* action) const;
    bool separatorValid() const;

    ViewWidget*       view_widget_   = nullptr;
    ViewToolSwitcher* tool_switcher_ = nullptr;

    std::map<int, Action> actions_;
    std::set<QAction*>    spacers_;
    std::vector<Callback> update_callbacks_;

    
};
