#ifndef SCATTERPLOTVIEWDATATOOLWIDGET_H
#define SCATTERPLOTVIEWDATATOOLWIDGET_H

#include <QCursor>
#include <QMenu>
#include <QWidget>
#include <map>

enum ScatterPlotViewDataTool
{
    SP_ZOOM_TOOL = 0,
    //LABEL_TOOL,
    //MEASURE_TOOL,
    SP_SELECT_TOOL
    //LABEL_MULTIPLE_TOOL
};

class QToolButton;
class QToolBar;

class ScatterPlotView;

class ScatterPlotViewDataToolWidget : public QWidget
{
    Q_OBJECT

signals:
    void toolChangedSignal(ScatterPlotViewDataTool selected, QCursor cursor);

    void invertSelectionSignal();
    void clearSelectionSignal();

    void zoomToHomeSignal ();

public slots:
    void actionTriggeredSlot(QAction* action);

    void loadingStartedSlot();
    void loadingDoneSlot();

public:
    ScatterPlotViewDataToolWidget(ScatterPlotView* view, QWidget* parent = nullptr);
    virtual ~ScatterPlotViewDataToolWidget();

private:
    ScatterPlotView* view_ {nullptr};

    QToolBar* toolbar_ {nullptr};

    QToolButton* zoom_button_{nullptr};
    QToolButton* select_button_{nullptr};

    void unselectAllTools();
};

#endif // SCATTERPLOTVIEWDATATOOLWIDGET_H
