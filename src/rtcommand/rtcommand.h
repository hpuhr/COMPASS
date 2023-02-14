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
#include "rtcommand_result.h"
#include "rtcommand_macros.h"

#include <QString>
#include <QVariant>

#include <memory>
#include <vector>

#include "json.h"

class QObject;
class QMainWindow;
class QDialog;

namespace boost
{
    namespace program_options
    {
        class options_description;
        class positional_options_description;
        class variables_map;
    }
}

namespace rtcommand
{

class WaitCondition;
class RTCommandString;

QMainWindow* mainWindow();
QDialog* activeDialog();
std::pair<FindObjectErrCode, QObject*> getCommandReceiver(const std::string& object_path); // mainwindow.osgview1, dialog.obj2, compass.child1

/**
 * Obtains the command receiver with the given path casted to T*.
 */
template <typename T>
inline std::pair<FindObjectErrCode, T*> getCommandReceiverAs(const std::string& object_path)
{
    auto obj = getCommandReceiver(object_path);
    if (obj.first != FindObjectErrCode::NoError)
        return std::make_pair(obj.first, nullptr);

    auto obj_cast = dynamic_cast<T*>(obj.second);
    if (!obj_cast)
        return std::make_pair(FindObjectErrCode::WrongType, nullptr);

    return std::make_pair(FindObjectErrCode::NoError, obj_cast);
}

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
    typedef boost::program_options::options_description            OptionsDescription;
    typedef boost::program_options::positional_options_description PosOptionsDescription;
    typedef boost::program_options::variables_map                  VariablesMap;

    RTCommand();
    virtual ~RTCommand();

    bool run() const;
    bool checkResult() const;
    QString name() const { return name_impl(); };
    QString description() const { return description_impl(); }

    virtual IsValid valid() const
    { 
        //the command name must not be empty
        CHECK_RTCOMMAND_INVALID_CONDITION(name().isEmpty(), "Command obtains no valid name")
        return true;
    };

    CmdState state() const { return state_; }
    const RTCommandResult& result() const;

    bool isFinished() const;

    bool isConfigured() const;
    bool checkConfiguration() const;
    bool configure(const RTCommandString& cmd);

    bool collectOptions(OptionsDescription& options,
                        PosOptionsDescription& positional,
                        QString* err_msg = nullptr);

    RTCommandWaitCondition condition;             //condition to wait for after executing the command.
    bool                   execute_async = false; //if true execution will immediately return after deploying the command to the main thread's event loop,
                                                  //if false execution will wait for the command to finish running in the main thread

    static const std::string HelpOptionFull;
    static const std::string HelpOptionShort;
    static const std::string HelpOption;
    static const std::string HelpOptionCmdFull;
    static const std::string HelpOptionCmdShort;

protected:
    void setResultMessage(const std::string& m) const;
    void setJSONReply(const nlohmann::json& json_reply, const std::string& reply_as_string = "") const;

    //implements command specific behaviour
    virtual bool run_impl() const = 0;
    virtual bool checkResult_impl() const { return true; }
    virtual void collectOptions_impl(OptionsDescription& options, 
                                     PosOptionsDescription& positional) = 0;
    virtual void assignVariables_impl(const VariablesMap& variables) = 0;

    //!implemented by DECLARE_RTCOMMAND macro!
    virtual QString name_impl() const = 0;
    virtual QString description_impl() const = 0;

    static const std::string ReplyStringIndentation;

private:
    friend class RTCommandRunner;

    void setState(CmdState state) const;
    void setError(CmdErrorCode code, boost::optional<std::string> msg = {}) const;

    bool assignVariables(const boost::program_options::variables_map& variables,
                         QString* err_msg = nullptr);

    void resetResult() const;

    mutable CmdState        state_;  //state the command is in
    mutable RTCommandResult result_; //command result struct containing error info and command result data
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

/**
 * The help command.
 */
struct RTCommandHelp : public RTCommand 
{
    QString command;
protected:
    virtual bool run_impl() const override;

    DECLARE_RTCOMMAND(help, "the help command, generates help information")
    DECLARE_RTCOMMAND_OPTIONS
};

} // namespace rtcommand
