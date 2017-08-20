#ifndef TASKMANAGER_H
#define TASKMANAGER_H

class RadarPlotPositionCalculatorTask;

class TaskManager
{
public:

    virtual ~TaskManager();

    static TaskManager& instance()
    {
        static TaskManager instance;
        return instance;
    }

    RadarPlotPositionCalculatorTask* getRadarPlotPositionCalculatorTask();

    void shutdown ();

private:
    TaskManager();

protected:
    RadarPlotPositionCalculatorTask* radar_plot_position_calculator_task_ {nullptr};
};

#endif // TASKMANAGER_H
