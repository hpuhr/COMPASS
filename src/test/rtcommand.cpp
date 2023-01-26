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

#include <QObject>
#include <QMainWindow>
#include <QApplication>
#include <QWindow>

namespace rtcommand
{

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
std::unique_ptr<WaitCondition> RTCommandWaitCondition::create() const
{
    if (!isSet())
        return {};

    auto main_window = mainWindow();
    if (!main_window)
        return {};

    if (type == Type::Signal)
    {
        return std::unique_ptr<WaitCondition>(new WaitConditionSignal(obj, 
                                                                      value, 
                                                                      timeout_ms,
                                                                      main_window));
    }
    else if (type == Type::Delay)
    {
        return std::unique_ptr<WaitCondition>(new WaitConditionDelay(timeout_ms));
    }

    return {};
}

/**
 */
QString RTCommandResult::generateMessage() const
{
    QString msg;
    if (wc_state == WaitConditionState::BadInit)
        msg = "Could not init wait condition";
    else if (cmd_state == CmdState::BadConfig)
        msg = "Badly configured command";
    else if (cmd_state == CmdState::Failed)
        msg = "Command failed";
    else if (wc_state == WaitConditionState::Failed)
        msg = "Wait condition failed";
    else if (success())
        msg = "Success";
    else
        msg = "Strange state";

    return msg;
}

/**
 */
bool RTCommand::run() const
{
    res.cmd_state = RTCommandResult::CmdState::Fresh;
    res.cmd_msg   = "";

    //command configuration valid?
    if (!valid())
    {
        res.cmd_state = RTCommandResult::CmdState::BadConfig;
        return false;
    }
        
    //run command
    if (!run_impl())
    {
        res.cmd_state = RTCommandResult::CmdState::Failed;
        return false;
    }
        
    res.cmd_state = RTCommandResult::CmdState::Success;
    return true;
}

} // namespace rtcommand
