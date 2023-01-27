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

#include <memory>
#include <vector>

class QObject;
class QMainWindow;

namespace rtcommand
{

class WaitCondition;

QMainWindow* mainWindow();

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
 * Represents a wait condition which is evaluated after a command has been executed.
 */
struct RTCommandWaitCondition
{
    enum class Type
    {
        None = 0, //do not wait
        Signal,   //wait for a signal to be emitted by a QObject
        Delay     //wait for a certain amount of time to pass by
    };

    bool isSet() const 
    {
        return (type != Type::None);
    }

    WaitConditionState state() const { return wc_state; }

    std::unique_ptr<WaitCondition> create() const;

    Type    type = Type::None; //type of wait condition
    QString obj;               //QObject name
    QString value;             //string value for the condition, e.g. a signal name
    int     timeout_ms = -1;   //Type::Signal: timeout if the signal wasn't received
                               //Type::Delay:  amount of time to wait
private:
    friend class RTCommandRunner;
    friend struct RTCommand;

    void resetState()
    {
        wc_state = WaitConditionState::Unknown;
    }

    mutable WaitConditionState wc_state = WaitConditionState::Success; // the current execution state
};

/**
 * Base for all runtime commands.
 */
struct RTCommand
{
    bool run() const;
    virtual QString name() const = 0;
    virtual QString description() const { return ""; }

    virtual bool valid() const 
    { 
        //the command name must not be empty
        return !name().isEmpty(); 
    };

    int                    delay = -1; //delay used during command execution (e.g. for each UI injection)
    RTCommandWaitCondition condition;  //condition to wait for after executing the command. 

    CmdState state() const { return cmd_state; }
    const QString& stateMsg() const { return cmd_msg; }

    bool success() const
    {
        //as the wait condition might be extremely important for any upcoming commands, 
        //its state is part of the successful execution of a command
        return (condition.state() == WaitConditionState::Success && state() == CmdState::Success);
    }

    QString generateStateString() const;

protected:
    virtual bool run_impl() const = 0;

private:
    friend class RTCommandRunner;

    void resetState()
    {
        cmd_state = CmdState::Fresh;
        cmd_msg   = "";

        condition.resetState();
    }

    mutable CmdState cmd_state = CmdState::Fresh; // the current execution state
    mutable QString  cmd_msg;                     // optional message for current execution state
};

/**
 * Command targeting a specific QObject.
 */
struct RTCommandObject : public RTCommand
{
    virtual bool valid() const override
    {
        //the object name must not be empty
        return (RTCommand::valid() && !obj.isEmpty());
    }

    QString obj;
};

/**
 * Command targeting a specific QObject with a specific value (e.g. a setter).
 */
struct RTCommandObjectValue : public RTCommandObject
{
    QString value;
};

/**
 * The empty command (can be used e.g. to execute a wait condition only).
*/
struct RTCommandEmpty : public RTCommand 
{
    virtual QString name() const override { return "empty"; }
protected:
    virtual bool run_impl() const override { return true; }
};

} // namespace rtcommand
