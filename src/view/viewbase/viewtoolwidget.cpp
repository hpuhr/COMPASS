
#include "viewtoolwidget.h"
#include "viewtoolswitcher.h"

#include <QShortcut>
#include <QToolButton>
#include <QApplication>

#include <cassert>

/**
 */
ViewToolWidget::ViewToolWidget(ViewToolSwitcher* tool_switcher, QWidget* parent)
:   QToolBar      (parent)
,   tool_switcher_(tool_switcher)
{
    assert(tool_switcher_);

    connect(tool_switcher_, &ViewToolSwitcher::toolChanged, this, &ViewToolWidget::toolSwitched);
}

/**
 */
void ViewToolWidget::addTool(int id)
{
    if (tool_actions_.find(id) != tool_actions_.end())
        throw std::runtime_error("ViewToolWidget::addTool: duplicate id");

    if (!tool_switcher_->hasTool(id))
        throw std::runtime_error("ViewToolWidget::addTool: unknown tool id");

    auto tool = tool_switcher_->getTool(id);

    bool isActiveTool = tool_switcher_->currentTool() == id;
        
    QAction* action = new QAction;
    action->setCheckable(true);
    action->setChecked(isActiveTool);
    action->setText(tool->name);
    action->setIcon(tool->icon);

    auto cb = [ = ] (bool on) 
    { 
        if (on)
            tool_switcher_->setCurrentTool(id);
        else
            tool_switcher_->endCurrentTool(); 
    };

    connect(action, &QAction::toggled, cb);

    tool_actions_[ id ] = action;

    addAction(action);
}

/**
 */
void ViewToolWidget::addActionCallback(const QString& name,
                                       const Callback& cb,
                                       const QIcon& icon,
                                       const QString& key_combination)
{
    QAction* action = new QAction;
    
    QString full_name = name;

    if (!key_combination.isEmpty())
    {
        full_name += " [" + key_combination + "]";

        action->setShortcut(key_combination);
        action->setShortcutContext(Qt::ShortcutContext::WidgetShortcut);
    }

    action->setText(full_name);
    action->setIcon(icon);

    connect(action, &QAction::triggered, cb);

    addAction(action);
}

/**
 */
void ViewToolWidget::addSeparator()
{
    QWidget* w = new QWidget;
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    addWidget(w);
}

/**
 */
void ViewToolWidget::toolSwitched(int id, const QCursor& cursor)
{
    for (auto& t : tool_actions_)
    {
        t.second->blockSignals(true);
        t.second->setChecked(t.first == id);
        t.second->blockSignals(false);
    }
}
