
#include "viewdatawidget.h"
#include "viewtoolswitcher.h"
#include "buffer.h"

#include <iostream>

/**
 */
ViewDataWidget::ViewDataWidget(QWidget* parent, Qt::WindowFlags f)
:   QWidget(parent, f)
{
    setObjectName("data_widget");
}

/**
 */
bool ViewDataWidget::hasData() const
{
    return !data_.empty();
}

/**
 */
bool ViewDataWidget::showsData() const
{
    return (hasData() && drawn_);
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

/**
 */
void ViewDataWidget::loadingStarted()
{
    logdbg << "ViewDataWidget::loadingStarted";

    //clear and update display
    clearData();
    redrawData(false);

    loadingStarted_impl();
}

/**
 */
void ViewDataWidget::loadingDone()
{
    logdbg << "ViewDataWidget::loadingDone";

    loadingDone_impl();

    emit dataLoaded();
}

/**
*/
void ViewDataWidget::loadingDone_impl()
{
    //default behavior: recompute and redraw after reload
    redrawData(true);
}

/**
 */
void ViewDataWidget::updateData(const BufferData& data, bool requires_reset)
{
    logdbg << "ViewDataWidget::updateData";

    data_ = data;

    updateData_impl(requires_reset);
}

/**
 */
void ViewDataWidget::clearData()
{
    logdbg << "ViewDataWidget::clearData";

    data_  = {};
    drawn_ = false;

    clearData_impl();
}

/**
*/
bool ViewDataWidget::redrawData(bool recompute)
{
    logdbg << "ViewDataWidget::redrawData";

    emit redrawStarted();

    drawn_ = redrawData_impl(recompute);

    emit redrawDone();

    return drawn_;
}

/**
*/
void ViewDataWidget::liveReload()
{
    liveReload_impl();

    emit liveDataLoaded();
}
