#ifndef RADARPLOTPOSITIONCALCULATORTASKWIDGET_H_
#define RADARPLOTPOSITIONCALCULATORTASKWIDGET_H_

#include <QWidget>

class Buffer;
class QLabel;
class RadarPlotPositionCalculatorTask;
class DBObjectComboBox;
class DBOVariableSelectionWidget;

class RadarPlotPositionCalculatorTaskWidget : public QWidget
{
    Q_OBJECT

public slots:
    void dbObjectChangedSlot();
    void anyVariableChangedSlot();
    void calculateSlot ();

public:
    RadarPlotPositionCalculatorTaskWidget(RadarPlotPositionCalculatorTask& task, QWidget * parent = 0, Qt::WindowFlags f = 0);
    virtual ~RadarPlotPositionCalculatorTaskWidget();

    void update ();

protected:
    RadarPlotPositionCalculatorTask& task_;

    DBObjectComboBox* object_box_ {nullptr};
    DBOVariableSelectionWidget* key_box_ {nullptr};
    DBOVariableSelectionWidget* datasource_box_ {nullptr};
    DBOVariableSelectionWidget* range_box_ {nullptr};
    DBOVariableSelectionWidget* azimuth_box_ {nullptr};
    DBOVariableSelectionWidget* altitude_box_ {nullptr};

    DBOVariableSelectionWidget* latitude_box_ {nullptr};
    DBOVariableSelectionWidget* longitude_box_ {nullptr};

    QLabel *count_label_ {nullptr};
    QLabel *load_status_label_ {nullptr};
    QLabel *calculated_status_label_ {nullptr};
    QLabel *written_status_label_  {nullptr};

    void setDBOBject (const std::string& object_name);
};

#endif /* RADARPLOTPOSITIONCALCULATORTASKWIDGET_H_ */
