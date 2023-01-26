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

#include "rtcommand_runner.h"
#include "rtcommand.h"
#include "rtcommand_chain.h"
#include "rtcommand_wait_condition.h"

#include "logger.h"

#include <QApplication>

namespace rtcommand
{

/**
 */
RTCommandRunner::RTCommandRunner() = default;

/**
 */
RTCommandRunner::~RTCommandRunner() = default;

/**
 */
void RTCommandRunner::addCommand(std::unique_ptr<RTCommand>&& cmd)
{
    commands_.push(std::shared_ptr<RTCommand>(cmd.release()));

    if (!isRunning())
        start();
}

/**
 */
void RTCommandRunner::addCommands(RTCommandChain&& cmds)
{
    while (auto cmd = cmds.pop())
        addCommand(std::move(cmd));
}

/**
 */
int RTCommandRunner::numCommands() const
{
    return (int)commands_.unsafe_size();
}

/**
 */
void RTCommandRunner::run()
{
    while (!commands_.empty())
    {
        std::shared_ptr<RTCommand> cmd;
        if (!commands_.try_pop(cmd))
            continue;

        auto cmd_ptr = cmd.get();

        auto cb = [ cmd_ptr ] () { return cmd_ptr->run(); };

        loginf << "Executing command '" << cmd_ptr->name().toStdString() << "'";

        bool ok = true;
        //QMetaObject::invokeMethod(qApp, cb, Qt::BlockingQueuedConnection, &ok);

        loginf << "Executing command '" 
               << cmd_ptr->name().toStdString() 
               << "' [" << (ok ? "Succeded" : "Failed") << "]";
    }
}

} // namespace rtcommand
