
#include "viewdatawidget.h"
#include "viewtoolswitcher.h"

#include <iostream>

ViewDataWidget::ViewDataWidget(QWidget* parent, Qt::WindowFlags f)
:   QWidget(parent, f)
{
}

/**
 */
void ViewDataWidget::setToolSwitcher(ViewToolSwitcher* tool_switcher)
{
    if (!tool_switcher)
        throw std::runtime_error("ViewDataWidget::setToolSwitcher: nullptr passed");
    if (tool_switcher_)
        throw std::runtime_error("ViewDataWidget::setToolSwitcher: called twice");

    tool_switcher_ = tool_switcher;
    tool_switcher_->setDataWidget(this);

    connect(tool_switcher_, &ViewToolSwitcher::toolChanged, this, &ViewDataWidget::toolChanged);

    tool_switcher_->update();
}

/**
 */
namespace
{
    void setCursorRecursive(QWidget* w, const QCursor& cursor)
    {
        w->setCursor(cursor);

        for(auto c : w->findChildren<QWidget*>())
            setCursorRecursive(c, cursor);
    }
}
void ViewDataWidget::toolChanged(int mode, const QCursor& cursor)
{
    setCursorRecursive(this, cursor);

    toolChanged_impl(mode);
}

/**
 */
void ViewDataWidget::endTool()
{
    if (tool_switcher_)
        tool_switcher_->endCurrentTool();
}
