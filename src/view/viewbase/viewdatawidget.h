
#pragma once

#include "appmode.h"

#include <QWidget>

class ViewToolSwitcher;
class Buffer;

/**
 */
class ViewDataWidget : public QWidget 
{
    Q_OBJECT
public:
    enum class RedrawType
    {
        Complete = 0, //clear display, update internal data and redraw display
        UpdateData,   //update internal data and redraw display
        UpdateDisplay //redraw display only
    };

    typedef std::map<std::string, std::shared_ptr<Buffer>> BufferData;

    ViewDataWidget(QWidget* parent = nullptr, Qt::WindowFlags f = 0);
    virtual ~ViewDataWidget() = default;

    void setToolSwitcher(ViewToolSwitcher* tool_switcher);

    void loadingStarted();
    void loadingDone();
    void updateData(const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset);
    void clearData();
    void redrawData(RedrawType type = RedrawType::Complete);

    virtual bool hasData() const = 0;               //checks if the view obtains/shows any data
    virtual void appModeSwitch(AppMode app_mode) {} //reacts on switching the application mode

signals:
    void displayChanged();
    void dataLoaded();
    void redrawStarted();
    void redrawDone();
    void updateStarted();
    void updateDone();
    
protected:
    virtual void toolChanged_impl(int tool_id) = 0;        //reacts on tool switches
    virtual void loadingStarted_impl() = 0;                //reacts on starting a reload
    virtual void loadingDone_impl() = 0;                   //reacts on finishing a reload
    virtual void updateData_impl(const BufferData& data,   //reacts on receiving new data
                                 bool requires_reset) = 0; 
    virtual void clearData_impl() = 0;                     //clears all data
    virtual void prepareData_impl() = 0;                   //prepares internal data needed for a display redraw
    virtual void redrawData_impl() = 0;                    //redraws the display (note: meant to do a pure redraw given the current internal state)

    void endTool();

private:
    friend class ViewLoadStateWidget;

    void toolChanged(int mode, const QCursor& cursor);

    ViewToolSwitcher* tool_switcher_ = nullptr;
};
