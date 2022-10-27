/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

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
