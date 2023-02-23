
#pragma once

#include "appmode.h"

#include <QWidget>

class ViewToolSwitcher;
class Buffer;

/**
 * Base class for view data widgets, which are used to display data in a view.
 */
class ViewDataWidget : public QWidget 
{
    Q_OBJECT
public:
    typedef std::map<std::string, std::shared_ptr<Buffer>> BufferData;

    ViewDataWidget(QWidget* parent = nullptr, Qt::WindowFlags f = 0);
    virtual ~ViewDataWidget() = default;

    void setToolSwitcher(ViewToolSwitcher* tool_switcher);

    void loadingStarted();
    void loadingDone();
    void updateData(const BufferData& buffer_data, bool requires_reset);
    void clearData();
    bool redrawData(bool recompute, bool notify = false);
    void liveReload();

    bool hasData() const;
    bool showsData() const;

    virtual void appModeSwitch(AppMode app_mode) {} //reacts on switching the application mode

signals:
    void displayChanged();
    void dataLoaded();
    void liveDataLoaded();
    void redrawStarted();
    void redrawDone();
    void updateStarted();
    void updateDone();
    
protected:
    virtual void toolChanged_impl(int tool_id) = 0;        //implements reactions on tool switches
    virtual void loadingStarted_impl() = 0;                //implements behavior at starting a reload
    virtual void loadingDone_impl();                       //implements behavior at finishing a reload
    virtual void updateData_impl(bool requires_reset) = 0; //implements behavior at receiving new data
    virtual void clearData_impl() = 0;                     //implements clearing all view data
    virtual bool redrawData_impl(bool recompute) = 0;      //implements redrawing the display (and possibly needed computations), and returns if the redraw succeeded
    virtual void liveReload_impl() = 0;                    //implements data reload during live running mode

    void endTool();

    const BufferData& data() const { return data_; }
    BufferData& data() { return data_; } //exposed because of selection

private:
    friend class ViewLoadStateWidget;

    void toolChanged(int mode, const QCursor& cursor);

    ViewToolSwitcher* tool_switcher_ = nullptr;

    BufferData data_;
    bool       drawn_ = false;
};
