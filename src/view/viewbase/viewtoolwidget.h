
#pragma once

#include <map>

#include <QToolBar>

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
                           const QString& key_combination = QString());
    void addSeparator();

private:
    void toolSwitched(int id, const QCursor& cursor);

    ViewToolSwitcher* tool_switcher_ = nullptr;

    std::map<int, QAction*> tool_actions_;
};
