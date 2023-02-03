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
#include "rtcommand_macros.h"

#include <QString>
#include <QVariant>

#include <memory>
#include <vector>

class QObject;
class QMainWindow;

namespace boost
{
    namespace program_options
    {
        class options_description;
        class variables_map;
    }
}

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
    bool setFromString(const QString& config_string);

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
    typedef boost::program_options::options_description OptionsDescription;
    typedef boost::program_options::variables_map       VariablesMap;

    RTCommand();
    virtual ~RTCommand();

    bool run() const;
    QString name() const { return name_impl(); };
    QString description() const { return description_impl(); }

    virtual bool valid() const 
    { 
        //the command name must not be empty
        return !name().isEmpty(); 
    };

    const RTCommandResult& result() const { return result_; };

    bool configure(const QString& cmd);

    RTCommandWaitCondition condition;  //condition to wait for after executing the command.

protected:
    void setResultData(const QString& d) const { result_.data = d; }
    void setResultMessage(const QString& m) const { result_.cmd_msg = m; }

    //implements command specific behaviour
    virtual bool run_impl() const = 0;
    virtual bool collectOptions_impl(OptionsDescription& options) = 0;
    virtual bool assignVariables_impl(const VariablesMap& variables) = 0;

    //!implemented by DECLARE_RTCOMMAND macro!
    virtual QString name_impl() const = 0;
    virtual QString description_impl() const = 0;

private:
    friend class RTCommandRunner;

    bool collectOptions(boost::program_options::options_description& options);
    bool assignVariables(const boost::program_options::variables_map& variables);

    void resetResult() const { result_.reset(); }

    mutable RTCommandResult result_; //command result struct containing execution state info and command result data
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

    DECLARE_RTCOMMAND_OPTIONS
};

/**
 * The empty command (can be used e.g. to execute a wait condition only).
*/
struct RTCommandEmpty : public RTCommand 
{
protected:
    virtual bool run_impl() const override { return true; } 

    DECLARE_RTCOMMAND(empty, "the empty command...does...nothing")
    DECLARE_RTCOMMAND_NOOPTIONS
};

} // namespace rtcommand
