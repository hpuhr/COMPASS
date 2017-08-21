#ifndef RADARPLOTPOSITIONCALCULATOR_H_
#define RADARPLOTPOSITIONCALCULATOR_H_

#include "configurable.h"

#include <QObject>
#include <memory>

class Buffer;
class RadarPlotPositionCalculatorTaskWidget;
class TaskManager;

class RadarPlotPositionCalculatorTask : public QObject, public Configurable
{
    Q_OBJECT

public slots:
    void readJobIntermediateSlot (std::shared_ptr<Buffer> buffer);
    void readJobObsoleteSlot ();
    void readJobDoneSlot();

public:
    RadarPlotPositionCalculatorTask(const std::string &class_id, const std::string &instance_id, TaskManager* task_manager);
    virtual ~RadarPlotPositionCalculatorTask();

    void calculate ();

    bool isCalculating ();
    unsigned int getNumLoaded () { return num_loaded_; }

    RadarPlotPositionCalculatorTaskWidget* widget();

protected:
    bool calculating_ {false};

    unsigned int num_loaded_ {0};

    RadarPlotPositionCalculatorTaskWidget* widget_ {nullptr};
};

#endif /* RADARPLOTPOSITIONCALCULATOR_H_ */
