#ifndef RADARPLOTPOSITIONCALCULATORTASKWIDGET_H_
#define RADARPLOTPOSITIONCALCULATORTASKWIDGET_H_

#include <QWidget>

class Buffer;
class QLabel;
class RadarPlotPositionCalculatorTask;
class DBObjectComboBox;
class DBOVariableSelectionWidet;

class RadarPlotPositionCalculatorTaskWidget : public QWidget
{
    Q_OBJECT

public slots:
    void calculateSlot ();

public:
    RadarPlotPositionCalculatorTaskWidget(RadarPlotPositionCalculatorTask& task, QWidget * parent = 0, Qt::WindowFlags f = 0);
    virtual ~RadarPlotPositionCalculatorTaskWidget();

    void update ();

protected:
    RadarPlotPositionCalculatorTask& task_;

    DBObjectComboBox* object_box;

    QLabel *count_label_ {nullptr};
    QLabel *load_status_label_ {nullptr};
    QLabel *calculated_status_label_ {nullptr};
    QLabel *written_status_label_  {nullptr};
};

#endif /* RADARPLOTPOSITIONCALCULATORTASKWIDGET_H_ */
