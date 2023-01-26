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
#include "rtcommand_runner_stash.h"

#include "logger.h"

#include <QApplication>

namespace rtcommand
{

/**
 */
RTCommandRunner::RTCommandRunner(RTCommandRunnerStash* stash)
:   stash_(stash)
{
}

/**
 */
RTCommandRunner::~RTCommandRunner() = default;

/**
 */
void RTCommandRunner::addCommand(std::unique_ptr<RTCommand>&& cmd)
{
    addCommand_internal(cmd.release());

    //nudge thread if not running
    if (!isRunning())
        start();
}

/**
*/
void RTCommandRunner::addCommand_internal(RTCommand* cmd)
{
    commands_.push(std::shared_ptr<RTCommand>(cmd));
}

/**
 */
void RTCommandRunner::addCommands(RTCommandChain&& cmds)
{
    while (auto cmd = cmds.pop())
        addCommand_internal(cmd.release());

    //nudge thread if not running
    if (!isRunning())
        start();
}

/**
 */
int RTCommandRunner::numCommands() const
{
    return (int)commands_.unsafe_size();
}

/**
 * Inits the wait condition before running the command.
 */
bool RTCommandRunner::initWaitCondition(RTCommand* cmd)
{
    

    if (!cmd || !stash_)    
        throw std::runtime_error("RTCommandRunner::initWaitCondition: Bad init");

    const auto& c = cmd->condition;

    if (c.type == RTCommandWaitCondition::Type::Signal)
    {
        //register signal spy
        bool ok      = false;
        bool invoked = QMetaObject::invokeMethod(stash_, "spyForSignal", Qt::BlockingQueuedConnection,
                                                 Q_RETURN_ARG(bool, ok),
                                                 Q_ARG(QString, c.obj),
                                                 Q_ARG(QString, c.value));
        if (!invoked || !ok)
        {
            cmd->res.wc_state = RTCommandResult::WaitConditionState::BadInit;
            return false;
        }   
    }

    return true;
}

/**
 * Executes the wait condition after running the command.
 */
bool RTCommandRunner::execWaitCondition(RTCommand* cmd)
{
    if (!cmd || !stash_)
        throw std::runtime_error("RTCommandRunner::execWaitCondition: Bad init");

    const auto& c = cmd->condition;

    bool ok = true;

    if (c.type == RTCommandWaitCondition::Type::Signal)
    {
        ok = waitForCondition([ = ] () { return stash_->spySignalReceived(); }, c.timeout_ms);
    }
    else if (c.type == RTCommandWaitCondition::Type::Delay)
    {
        ok = waitForCondition(WaitConditionDelay(c.timeout_ms));
    }

    cmd->res.wc_state = ok ? RTCommandResult::WaitConditionState::Success : 
                             RTCommandResult::WaitConditionState::Failed;
    return ok;
}

/**
 * Cleans up after wait condition.
 */
bool RTCommandRunner::cleanupWaitCondition(RTCommand* cmd)
{
    if (!cmd || !stash_)
        throw std::runtime_error("RTCommandRunner::cleanupWaitCondition: Bad init");

    const auto& c = cmd->condition;

    if (c.type == RTCommandWaitCondition::Type::Signal)
    {
        if (!QMetaObject::invokeMethod(stash_, "removeSpy", Qt::BlockingQueuedConnection))
            return false;
    }

    return true;
}

/**
 */
bool RTCommandRunner::executeCommand(RTCommand* cmd)
{
    if (!cmd || !stash_)
        throw std::runtime_error("RTCommandRunner::executeCommand: Bad init");

    logMsg("Executing...", cmd);

    bool ok      = false;
    bool invoked = QMetaObject::invokeMethod(stash_, "executeCommand", Qt::BlockingQueuedConnection,
                                             Q_RETURN_ARG(bool, ok),
                                             Q_ARG(const RTCommand*, cmd));
    bool succeeded = (ok && invoked);

    if (!invoked)
        cmd->res.cmd_state = RTCommandResult::CmdState::Failed;

    logMsg(std::string("[") + (succeeded ? "Succeded" : "Failed") + "]", cmd);

    return succeeded;
}

/**
 */
void RTCommandRunner::logMsg(const std::string& msg, RTCommand* cmd)
{
    std::string prefix = (cmd ? "Command '" + cmd->name().toStdString() + "': " : "");

    std::cout << " ---------------------------------------------------------------" << std::endl;
    std::cout << "| " << prefix << msg << std::endl;
    std::cout << " ---------------------------------------------------------------" << std::endl;
}

/**
 */
void RTCommandRunner::run()
{
    if (!stash_)
        throw std::runtime_error("RTCommandRunner::run: No stash");

    auto processCommand = [ & ] (RTCommand* cmd)
    {
        //reset state
        cmd->res = {};

        //init wait condition and execute command
        if (initWaitCondition(cmd) &&
            executeCommand(cmd))
        {
            // if execution went well -> execute wait condition
            execWaitCondition(cmd);
        }

        //always try to clean up wait condition
        cleanupWaitCondition(cmd);

        logMsg(cmd->result().generateMessage().toStdString(), cmd);
    };

    while (!commands_.empty())
    {
        std::shared_ptr<RTCommand> cmd;
        if (!commands_.try_pop(cmd))
            continue;

        auto cmd_ptr = cmd.get();

        processCommand(cmd_ptr);
    }
}

} // namespace rtcommand
