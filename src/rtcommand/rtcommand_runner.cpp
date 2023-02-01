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
RTCommandRunner::RTCommandRunner()
:   stash_(new RTCommandRunnerStash)
{
}

/**
 */
RTCommandRunner::~RTCommandRunner() = default;

/**
 * Asynchronously run the given command and return a future containing the execution state.
 */
std::future<RTCommandRunner::Results> RTCommandRunner::runCommand(std::unique_ptr<RTCommand>&& cmd)
{
    RTCommandChain c;
    c.append(std::move(cmd));

    return runCommands(std::move(c));
}

/**
 * Asynchronously run the given commands and return a future containing the execution states.
 */
std::future<RTCommandRunner::Results> RTCommandRunner::runCommands(RTCommandChain&& cmds)
{
    RTCommandRunnerStash* s = stash_.get();

    auto runCommand = [ s ] (RTCommandChain&& cmds_to_run) mutable
    {
        Results results;

        while (auto cmd = cmds_to_run.pop())
        {
            RTCommandRunner::runCommand(cmd.get(), s);
            results.push_back(cmd->result());
        }

        return results;
    };

    return std::async(std::launch::async, runCommand, std::move(cmds));
}

/**
 * Inits the wait condition before running the command.
 */
bool RTCommandRunner::initWaitCondition(RTCommand* cmd, RTCommandRunnerStash* stash)
{
    if (!cmd || !stash)    
        throw std::runtime_error("RTCommandRunner::initWaitCondition: Bad init");

    auto& c = cmd->condition;

    if (c.type == RTCommandWaitCondition::Type::Signal)
    {
        //register signal spy in main thread and block until finished
        bool ok      = false;
        bool invoked = QMetaObject::invokeMethod(stash, "spyForSignal", Qt::BlockingQueuedConnection,
                                                 Q_RETURN_ARG(bool, ok),
                                                 Q_ARG(QString, c.obj),
                                                 Q_ARG(QString, c.value));
        if (!invoked || !ok)
        {
            cmd->result_.wc_state = WaitConditionState::BadInit;
            return false;
        }   
    }

    return true;
}

/**
 * Executes the wait condition after running the command.
 */
bool RTCommandRunner::execWaitCondition(RTCommand* cmd, RTCommandRunnerStash* stash)
{
    if (!cmd || !stash)
        throw std::runtime_error("RTCommandRunner::execWaitCondition: Bad init");

    bool ok = true;

    const auto& c = cmd->condition;

    if (c.type == RTCommandWaitCondition::Type::Signal)
    {
        ok = waitForCondition([ = ] () { return stash->spySignalReceived(); }, c.timeout_ms);
    }
    else if (c.type == RTCommandWaitCondition::Type::Delay)
    {
        ok = waitForCondition(WaitConditionDelay(c.timeout_ms));
    }

    cmd->result_.wc_state = ok ? WaitConditionState::Success : 
                                 WaitConditionState::Failed;
    return ok;
}

/**
 * Cleans up after wait condition.
 */
bool RTCommandRunner::cleanupWaitCondition(RTCommand* cmd, RTCommandRunnerStash* stash)
{
    if (!cmd || !stash)
        throw std::runtime_error("RTCommandRunner::cleanupWaitCondition: Bad init");

    const auto& c = cmd->condition;

    if (c.type == RTCommandWaitCondition::Type::Signal)
    {
        //remove signal spy in main thread and block until finished
        if (!QMetaObject::invokeMethod(stash, "removeSpy", Qt::BlockingQueuedConnection))
            return false;
    }

    return true;
}

/**
 * Executes the given command in the main thread.
 */
bool RTCommandRunner::executeCommand(RTCommand* cmd, RTCommandRunnerStash* stash)
{
    if (!cmd || !stash)
        throw std::runtime_error("RTCommandRunner::executeCommand: Bad init");

    logMsg("Executing...", cmd);

    //execute command in main thread and block until finished
    bool ok      = false;
    bool invoked = QMetaObject::invokeMethod(stash, "executeCommand", Qt::BlockingQueuedConnection,
                                             Q_RETURN_ARG(bool, ok),
                                             Q_ARG(const RTCommand*, cmd));
    bool succeeded = (ok && invoked);

    //if invoking the execution failed, we set the commands state to failed
    if (!invoked)
        cmd->result_.cmd_state = CmdState::Failed;

    logMsg(std::string("[") + (succeeded ? "Succeeded" : "Failed") + "]", cmd);

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
 * Runs the given command in the main thread and handles any set wait conditions.
 */
void RTCommandRunner::runCommand(RTCommand* cmd, RTCommandRunnerStash* stash)
{
    if (!stash)
        throw std::runtime_error("RTCommandRunner::run: No stash");

    //reset state
    cmd->resetResult();

    //init wait condition and execute command
    if (initWaitCondition(cmd, stash) &&
        executeCommand(cmd, stash))
    {
        // if execution went well -> execute wait condition
        execWaitCondition(cmd, stash);
    }

    //always try to clean up wait condition
    cleanupWaitCondition(cmd, stash);

    logMsg("Ended with state '" + cmd->result().toString().toStdString() + "'", cmd);
}

} // namespace rtcommand
