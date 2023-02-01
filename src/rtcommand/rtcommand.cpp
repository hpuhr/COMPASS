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

#include <iostream>

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
 * Generates a string describing the result.
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

void RTCommandWaitCondition::setDelay(int ms)
{
    *this      = {};
    type       = Type::Delay;
    timeout_ms = ms;
}

/***************************************************************************************
 * RTCommand
 ***************************************************************************************/

/**
 * Run the command and track state.
 */
bool RTCommand::run() const
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
    return true;
}

} // namespace rtcommand
