/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

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
