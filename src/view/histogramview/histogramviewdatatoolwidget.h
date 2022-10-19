#ifndef HISTOGRAMVIEWDATATOOLWIDGET_H
#define HISTOGRAMVIEWDATATOOLWIDGET_H

#include <QCursor>
#include <QMenu>
#include <QWidget>
#include <map>

enum HistogramViewDataTool
{
    HG_DEFAULT_TOOL = 0,
    HG_SELECT_TOOL,
    HG_ZOOM_TOOL
};

class QToolButton;
class QToolBar;

class HistogramView;

class HistogramViewDataToolWidget : public QWidget
{
    Q_OBJECT

signals:
    //void toolChangedSignal(HistoGramViewDataTool selected, QCursor cursor);

    void invertSelectionSignal();
    void clearSelectionSignal();

    void zoomToHomeSignal ();

public slots:
    void actionTriggeredSlot(QAction* action);

    void loadingStartedSlot();
    void loadingDoneSlot();

public:
    HistogramViewDataToolWidget(HistogramView* view, QWidget* parent = nullptr);
    virtual ~HistogramViewDataToolWidget();

private:
    HistogramView* view_ {nullptr};

    QToolBar* toolbar_ {nullptr};

    QToolButton* select_button_{nullptr};

    void unselectAllTools();
};

#endif // HISTOGRAMVIEWDATATOOLWIDGET_H
