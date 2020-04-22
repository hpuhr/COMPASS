#ifndef VIEWPOINTSTOOLWIDGET_H
#define VIEWPOINTSTOOLWIDGET_H

#include <QCursor>
#include <QMenu>
#include <QWidget>
#include <map>

//Select first open View Point (from current ordering)
//Select next open View Point (from current View Point)
//Mark current Viewpoint as closed and select next open View Point (from current View Point)
//Mark current Viewpoint as open and select next open View Point (from current View Point)
//Clear All View Points and import from File
//Export All View Points as file

enum ViewPointsTool
{
    SELECT_NEXT_TOOL = 0,
    SELECT_NEXT_OPEN_TOOL,
    STEP_NEXT_TOOL,
    STEP_NEXT_OPEN_TOOL,
    OPEN_CURRENT_STEP_NEXT_TOOL,
    CLOSE_CURRENT_STEP_NEXT_TOOL
};

class QToolButton;
class QToolBar;

class ViewPointsWidget;

class ViewPointsToolWidget : public QWidget
{
    Q_OBJECT

signals:
    //void toolChangedSignal(ViewPointsTool selected, QCursor cursor);

public slots:
    void actionTriggeredSlot(QAction* action);

//    void loadingStartedSlot();
//    void loadingDoneSlot();

public:
    ViewPointsToolWidget(ViewPointsWidget* vp_widget, QWidget* parent = nullptr);

private:
    ViewPointsWidget* vp_widget_ {nullptr};

    QToolBar* toolbar_ {nullptr};

    QToolButton* select_next_button_{nullptr};
    QToolButton* select_next_open_button_{nullptr};

    QToolButton* step_next_open_button_{nullptr};
    QToolButton* open_current_step_next_button_{nullptr};
    QToolButton* close_current_step_next_button_{nullptr};
};

#endif // VIEWPOINTSTOOLWIDGET_H
