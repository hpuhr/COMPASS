
#pragma once

#include <QToolBar>
#include <QKeySequence>

#include <map>
#include <functional>

class ViewToolSwitcher;

class QShortcut;
class QCursor;
class QAction;

/**
 */
class ViewToolWidget : public QToolBar
{
public:
    typedef std::function<void()>     Callback;
    typedef std::function<void(bool)> ToggleCallback;

    ViewToolWidget(ViewToolSwitcher* tool_switcher, QWidget* parent = nullptr);
    virtual ~ViewToolWidget() = default;

    void addTool(int id);
    void addActionCallback(const QString& name,
                           const Callback& cb,
                           const QIcon& icon = QIcon(),
                           const QKeySequence& key_combination = QKeySequence());
    void addActionCallback(const QString& name,
                           const ToggleCallback& cb,
                           const QIcon& icon = QIcon(),
                           const QKeySequence& key_combination = QKeySequence(),
                           bool checked = false);
    void addSpacer();

private:
    void toolSwitched(int id, const QCursor& cursor);

    ViewToolSwitcher* tool_switcher_ = nullptr;

    std::map<int, QAction*> tool_actions_;
};
