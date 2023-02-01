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

#include "rtcommand_defs.h"

#include <QString>
#include <QVariant>

#include <memory>
#include <vector>

class QObject;
class QMainWindow;

#define DECLARE_RTCOMMAND(Name)                                         \
public:                                                                 \
    static QString staticName() { return #Name; }                       \
    static void init() { is_registered_ = true; }                       \
protected:                                                              \
    virtual QString name_impl() const override { return staticName(); } \
private:                                                                \
    static bool is_registered_;

#define REGISTER_RTCOMMAND(Class) \
    bool Class::is_registered_ = rtcommand::RTCommandRegistry::instance().registerCommand(Class::staticName(), [] () { return new Class; });

namespace rtcommand
{

class WaitCondition;

QMainWindow* mainWindow();

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

    void setSignal(const QString& obj_name, const QString& signal_name, int timeout_in_ms = -1);
    void setDelay(int ms);

    Type    type = Type::None; //type of wait condition
    QString obj;               //QObject name
    QString value;             //string value for the condition, e.g. a signal name
    int     timeout_ms = -1;   //Type::Signal: timeout if the signal wasn't received
                               //Type::Delay:  amount of time to wait
};

/**
 * Base for all runtime commands.
 */
struct RTCommand
{
    bool run() const;
    virtual QString name() const { return name_impl(); };
    virtual QString description() const { return ""; }

    virtual bool valid() const 
    { 
        //the command name must not be empty
        return !name().isEmpty(); 
    };

    int                    delay = -1; //delay used during command execution (e.g. for each UI injection)
    RTCommandWaitCondition condition;  //condition to wait for after executing the command. 

    const RTCommandResult& result() const { return result_; };

protected:
    void setResultData(const QString& d) const { result_.data = d; }
    void setResultMessage(const QString& m) const { result_.cmd_msg = m; }

    virtual bool run_impl() const = 0;
    virtual QString name_impl() const = 0;

private:
    friend class RTCommandRunner;

    void resetResult() const { result_.reset(); }

    mutable RTCommandResult result_;
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
    DECLARE_RTCOMMAND(empty)
protected:
    virtual bool run_impl() const override { return true; } 
};

/**
*/
struct RTCommandTestInit : public RTCommand 
{
    DECLARE_RTCOMMAND(test_init)
protected:
    virtual bool run_impl() const override { return true; } 
};

/**
*/
struct RTCommandTestRegistrator : public RTCommand 
{
    DECLARE_RTCOMMAND(test_registrator)
protected:
    virtual bool run_impl() const override { return true; } 
};

} // namespace rtcommand
