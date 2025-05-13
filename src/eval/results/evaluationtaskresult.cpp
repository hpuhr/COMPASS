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
#include "eval/results/base/joined.h"

#include "task/result/report/sectioncontentfigure.h"
#include "task/result/report/sectioncontenttable.h"

#include "compass.h"
#include "logger.h"

#include <QMenu>

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
Result EvaluationTaskResult::recompute_impl()
{
    auto calc = calculator();

    calc->evaluate(true, true);

    return Result::succeeded();
}

/**
 */
Result EvaluationTaskResult::canRecompute_impl() const
{
    auto calc = calculator();
    if (!calc)
        return Result::failed("Calculator could not be instantiated");

    return calc->canEvaluate();
}

/**
 */
bool EvaluationTaskResult::recomputeNeeded_impl() const
{
    auto calc = calculator();
    if (!calc)
        return true;

    return calc->results().empty() || calc->evaluationUTNs().size() > 0;
}

namespace helpers
{
    /**
     */
    std::pair<unsigned int, Evaluation::RequirementResultID> singleResultContentProperties(const ResultReport::SectionContent* content)
    {
        assert(content);

        auto info = EvaluationRequirementResult::Single::singleContentProperties(*content);
        assert(info.has_value());

        return std::make_pair(info->first, info->second);
    }

    /**
     */
    Evaluation::RequirementResultID joinedResultContentProperties(const ResultReport::SectionContent* content)
    {
        assert(content);

        auto info = EvaluationRequirementResult::Joined::joinedContentProperties(*content);
        assert(info.has_value());

        return info.value();
    }

    /**
     */
    EvaluationRequirementResult::Single* obtainSingleResult(const ResultReport::SectionContent* content,
                                                            EvaluationCalculator* calculator)
    {
        auto info = singleResultContentProperties(content);

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

    /**
     */
    EvaluationRequirementResult::Joined* obtainJoinedResult(const ResultReport::SectionContent* content,
                                                            EvaluationCalculator* calculator)
    {
        auto info = joinedResultContentProperties(content);

        loginf << "obtainResult: Obtaining result for" 
               << " layer " << info.sec_layer_name
               << " group " << info.req_group_name
               << " req " << info.req_name;

        //result already present?
        auto r = calculator->joinedResult(info);
        if (r)
            return r;

        //otherwise evaluate for specified requirement
        calculator->evaluate(true, false, {}, { info });

        //then return result
        return calculator->joinedResult(info);
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
            auto result = helpers::obtainSingleResult(figure, calc);
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
        if (table->name() == EvaluationRequirementResult::Single::TRDetailsTableName)
        {
            //get result for section
            auto result = helpers::obtainSingleResult(table, calc);
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

/**
 */
bool EvaluationTaskResult::customContextMenu_impl(QMenu& menu, 
                                                  ResultReport::SectionContentTable* table, 
                                                  unsigned int row) const
{
    logdbg << "EvaluationTaskResult: customContextMenu_impl";

    if (table->name() == EvaluationRequirementResult::Single::TRDetailsTableName)
    {
        //target report details table in single result section
    }
    else if (table->name() == EvaluationRequirementResult::Joined::TargetsTableName)
    {
        //target table in joined result section
        auto info = helpers::joinedResultContentProperties(table);

        assert(table->hasColumn("UTN"));
        const auto& d = table->getData(row, "UTN");

        if (!d.is_number_unsigned())
            return false;

        unsigned int utn = d;
        
        //@TODO
        loginf << "EvaluationTaskResult: customContextMenu_impl: Context menu requested for utn " << utn;

        return true;
    }
    else if (table->name() == EvaluationRequirementResult::Base::RequirementOverviewTableName)
    {
        //requirement table in overview section
    }
    else if (table->name() == EvaluationData::TargetsTableName)
    {
        //evaluation target table
    }

    // unsigned int row_index = source_index.row();

    // if (result_ptrs_.at(row_index) && result_ptrs_.at(row_index)->isSingle())
    // {
    //     EvaluationRequirementResult::Single* single_result =
    //             static_cast<EvaluationRequirementResult::Single*>(result_ptrs_.at(row_index));
    //     assert (single_result);

    //     QMenu menu;

    //     unsigned int utn = single_result->utn();
    //     loginf << "SectionContentTable: customContextMenuSlot: utn " << utn;

    //     assert (eval_man_.getData().hasTargetData(utn));

    //     const EvaluationTargetData& target_data = eval_man_.getData().targetData(utn);

    //     if (target_data.use())
    //     {
    //         QAction* action = new QAction("Remove", this);
    //         connect (action, &QAction::triggered, this, &SectionContentTable::removeUTNSlot);
    //         action->setData(utn);

    //         menu.addAction(action);
    //     }
    //     else
    //     {
    //         QAction* action = new QAction("Add", this);
    //         connect (action, &QAction::triggered, this, &SectionContentTable::addUTNSlot);
    //         action->setData(utn);

    //         menu.addAction(action);
    //     }

    //     QAction* action = new QAction("Show Full UTN", this);
    //     connect (action, &QAction::triggered, this, &SectionContentTable::showFullUTNSlot);
    //     action->setData(utn);
    //     menu.addAction(action);

    //     QAction* action2 = new QAction("Show Surrounding Data", this);
    //     connect (action2, &QAction::triggered, this, &SectionContentTable::showSurroundingDataSlot);
    //     action2->setData(utn);
    //     menu.addAction(action2);

    //     menu.exec(table_view_->viewport()->mapToGlobal(p));
    // }
    // else
    //     loginf << "SectionContentTable: customContextMenuSlot: no associated utn";

    return false;
}

/**
 */
void EvaluationTaskResult::postprocessTable_impl(ResultReport::SectionContentTable* table) const
{
    if (table->name() == EvaluationRequirementResult::Single::TRDetailsTableName)
    {
        //target report details table in single result section
    }
    else if (table->name() == EvaluationRequirementResult::Joined::TargetsTableName)
    {
        //target table in joined result section
    }
    else if (table->name() == EvaluationRequirementResult::Base::RequirementOverviewTableName)
    {
        //requirement table in overview section
    }
    else if (table->name() == EvaluationData::TargetsTableName)
    {
        //evaluation target table
        auto this_unconst = const_cast<EvaluationTaskResult*>(this);
        table->registerCallBack("Rerun Evaluation", [ this_unconst ] () { this_unconst->recompute(true); });
    }
}
