#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "singleton.h"
#include "configurable.h"

class ATSDB;
class RadarPlotPositionCalculatorTask;

class TaskManager : public Configurable
{
public:
    TaskManager(const std::string &class_id, const std::string &instance_id, ATSDB *atsdb);

    virtual ~TaskManager();

    RadarPlotPositionCalculatorTask* getRadarPlotPositionCalculatorTask();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    void shutdown ();

protected:
    RadarPlotPositionCalculatorTask* radar_plot_position_calculator_task_ {nullptr};

    virtual void checkSubConfigurables ();
};

#endif // TASKMANAGER_H
