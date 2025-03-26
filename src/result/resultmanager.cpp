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

#include "resultmanager.h"
#include "compass.h"
#include "logger.h"
#include "taskresult.h"

 /**
  */
ResultManager::ResultManager(const std::string& class_id,
                             const std::string& instance_id,
                             COMPASS* parent)
    :   Configurable(class_id, instance_id, parent, "results.json")
{
     createSubConfigurables();
}

/**
 */
ResultManager::~ResultManager() = default;

const std::map<unsigned int, std::shared_ptr<TaskResult>>& ResultManager::results() const
{
    return results_;
}

std::shared_ptr<TaskResult> ResultManager::result(unsigned int id) const // get existing result
{
    assert (results_.count(id));
    return results_.at(id);
}

std::shared_ptr<TaskResult> ResultManager::getOrCreateResult (const std::string& name) // get or create result
{
    if (hasResult(name))
        return std::find_if(results_.begin(), results_.end(),
                            [&name](const std::pair<const unsigned int, std::shared_ptr<TaskResult>>& pair) {
                                return pair.second && pair.second->name() == name;
                            })->second;
    else // create
    {
        unsigned int new_id{0};

        if (results_.size())
            new_id = results_.rend()->first + 1;

        results_[new_id] = std::make_shared<TaskResult>(new_id);
        results_.at(new_id)->name(name);

        return results_.at(new_id);
    }
}

bool ResultManager::hasResult (const std::string& name) const
{
    auto it = std::find_if(results_.begin(), results_.end(),
                           [&name](const std::pair<const unsigned int, std::shared_ptr<TaskResult>>& pair) {
                               return pair.second && pair.second->name() == name;
                           });

    return it != results_.end();
}


void ResultManager::databaseOpenedSlot()
{

}

void ResultManager::databaseClosedSlot()
{

}
