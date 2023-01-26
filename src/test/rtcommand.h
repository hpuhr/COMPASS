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
 */
struct RTCommandWaitCondition
{
    enum class Type
    {
        None = 0,
        Signal,
        Delay
    };

    bool isSet() const 
    {
        return (type != Type::None);
    }

    std::unique_ptr<WaitCondition> create() const;

    Type    type = Type::None;
    QString obj;
    QString value;
    int     timeout_ms = -1;
};

typedef std::vector<RTCommandWaitCondition> RTCommandWaitConditions;

/**
*/
struct RTCommandResult
{
    enum class CmdState
    {
        Fresh = 0,
        BadConfig,
        Failed,
        Success
    };

    enum class WaitConditionState
    {
        Unknown = 0,
        BadInit,
        Failed,
        Success
    };

    bool success() const
    {
        return (wc_state == WaitConditionState::Success && cmd_state == CmdState::Success);
    }

    QString generateMessage() const;

    WaitConditionState wc_state  = WaitConditionState::Unknown;
    CmdState           cmd_state = CmdState::Fresh;
    QString            cmd_msg;
};

/**
 */
struct RTCommand
{
    bool run() const;
    virtual bool valid() const = 0;
    virtual QString name() const = 0;
    virtual QString description() const { return ""; }

    int                    delay = -1;
    RTCommandWaitCondition condition;

    const RTCommandResult& result() const { return res; }

protected:
    virtual bool run_impl() const = 0;

private:
    friend class RTCommandRunner;

    mutable RTCommandResult res;
};

/**
 */
struct RTCommandObject : public RTCommand
{
    virtual bool valid() const override
    {
        return !obj.isEmpty();
    }

    QString obj;
};

/**
 */
struct RTCommandObjectValue : public RTCommandObject
{
    QString value;
};

/**
*/
struct RTCommandEmpty : public RTCommand 
{
    virtual bool valid() const override { return true; }
    virtual QString name() const override { return "empty"; }
protected:
    virtual bool run_impl() const override { return true; }
};

} // namespace rtcommand
