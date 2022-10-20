
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
    typedef std::function<void()> Callback;

    ViewToolWidget(ViewToolSwitcher* tool_switcher, QWidget* parent = nullptr);
    virtual ~ViewToolWidget() = default;

    void addTool(int id);
    void addActionCallback(const QString& name,
                           const Callback& cb,
                           const QIcon& icon = QIcon(),
                           const QKeySequence& key_combination = QKeySequence());
    void addSeparator();

private:
    void toolSwitched(int id, const QCursor& cursor);

    ViewToolSwitcher* tool_switcher_ = nullptr;

    std::map<int, QAction*> tool_actions_;
};
