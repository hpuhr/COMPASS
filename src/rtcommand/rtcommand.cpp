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

#include "rtcommand.h"
#include "rtcommand_wait_condition.h"
#include "rtcommand_registry.h"
#include "rtcommand_string.h"

#include <iostream>

#include <boost/program_options.hpp>

#include <QObject>
#include <QMainWindow>
#include <QApplication>
#include <QWindow>

REGISTER_RTCOMMAND(rtcommand::RTCommandEmpty)

namespace rtcommand
{

/***************************************************************************************
 * Helpers
 ***************************************************************************************/

/**
 * Return the application's main window.
 */
QMainWindow* mainWindow()
{
    for (auto win : qApp->topLevelWidgets())
    {
        auto mw = dynamic_cast<QMainWindow*>(win);
        if (mw)
            return mw;
    }
    return nullptr;
}

/***************************************************************************************
 * RTCommandResult
 ***************************************************************************************/

/**
 * Generates a string describing the result state.
 */
QString RTCommandResult::toString() const
{
    QString s;
    if (wc_state == WaitConditionState::BadInit)
        s = "Could not init wait condition";
    else if (cmd_state == CmdState::BadConfig)
        s = "Badly configured command";
    else if (cmd_state == CmdState::Failed)
        s = "Command failed";
    else if (wc_state == WaitConditionState::Failed)
        s = "Wait condition failed";
    else if (cmd_state == CmdState::Success && wc_state == WaitConditionState::Success)
        s = "Success";
    else
        s = "Strange state";

    if (!cmd_msg.isEmpty())
        s += " (" + cmd_msg + ")";

    return s;
}

/***************************************************************************************
 * RTCommandWaitCondition
 ***************************************************************************************/

/**
 * Configures the wait condition as 'signal' type.
 */
void RTCommandWaitCondition::setSignal(const QString& obj_name, 
                                       const QString& signal_name, 
                                       int timeout_in_ms)
{
    *this      = {};
    type       = Type::Signal;
    obj        = obj_name;
    value      = signal_name;
    timeout_ms = timeout_in_ms;
}

/**
 * Configures the wait condition as 'delay' type.
 */
void RTCommandWaitCondition::setDelay(int ms)
{
    *this      = {};
    type       = Type::Delay;
    timeout_ms = ms;
}

/**
 * Configures the wait condition from the given config string.
 * 
 * Either:
 * "signal;obj_name;obj_signal_name;timeout" OR
 * "delay;timeout"
 */
bool RTCommandWaitCondition::setFromString(const QString& config_string)
{
    QStringList parts = config_string.split(";");

    if (parts.count() < 1 || parts[ 0 ].isEmpty())
        return false;

    if (parts[ 0 ] == "signal")
    {
        if (parts.count() != 4)
            return false;

        obj   = parts[ 1 ];
        value = parts[ 2 ];
        
        bool ok;
        timeout_ms = parts[ 3 ].toInt(&ok);

        if (obj.isEmpty() || value.isEmpty() || !ok)
            return false;

        type = Type::Signal;   

        return true;
    }
    else if (parts[ 0 ] == "delay")
    {
        if (parts.count() != 2)
            return false;

        bool ok;
        timeout_ms = parts[ 1 ].toInt(&ok);

        if (!ok)
            return false;

        type = Type::Delay;

        return true;
    }
    return false;
}

/***************************************************************************************
 * RTCommand
 ***************************************************************************************/

/**
 */
RTCommand::RTCommand() = default;

/**
 */
RTCommand::~RTCommand() = default;

/**
 * Collects command option descriptions throughout the class hierarchy.
 */
bool RTCommand::collectOptions(boost::program_options::options_description& options)
{
    bool ok;

    try
    {
        //add basic command options here
        ADD_RTCOMMAND_OPTIONS(options)
            ("wait_condition", po::value<std::string>()->default_value(""), "wait condition config string")
            ("help,h", "show command help information");

        //collect from derived 
        ok = collectOptions_impl(options);
    }
    catch(...)
    {
        ok = false;
    }

    return ok;
}

/**
 * Assigns scanned variables to struct data throughout the class hierarchy.
 */
bool RTCommand::assignVariables(const boost::program_options::variables_map& variables)
{
    bool ok;

    try
    {
        //assign basic options here
        QString condition_config_str;
        RTCOMMAND_GET_QSTRING_OR_FAIL(variables, "wait_condition", condition_config_str)

        if (!condition_config_str.isEmpty() &&
            !condition.setFromString(condition_config_str))
        {
            std::cout << "Could not configure condition!" << std::endl;
            return false;
        }

        //assign in derived 
        ok = assignVariables_impl(variables);
    }
    catch(...)
    {
        ok = false;
    }

    return ok;
}

/**
 * Run the command and track state.
 */
bool RTCommand::run() const
{
    try
    {
        result_.cmd_state = CmdState::Fresh;
        result_.cmd_msg   = "";

        //command configuration valid?
        if (!valid())
        {
            result_.cmd_state = CmdState::BadConfig;
            return false;
        }
            
        //run command
        if (!run_impl())
        {
            result_.cmd_state = CmdState::Failed;
            return false;
        }
        
        result_.cmd_state = CmdState::Success;
    }
    catch(...)
    {
        result_.cmd_state = CmdState::Failed;
        return false;
    }

    return true;
}

/**
 * Configure the command struct using the given command string.
 */
bool RTCommand::configure(const QString& cmd)
{
    //check if command is basically valid
    RTCommandString cmd_str(cmd);
    if (!cmd_str.valid())
    {
        //std::cout << "RTCommand::configure: Passed command not valid" << std::endl;
        return false;
    }

    //does the command even concern me?
    if (name() != cmd_str.cmdName())
    {
        //std::cout << "RTCommand::configure: Name '" << name().toStdString() 
        //          << "' does not match command name '" << cmd_str.cmdName().toStdString() << "'" << std::endl;
        return false;
    }

    //collect options description
    namespace po = boost::program_options;
    po::options_description od;
    if (!collectOptions(od))
    {
        //std::cout << "RTCommand::configure: Could not collect options" << std::endl;
        return false;
    }

    //parse command using collected options description
    po::variables_map vm;
    if (!cmd_str.parse(vm, od))
    {
        //std::cout << "RTCommand::configure: Could not parse command" << std::endl;
        return false;
    }

    //store parsed variables
    if (!assignVariables(vm))
    {
        //std::cout << "RTCommand::configure: Could not assign vars" << std::endl;
        return false;
    }

    return true;
}

/**
*/
void RTCommand::printHelpInformation()
{
    //@TODO
}

/***************************************************************************************
 * RTCommandObject
 ***************************************************************************************/

/**
 */
bool RTCommandObject::collectOptions_impl(OptionsDescription& options)
{
    //add basic command options here
    ADD_RTCOMMAND_OPTIONS(options)
        ("object,o", po::value<std::string>()->required(), "name of an ui object");
    return true;
}

/**
 */
bool RTCommandObject::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_QSTRING_OR_FAIL(variables, "object", obj)
    return true;
}

} // namespace rtcommand