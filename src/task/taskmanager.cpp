#include "taskmanager.h"
#include "radarplotpositioncalculatortask.h"

#include <cassert>

TaskManager::TaskManager()
{

}

TaskManager::~TaskManager()
{

}

RadarPlotPositionCalculatorTask* TaskManager::getRadarPlotPositionCalculatorTask()
{
    if (!radar_plot_position_calculator_task_)
    {
        radar_plot_position_calculator_task_ = new RadarPlotPositionCalculatorTask ();
    }
    assert (radar_plot_position_calculator_task_);
    return radar_plot_position_calculator_task_;
}

void TaskManager::shutdown ()
{
    if (radar_plot_position_calculator_task_)
    {
        delete radar_plot_position_calculator_task_;
        radar_plot_position_calculator_task_=nullptr;
    }
}
