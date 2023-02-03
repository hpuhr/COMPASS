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

#pragma once

#include <QString>

namespace rtcommand
{

/**
 * The execution state a command can be in.
 */
enum class CmdState
{
    Fresh = 0,
    BadConfig,
    Failed,
    Success
};

/**
 * The execution state a wait condition can be in.
 */
enum class WaitConditionState
{
    Unknown = 0,
    BadInit,
    Failed,
    Success
};

/**
 * Keeps track of runtime command execution result and state (including the state of the command's wait condition).
 */
struct RTCommandResult
{
    bool success() const
    { 
        //as the wait condition might be extremely important for any upcoming commands, 
        //its state is part of the successful execution of a command
        return (wc_state == WaitConditionState::Success && cmd_state == CmdState::Success);
    }

    void reset()
    {
        wc_state  = WaitConditionState::Unknown;
        cmd_state = CmdState::Fresh;
        cmd_msg   = "";
    }

    QString toString() const;

    WaitConditionState wc_state  = WaitConditionState::Unknown; // wait condition state
    CmdState           cmd_state = CmdState::Fresh;             // execution state
    QString            cmd_msg;                                 // optional execution result message 

    QString            data; // command result data (most likely json)
};

/**
 * Describes a runtime command.
*/
struct RTCommandDescription
{
    QString name;
    QString description;
};

} // namespace rtcommand