#ifndef RADARPLOTPOSITIONCALCULATORTASKDIALOG_H
#define RADARPLOTPOSITIONCALCULATORTASKDIALOG_H

#include <QDialog>

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

    bool runWanted() const;

protected:
    RadarPlotPositionCalculatorTask& task_;

    bool run_wanted_ {false};
};

#endif // RADARPLOTPOSITIONCALCULATORTASKDIALOG_H
