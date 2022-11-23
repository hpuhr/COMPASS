
#pragma once

#include <QWidget>

class ViewToolSwitcher;

/**
 */
class ViewDataWidget : public QWidget 
{
public:
    ViewDataWidget(QWidget* parent = nullptr, Qt::WindowFlags f = 0);
    virtual ~ViewDataWidget() = default;

    void setToolSwitcher(ViewToolSwitcher* tool_switcher);
    
protected:
    virtual void toolChanged_impl(int tool_id) = 0;
    void endTool();

private:
    void toolChanged(int mode, const QCursor& cursor);

    ViewToolSwitcher* tool_switcher_ = nullptr;
};