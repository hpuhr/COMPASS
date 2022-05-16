/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RADARPLOTPOSITIONCALCULATOR_H_
#define RADARPLOTPOSITIONCALCULATOR_H_

#include "configurable.h"
#include "task.h"
#include "dbcontent/variable/variableset.h"

#include <QObject>

#include "boost/date_time/posix_time/posix_time.hpp"

#include <memory>


class Buffer;
class DBContent;

class RadarPlotPositionCalculatorTaskDialog;
class TaskManager;
class UpdateBufferDBJob;

namespace dbContent {
class Variable;
}

class QMessageBox;

class RadarPlotPositionCalculatorTask : public Task, public Configurable
{
    Q_OBJECT

public slots:
    void loadedDataDataSlot(const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset);
    void loadingDoneSlot();

    void updateDoneSlot(DBContent& db_content);

    void dialogCloseSlot();

public:
    RadarPlotPositionCalculatorTask(const std::string& class_id, const std::string& instance_id,
                                    TaskManager& task_manager);
    virtual ~RadarPlotPositionCalculatorTask();

    bool isCalculating();
    unsigned int getNumLoaded() { return num_loaded_; }

    RadarPlotPositionCalculatorTaskDialog* dialog();

    virtual bool checkPrerequisites();
    virtual bool isRecommended();
    virtual bool isRequired();

    virtual bool canRun();
    virtual void run();

    static const std::string DONE_PROPERTY_NAME;

protected:
    //std::map<std::string, std::shared_ptr<UpdateBufferDBJob>> job_ptr_;

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;

    bool calculating_{false};

    unsigned int num_loaded_{0};
    std::map<std::string, std::shared_ptr<Buffer>> data_;
    std::set<std::string> dbcontent_done_;

    std::unique_ptr<RadarPlotPositionCalculatorTaskDialog> dialog_;

    QMessageBox* msg_box_{nullptr};

    dbContent::VariableSet getReadSetFor(const std::string& dbcontent_name);
};

#endif /* RADARPLOTPOSITIONCALCULATOR_H_ */
