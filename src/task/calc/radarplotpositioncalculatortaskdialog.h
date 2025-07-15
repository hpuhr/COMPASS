#pragma once

#include "radarplotpositioncalculatortaskwidget.h"

#include <QDialog>

#include <memory>

class RadarPlotPositionCalculatorTask;

class RadarPlotPositionCalculatorTaskDialog : public QDialog
{
    Q_OBJECT

signals:
    void closeSignal();

public slots:
    void okClickedSlot();
    void cancelClickedSlot();

public:
    RadarPlotPositionCalculatorTaskDialog(RadarPlotPositionCalculatorTask& task);
    virtual ~RadarPlotPositionCalculatorTaskDialog();

    bool runWanted() const;

    void updateCanRun();

protected:
    RadarPlotPositionCalculatorTask& task_;

    std::unique_ptr<RadarPlotPositionCalculatorTaskWidget> widget_;

    QPushButton* ok_button_ {nullptr};

    bool run_wanted_ {false};
};

