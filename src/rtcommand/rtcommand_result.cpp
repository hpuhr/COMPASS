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

#include "rtcommand_result.h"
#include "timeconv.h"

namespace rtcommand
{

/**
 * Reset result.
 */
void RTCommandResult::reset()
{
    wc_state  = WaitConditionState::Unknown;
    cmd_state = CmdState::Fresh;
    cmd_msg   = "";
    
    runtime.reset();
    reply_data = {};
}

/**
 * Generates a string describing the result state.
 */
std::string RTCommandResult::stateToString() const
{
    std::string s;
    if (wc_state == WaitConditionState::BadInit)
        s = "Could not init wait condition";
    else if (cmd_state == CmdState::BadConfig)
        s = "Badly configured command";
    else if (cmd_state == CmdState::ExecFailed)
        s = "Command execution failed";
    else if (wc_state == WaitConditionState::Failed)
        s = "Wait condition failed";
    else if (cmd_state == CmdState::ResultCheckFailed)
        s = "Result check failed";
    else if (cmd_state == CmdState::Success && wc_state == WaitConditionState::Success)
        s = "Success";
    else
        s = "Undetermined state";

    if (!cmd_msg.empty())
        s += " (" + cmd_msg + ")";

    if (runtime.has_value())
        s += " [" + Utils::Time::toString(runtime.value(), 3) + "]";

    return s;
}

/**
*/
std::string RTCommandResult::toJSONReplyString() const
{
    //@TODO

    return std::string();
}

} // namespace rtcommand
