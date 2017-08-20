#ifndef RADARPLOTPOSITIONCALCULATOR_H_
#define RADARPLOTPOSITIONCALCULATOR_H_

#include <QObject>

class RadarPlotPositionCalculatorTask : public QObject
{
    Q_OBJECT

public slots:
    void readJobIntermediateSlot (std::shared_ptr<Buffer> buffer);
    void readJobObsoleteSlot ();
    void readJobDoneSlot();

public:
    RadarPlotPositionCalculatorTask();
    virtual ~RadarPlotPositionCalculatorTask();

    void calculate ();

    bool isCalculating ();
    unsigned int getNumLoaded () { return num_loaded_; }

protected:
    bool calculating_ {false};

    unsigned int num_loaded_ {0};
};

#endif /* RADARPLOTPOSITIONCALCULATOR_H_ */
