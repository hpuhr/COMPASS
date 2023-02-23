
#pragma once

#include "appmode.h"

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
 */
class ViewToolWidget : public QToolBar
{
public:
    typedef std::function<void()>         Callback;
    typedef std::function<void(bool)>     ToggleCallback;
    typedef std::function<void(QAction*)> UpdateCallback;

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
    void addConfigWidgetToggle();

    void enableAction(int id, bool enable);

    void updateItems();

    void loadingStarted();
    void loadingDone();
    void redrawStarted();
    void redrawDone();

    void appModeSwitch(AppMode app_mode);

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
