#ifndef RADARPLOTPOSITIONCALCULATORTASKDIALOG_H
#define RADARPLOTPOSITIONCALCULATORTASKDIALOG_H

#include <QDialog>

class RadarPlotPositionCalculatorTask;
class RadarPlotPositionCalculatorTaskWidget;

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

protected:
    RadarPlotPositionCalculatorTask& task_;

    std::unique_ptr<RadarPlotPositionCalculatorTaskWidget> widget_;

    bool run_wanted_ {false};
};

#endif // RADARPLOTPOSITIONCALCULATORTASKDIALOG_H
