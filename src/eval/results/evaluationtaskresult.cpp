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

#include "evaluationtaskresult.h"
#include "evaluationcalculator.h"

#include "compass.h"
#include "logger.h"

/**
 */
EvaluationTaskResult::EvaluationTaskResult(unsigned int id, 
                                           TaskManager& task_man)
:   TaskResult(id, task_man)
{
}

/**
 */
EvaluationTaskResult::~EvaluationTaskResult() = default;

/**
 */
EvaluationTaskResult::ContentPtr EvaluationTaskResult::createOnDemandContent(const std::string& section_id,
                                                                             const std::string& content_id) const
{
    if (!calculator_)
        return ContentPtr();

    ContentPtr content;

    //@TODO

    return content;
}

/**
 */
void EvaluationTaskResult::toJSON_impl(nlohmann::json& root_node) const
{
}

/**
 */
bool EvaluationTaskResult::fromJSON_impl(const nlohmann::json& j)
{
    if (!j.contains(FieldConfig))
        return false;

    const auto& jconfig = j.at(FieldConfig);
    if (!jconfig.is_object())
        return false;
    
    auto& eval_manager = COMPASS::instance().evaluationManager();

    try
    {
        //create calculator based on stored config
        calculator_.reset(new EvaluationCalculator(eval_manager, jconfig));
    }
    catch(const std::exception& ex)
    {
        logerr << "EvaluationTaskResult: toJSON_impl: Could not create calculator from stored config: " << ex.what();
        return false;
    }
    catch(...)
    {
        logerr << "EvaluationTaskResult: toJSON_impl: Could not create calculator from stored config: Unknown error";
        return false;
    }
    
    return true;
}
