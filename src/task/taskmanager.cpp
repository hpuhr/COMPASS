#include "atsdb.h"
#include "taskmanager.h"
#include "radarplotpositioncalculatortask.h"

#include <cassert>

TaskManager::TaskManager(const std::string &class_id, const std::string &instance_id, ATSDB *atsdb)
: Configurable (class_id, instance_id, atsdb, "conf/config_task.xml")
{
    createSubConfigurables();
}

TaskManager::~TaskManager()
{
    if (radar_plot_position_calculator_task_)
    {
        delete radar_plot_position_calculator_task_;
        radar_plot_position_calculator_task_=nullptr;
    }
}

RadarPlotPositionCalculatorTask* TaskManager::getRadarPlotPositionCalculatorTask()
{
    assert (radar_plot_position_calculator_task_);
    return radar_plot_position_calculator_task_;
}

void TaskManager::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    if (class_id.compare ("RadarPlotPositionCalculatorTask") == 0)
    {
        assert (!radar_plot_position_calculator_task_);
        radar_plot_position_calculator_task_ = new RadarPlotPositionCalculatorTask (class_id, instance_id, this);
        assert (radar_plot_position_calculator_task_);
    }
    else
        throw std::runtime_error ("TaskManager: generateSubConfigurable: unknown class_id "+class_id );
}

void TaskManager::checkSubConfigurables ()
{
    if (!radar_plot_position_calculator_task_)
    {
        radar_plot_position_calculator_task_ = new RadarPlotPositionCalculatorTask ("RadarPlotPositionCalculatorTask", "RadarPlotPositionCalculatorTask0", this);
        assert (radar_plot_position_calculator_task_);
    }
}

void TaskManager::shutdown ()
{
    // TODO waiting for jobs
}
