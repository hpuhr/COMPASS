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
#include "eval/results/base/single.h"

#include "task/result/report/sectioncontentfigure.h"
#include "task/result/report/sectioncontenttable.h"

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

namespace helpers
{
    /**
     */
    std::pair<unsigned int, Evaluation::RequirementResultID> resultInfoFromProperties(const ResultReport::SectionContent* content)
    {
        assert(content->hasJSONProperty(EvaluationRequirementResult::Single::PropertyUTN));
        assert(content->hasJSONProperty(EvaluationRequirementResult::Single::PropertySectorLayer));
        assert(content->hasJSONProperty(EvaluationRequirementResult::Single::PropertyReqGroup));
        assert(content->hasJSONProperty(EvaluationRequirementResult::Single::PropertyReqName));

        unsigned int utn;
        Evaluation::RequirementResultID id;

        utn               = content->jsonProperty(EvaluationRequirementResult::Single::PropertyUTN);
        id.sec_layer_name = content->jsonProperty(EvaluationRequirementResult::Single::PropertySectorLayer);
        id.req_group_name = content->jsonProperty(EvaluationRequirementResult::Single::PropertyReqGroup);
        id.req_name       = content->jsonProperty(EvaluationRequirementResult::Single::PropertyReqName);

        return std::make_pair(utn, id);
    }

    /**
     */
    EvaluationRequirementResult::Single* obtainResult(const ResultReport::SectionContent* content,
                                                      EvaluationCalculator* calculator)
    {
        auto info = resultInfoFromProperties(content);

        loginf << "obtainResult: Obtaining result for" 
               << " utn " << info.first 
               << " layer " << info.second.sec_layer_name
               << " group " << info.second.req_group_name
               << " req " << info.second.req_name;

        //result already present?
        auto r = calculator->singleResult(info.second, info.first);
        if (r)
            return r;

        //otherwise evaluate for specified utn and requirement
        calculator->evaluate(true, false, { info.first }, { info.second });

        //then return result
        return calculator->singleResult(info.second, info.first);
    }
}

/**
 */
bool EvaluationTaskResult::loadOnDemandFigure(ResultReport::SectionContentFigure* figure) const
{
    auto calc = calculator();

    if (!calc || !calc->canEvaluate().ok())
    {
        logerr << "EvaluationTaskResult: loadOnDemandFigure: invalid calulcator";
        return false;
    }

    try
    {
        if (figure->name() == EvaluationRequirementResult::Single::TargetOverviewID)
        {
            //get result for section
            auto result = helpers::obtainResult(figure, calc);
            if (!result)
            {
                logerr << "EvaluationTaskResult: loadOnDemandFigure: result could not be obtained";
                return false;
            }

            //add overview to figure
            result->addOverviewToFigure(*figure);

            return true;
        }
    }
    catch(...)
    {
        logerr << "EvaluationTaskResult: loadOnDemandFigure: critical error during load";
    }
    
    return false;
}

/**
 */
bool EvaluationTaskResult::loadOnDemandTable(ResultReport::SectionContentTable* table) const
{
    auto calc = calculator();

    if (!calc || !calc->canEvaluate().ok())
    {
        logerr << "EvaluationTaskResult: loadOnDemandTable: invalid calulcator";
        return false;
    }

    try
    {
        if (table->name() == EvaluationRequirementResult::Single::tr_details_table_name_)
        {
            //get result for section
            auto result = helpers::obtainResult(table, calc);
            if (!result)
            {
                logerr << "EvaluationTaskResult: loadOnDemandTable: result could not be obtained";
                return false;
            }

            //add table details
            result->addDetailsToTable(*table);

            return true;
        }
    }
    catch(...)
    {
        logerr << "EvaluationTaskResult: loadOnDemandTable: critical error during load";
    }
    
    return false;
}

/**
 */
EvaluationCalculator* EvaluationTaskResult::calculator() const
{
    if (calculator_)
        return calculator_.get();

    if (!config_.is_object())
    {
        logerr << "EvaluationTaskResult: calculator: no config available";
        return nullptr;
    }

    auto& eval_manager = COMPASS::instance().evaluationManager();

    try
    {
        //create calculator based on stored config
        calculator_.reset(new EvaluationCalculator(eval_manager, config_));
    }
    catch(const std::exception& ex)
    {
        logerr << "EvaluationTaskResult: toJSON_impl: Could not create calculator from stored config: " << ex.what();
        return nullptr;
    }
    catch(...)
    {
        logerr << "EvaluationTaskResult: toJSON_impl: Could not create calculator from stored config: Unknown error";
        return nullptr;
    }

    return calculator_.get();
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
    return true;
}
