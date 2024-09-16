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

#include "evaluation_commands.h"
#include "rtcommand/rtcommand_macros.h"
#include "rtcommand_registry.h"
#include "compass.h"
#include "evaluationmanager.h"
#include "evaluationmanagerwidget.h"

#include <boost/program_options.hpp>

REGISTER_RTCOMMAND(RTCommandGetEvalResult)

/**
*/
void init_evaluation_commands()
{
    RTCommandGetEvalResult::init();
}

/***************************************************************************************
 * RTCommandGetEvalResult
 ***************************************************************************************/

/**
*/
RTCommandGetEvalResult::RTCommandGetEvalResult()
    : rtcommand::RTCommand()
{
}

rtcommand::IsValid RTCommandGetEvalResult::valid() const 
{
    CHECK_RTCOMMAND_INVALID_CONDITION(result.empty(), "Result ID must not be empty")
    CHECK_RTCOMMAND_INVALID_CONDITION(table.empty(), "Table ID must not be empty")
    
    return rtcommand::RTCommand::valid(); 
}

/**
*/
bool RTCommandGetEvalResult::run_impl()
{
    return true;
}

/**
*/
bool RTCommandGetEvalResult::checkResult_impl()
{
    EvaluationManager& eval_man = COMPASS::instance().evaluationManager();

    if (!eval_man.hasResults())
    {
        setResultMessage("No evaluation results available");
        return false;
    }

    if (result.empty())
    {
        setResultMessage("No result id specified");
        return false;
    }

    if (table.empty())
    {
        setResultMessage("No table id specified");
        return false;
    }

    auto eval_widget = eval_man.widget();

    if (!eval_widget)
    {
        setResultMessage("No evaluation widget");
        return false;
    }

    auto json_reply = eval_widget->getTableData(result, table, !colwise, columns);

    if (!json_reply.has_value())
    {
        setResultMessage("Could not obtain table for given id");
        return false;
    }

    setJSONReply(json_reply.value());

    return true;
}

/**
 */
void RTCommandGetEvalResult::collectOptions_impl(OptionsDescription &options,
                                                 PosOptionsDescription &positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("result", po::value<std::string>()->default_value(""), "which evaluation result to retrieve")
        ("table", po::value<std::string>()->default_value(""), "which evaluation result table to retrieve")
        ("columns", po::value<std::string>()->default_value(""), "which table columns to retrieve")
        ("colwise", "retrieve evaluation result table column-wise");

    ADD_RTCOMMAND_POS_OPTION(positional, "result")
    ADD_RTCOMMAND_POS_OPTION(positional, "table")
    ADD_RTCOMMAND_POS_OPTION(positional, "columns")
}

/**
 */
void RTCommandGetEvalResult::assignVariables_impl(const VariablesMap &vars)
{
    RTCOMMAND_GET_VAR_OR_THROW(vars, "result", std::string, result)
    RTCOMMAND_GET_VAR_OR_THROW(vars, "table", std::string, table)
    RTCOMMAND_GET_INTVECTOR_OR_THROW(vars, "columns", columns)
    RTCOMMAND_CHECK_VAR(vars, "colwise", colwise)
}
