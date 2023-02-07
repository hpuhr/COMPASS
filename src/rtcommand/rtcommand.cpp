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

#include "logger.h"

#include <iostream>

#include <boost/program_options.hpp>
#include <boost/optional.hpp>

#include <QObject>
#include <QMainWindow>
#include <QApplication>
#include <QWindow>
#include <QDialog>

REGISTER_RTCOMMAND(rtcommand::RTCommandEmpty)
REGISTER_RTCOMMAND(rtcommand::RTCommandHelp)

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

/**
*/
QDialog* activeDialog()
{
    QWidget* w = qApp->activeModalWidget();
    if (!w)
        return nullptr;

    QDialog* dlg = dynamic_cast<QDialog*>(w);
    if (!dlg)
        return nullptr;

    return dlg;
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
    try
    {
        //add basic command options here
        ADD_RTCOMMAND_OPTIONS(options)
            ("wait_condition", po::value<std::string>()->default_value(""), "wait condition config string")
            ("async", "enables asynchrous command execution, meaning execution will return immediately after the command has been deployed to the main thread")
            ("help,h", "show command help information");

        //collect from derived 
        collectOptions_impl(options);
    }
    catch(const std::exception& ex)
    {
        logerr << "RTCommand::collectOptions(): Error: " << ex.what();
        return false;
    }
    catch(...)
    {
        logerr << "RTCommand::collectOptions(): Unknown error";
        return false;
    }

    return true;
}

/**
 * Assigns scanned variables to struct data throughout the class hierarchy.
 */
bool RTCommand::assignVariables(const boost::program_options::variables_map& variables)
{
    try
    {
        //assign basic options here
        QString condition_config_str;
        RTCOMMAND_GET_QSTRING_OR_THROW(variables, "wait_condition", condition_config_str)

        if (!condition_config_str.isEmpty() &&
            !condition.setFromString(condition_config_str))
            throw ("Could not configure condition");

        RTCOMMAND_CHECK_VAR(variables, "async", execute_async)

        //assign in derived 
        assignVariables_impl(variables);
    }
    catch(const std::exception& ex)
    {
        logerr << "RTCommand::assignVariables(): Error: " << ex.what();
        return false;
    }
    catch(...)
    {
        logerr << "RTCommand::assignVariables(): Unknown error";
        return false;
    }

    return true;
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
bool RTCommand::configure(const RTCommandString& cmd)
{
    if (!cmd.valid())
    {
        //std::cout << "RTCommand::configure: Passed command not valid" << std::endl;
        return false;
    }

    //does the command even concern me?
    if (name() != cmd.cmdName())
    {
        //std::cout << "RTCommand::configure: Name '" << name().toStdString() 
        //          << "' does not match command name '" << cmd.cmdName().toStdString() << "'" << std::endl;
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
    if (!cmd.parse(vm, od))
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
 * RTCommandHelp
 ***************************************************************************************/

/**
 */
bool RTCommandHelp::run_impl() const
{
    if (command.isEmpty())
    {
        const auto& cmds = RTCommandRegistry::instance().availableCommands();

        loginf << "Available commands: ";
        loginf << "";

        for (const auto& elem : cmds)
        {
            loginf << "   " << elem.first.toStdString();
            loginf << "      " << elem.second.description.toStdString();
        }
    }
    else
    {
        if (!RTCommandRegistry::instance().hasCommand(command))
            return false;

        auto cmd =  RTCommandRegistry::instance().createCommandTemplate(command);
        if (!cmd)
            return false;

        boost::program_options::options_description options;
        if (!cmd->collectOptions(options))
            return false;

        loginf << cmd->name().toStdString();
        loginf << "";
        loginf << "   " << cmd->description().toStdString();
        loginf << "";
        
        for (const auto& o : options.options())
        {
            loginf << "   " << o->long_name() << " " << o->format_name();
            loginf << "      " << o->description();
            loginf << "";
        }
    }

    return true;
}

/**
 */
void RTCommandHelp::collectOptions_impl(OptionsDescription& options)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("command", po::value<std::string>()->default_value(""), "command to retrieve help information for");
}

/**
 */
void RTCommandHelp::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_QSTRING_OR_THROW(variables, "command", command)
}

} // namespace rtcommand
