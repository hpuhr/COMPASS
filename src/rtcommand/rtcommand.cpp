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

#include "rtcommand.h"
#include "rtcommand_wait_condition.h"
#include "rtcommand_registry.h"
#include "rtcommand_string.h"
#include "stringconv.h"
#include "ui_test_find.h"
#include "compass.h"
#include "viewmanager.h"
#include "viewcontainerwidget.h"

#include "logger.h"

#include <iostream>

#include <boost/program_options.hpp>
#include <boost/optional.hpp>

#include <QObject>
#include <QMainWindow>
#include <QApplication>
#include <QWindow>
#include <QDialog>

REGISTER_RTCOMMAND(rtcommand::RTCommandEmpty)
REGISTER_RTCOMMAND(rtcommand::RTCommandHelp)

using namespace Utils;

namespace rtcommand
{

    /***************************************************************************************
     * Helpers
     ***************************************************************************************/

    /**
     * Return the application's main window.
     */
    QMainWindow *mainWindow()
    {
        for (auto win : qApp->topLevelWidgets())
        {
            auto mw = dynamic_cast<QMainWindow *>(win);
            if (mw)
                return mw;
        }
        return nullptr;
    }

    /**
     * Returns the currently active modal dialog.
     */
    QDialog *activeDialog()
    {
        QWidget *w = qApp->activeModalWidget();
        if (!w)
            return nullptr;

        QDialog *dlg = dynamic_cast<QDialog *>(w);
        if (!dlg)
            return nullptr;

        return dlg;
    }

    /**
     * Finds the QObject described by the given object path.
     * Unifies Configurables and UI elements (e.g. QWidget's).
     */
    std::pair<rtcommand::FindObjectErrCode, QObject *> getCommandReceiver(const std::string &object_path)
    {
        std::vector<std::string> parts = String::split(object_path, '.');

        if (!parts.size())
            return {rtcommand::FindObjectErrCode::NotFound, nullptr};

        std::string first_part = parts.at(0);
        parts.erase(parts.begin());
        std::string remainder = String::compress(parts, '.');

        if (first_part == "mainwindow")
        {
            return ui_test::findObject(mainWindow(), remainder.c_str());
        }
        else if (first_part.find("window") == 0)
        {
            //HACK, ViewContainerWidget could be a non-modal QDialog instead of a free-floating QWidget
            QString num = QString::fromStdString(first_part).remove("window");

            bool ok;
            int idx = num.toInt(&ok);

            if (ok)
            {
                QString view_container_name = "ViewWindow" + num;

                auto container = COMPASS::instance().viewManager().containerWidget(view_container_name.toStdString());
                if (!container)
                    return std::make_pair(FindObjectErrCode::NotFound, nullptr);

                return ui_test::findObject(container, remainder.c_str());
            }
        }
        else if (first_part == "dialog")
        {
            return ui_test::findObject(activeDialog(), remainder.c_str());
        }   
        else if (first_part == "compass")
        {
            std::pair<rtcommand::FindObjectErrCode, Configurable *> ret = COMPASS::instance().findSubConfigurable(remainder);

            QObject *obj_casted = dynamic_cast<QObject *>(ret.second);

            if (!obj_casted)
                return {rtcommand::FindObjectErrCode::WrongType, nullptr};

            return {rtcommand::FindObjectErrCode::NoError, obj_casted};
        }

        return {rtcommand::FindObjectErrCode::NotFound, nullptr};
    }

    /***************************************************************************************
     * RTCommandWaitCondition
     ***************************************************************************************/

    /**
     * Configures the wait condition as 'signal' type.
     */
    void RTCommandWaitCondition::setSignal(const QString &obj_name,
                                           const QString &signal_name,
                                           int timeout_in_ms)
    {
        *this = {};
        type = Type::Signal;
        obj = obj_name;
        value = signal_name;
        timeout_ms = timeout_in_ms;
    }

    /**
     * Configures the wait condition as 'delay' type.
     */
    void RTCommandWaitCondition::setDelay(int ms)
    {
        *this = {};
        type = Type::Delay;
        timeout_ms = ms;
    }

    /**
     * Configures the wait condition from the given config string.
     *
     * Either:
     * "signal;obj_name;obj_signal_name;timeout" OR
     * "delay;timeout"
     */
    bool RTCommandWaitCondition::setFromString(const QString &config_string)
    {
        QStringList parts = config_string.split(";");

        if (parts.count() < 1 || parts[0].isEmpty())
            return false;

        if (parts[0] == "signal")
        {
            if (parts.count() != 4)
                return false;

            obj = parts[1];
            value = parts[2];

            bool ok;
            timeout_ms = parts[3].toInt(&ok);

            if (obj.isEmpty() || value.isEmpty() || !ok)
                return false;

            type = Type::Signal;

            return true;
        }
        else if (parts[0] == "delay")
        {
            if (parts.count() != 2)
                return false;

            bool ok;
            timeout_ms = parts[1].toInt(&ok);

            if (!ok)
                return false;

            type = Type::Delay;

            return true;
        }
        return false;
    }

    /***************************************************************************************
     * RTCommand
     ***************************************************************************************/

    const std::string RTCommand::HelpOptionFull = "help";
    const std::string RTCommand::HelpOptionShort = "h";
    const std::string RTCommand::HelpOption = HelpOptionFull + "," + HelpOptionShort;
    const std::string RTCommand::HelpOptionCmdFull = "--" + HelpOptionFull;
    const std::string RTCommand::HelpOptionCmdShort = "-" + HelpOptionShort;

    const std::string RTCommand::ReplyStringIndentation = "   ";

    /**
     */
    RTCommand::RTCommand() = default;

    /**
     */
    RTCommand::~RTCommand() = default;

    /**
     */
    void RTCommand::resetResult() const 
    { 
        result_.reset();
    }

    /**
     */
    const RTCommandResult& RTCommand::result() const
    { 
        //update command name
        result_.command = name().toStdString();

        return result_;
    }

    /**
     * Collects command option descriptions throughout the class hierarchy.
     */
    bool RTCommand::collectOptions(OptionsDescription &options,
                                   PosOptionsDescription &positional,
                                   QString* err_msg)
    {
        try
        {
            // add basic command options here
            ADD_RTCOMMAND_OPTIONS(options)
            ("wait_condition", po::value<std::string>()->default_value(""), "wait condition config string")
            ("async", "enables asynchrous command execution, meaning execution will return immediately after the command has been deployed to the main thread")
            (HelpOption.c_str(), "show command help information");

            // collect from derived
            collectOptions_impl(options, positional);
        }
        catch (const std::exception &ex)
        {
            logerr << "RTCommand::collectOptions(): Error: " << ex.what();
            if (err_msg) *err_msg = QString(ex.what());
            return false;
        }
        catch (...)
        {
            logerr << "RTCommand::collectOptions(): Unknown error";
            if (err_msg) *err_msg = "Unknown error";
            return false;
        }

        return true;
    }

    /**
     * Assigns scanned variables to struct data throughout the class hierarchy.
     */
    bool RTCommand::assignVariables(const boost::program_options::variables_map &variables,
                                    QString* err_msg)
    {
        try
        {
            // assign basic options here
            QString condition_config_str;
            RTCOMMAND_GET_QSTRING_OR_THROW(variables, "wait_condition", condition_config_str)

            if (!condition_config_str.isEmpty() &&
                !condition.setFromString(condition_config_str))
                throw("Could not configure condition");

            RTCOMMAND_CHECK_VAR(variables, "async", execute_async)

            // assign in derived
            assignVariables_impl(variables);
        }
        catch (const std::exception &ex)
        {
            logerr << "RTCommand::assignVariables(): Error: " << ex.what();
            if (err_msg) *err_msg = QString(ex.what());
            return false;
        }
        catch (...)
        {
            logerr << "RTCommand::assignVariables(): Unknown error";
            if (err_msg) *err_msg = "Unknown error";
            return false;
        }

        return true;
    }

    /**
    */
    void RTCommand::setResultMessage(const std::string& m) const 
    { 
        result_.error.message = m; 
    }

    /**
    */
    void RTCommand::setJSONReply(const nlohmann::json& json_reply, const std::string& reply_as_string) const
    { 
        result_.json_reply        = json_reply;
        result_.json_reply_string = reply_as_string;
    }

    /**
    */
    void RTCommand::setState(CmdState state) const
    {
        state_ = state;
    }

    /**
    */
    void RTCommand::setError(CmdErrorCode code, boost::optional<std::string> msg) const
    {
        result_.error.code = code;
        if (msg.has_value())
            result_.error.message = msg.value();
    }

    /**
     * Run the command and track state.
     */
    bool RTCommand::run() const
    {
        try
        {
            if (result().hasError())
                return false;

            //command should be in configured state
            if (state() != CmdState::Configured)
            {
                setError(CmdErrorCode::Exec_Unconfigured);
                return false;
            }

            // check again: command configuration valid?
            auto valid_state = valid();
            if (!valid_state.is_valid)
            {
                setError(CmdErrorCode::Exec_InvalidConfig, valid_state.errorString());
                return false;
            }

            // run command
            if (!run_impl())
            {
                setError(CmdErrorCode::Exec_Failed);
                return false;
            }
        }
        catch (const std::exception& ex)
        {
            setError(CmdErrorCode::Exec_Crash, std::string(ex.what()));
            return false;
        }
        catch (...)
        {
            setError(CmdErrorCode::Exec_Crash, std::string("Unexpected crash"));
            return false;
        }

        return true;
    }

    /**
     */
    bool RTCommand::checkResult() const
    {
        try
        {
            if (result().hasError())
                return false;

            //command should be in executed state
            if (state() != CmdState::Executed)
            {
                setError(CmdErrorCode::ResultCheck_NotExecuted);
                return false;
            }

            // run check
            if (!checkResult_impl())
            {
                setError(CmdErrorCode::ResultCheck_InvalidResult);
                return false;
            }
        }
        catch (const std::exception& ex)
        {
            setError(CmdErrorCode::ResultCheck_Crash, std::string(ex.what()));
            return false;
        }
        catch (...)
        {
            setError(CmdErrorCode::ResultCheck_Crash, std::string("Unexpected crash"));
            return false;
        }

        return true;
    }

    /**
     * Checks if the command in finished state.
    */
    bool RTCommand::isFinished() const
    {
        return (state() == CmdState::Finished);
    }

    /**
     * Checks if the command in configured state.
    */
    bool RTCommand::isConfigured() const
    {
        return (state() == CmdState::Configured);
    }

    /**
     * Checks configuration validity, and sets the commands state accordingly.
     */
    bool RTCommand::checkConfiguration() const
    {
        auto valid_state = valid();
        if (!valid_state.is_valid)
        {
            setError(CmdErrorCode::Config_Invalid, valid_state.errorString());
            return false;
        }

        setState(CmdState::Configured);
        return true;
    }

    /**
     * Configure the command struct using the given command string.
     */
    bool RTCommand::configure(const RTCommandString &cmd)
    {
        setState(CmdState::Unconfigured);

        if (!cmd.valid())
        {
            // std::cout << "RTCommand::configure: Passed command not valid" << std::endl;
            setError(CmdErrorCode::Config_CommandStringInvalid);
            return false;
        }

        // does the command even concern me?
        if (name() != cmd.cmdName())
        {
            // std::cout << "RTCommand::configure: Name '" << name().toStdString()
            //           << "' does not match command name '" << cmd.cmdName().toStdString() << "'" << std::endl;
            setError(CmdErrorCode::Config_CommandStringMismatch);
            return false;
        }

        QString err_msg;

        // collect options description
        namespace po = boost::program_options;
        OptionsDescription od;
        PosOptionsDescription pod;
        if (!collectOptions(od, pod, &err_msg))
        {
            // std::cout << "RTCommand::configure: Could not collect options" << std::endl;
            setError(CmdErrorCode::Config_CollectOptionsFailed, err_msg.toStdString());
            return false;
        }

        // parse command using collected options description
        po::variables_map vm;
        QString parse_err;
        if (!cmd.parse(vm, od, pod, true, &parse_err))
        {
            // std::cout << "RTCommand::configure: Could not parse command" << std::endl;
            setError(CmdErrorCode::Config_ParseOptionsFailed, err_msg.toStdString());
            return false;
        }

        // store parsed variables
        if (!assignVariables(vm, &err_msg))
        {
            // std::cout << "RTCommand::configure: Could not assign vars" << std::endl;
            setError(CmdErrorCode::Config_AssignOptionsFailed, err_msg.toStdString());
            return false;
        }

        //finally, check parsed configuration (also handles state)
        return checkConfiguration();
    }

    /***************************************************************************************
     * RTCommandHelp
     ***************************************************************************************/

    /**
     */
    bool RTCommandHelp::run_impl() const
    {
        nlohmann::json root;
        std::string    str;

        if (command.isEmpty())
        {
            const auto &cmds = RTCommandRegistry::instance().availableCommands();

            for (const auto &elem : cmds)
            {
                nlohmann::json entry;
                entry[ "command_name"        ] = elem.first.toStdString();
                entry[ "command_description" ] = elem.second.description.toStdString();

                str += elem.first.toStdString() + "\n";
                str += ReplyStringIndentation + elem.second.description.toStdString() + "\n";
                str += "\n";

                root.push_back(entry);
            }
        }
        else
        {
            if (!RTCommandRegistry::instance().hasCommand(command))
            {
                setResultMessage("Command '" + command.toStdString() + "' not registered");
                return false;
            }

            auto cmd = RTCommandRegistry::instance().createCommandTemplate(command);
            if (!cmd)
            {
                setResultMessage("Command '" + command.toStdString() + "' could not be created");
                return false;
            }

            boost::program_options::options_description options;
            boost::program_options::positional_options_description p_options;
            QString err_msg;
            if (!cmd->collectOptions(options, p_options, &err_msg))
            {
                setResultMessage(err_msg.toStdString());
                return false;
            }

            root[ "command_name"        ] = cmd->name().toStdString();
            root[ "command_description" ] = cmd->description().toStdString();

            str += cmd->description().toStdString() + "\n";
            str += "\n";
            
            nlohmann::json option_node;

            for (const auto &o : options.options())
            {
                nlohmann::json option;
                option[ "option_name"        ] = o->long_name();
                option[ "option_name_format" ] = o->format_name();
                option[ "option_description" ] = o->description();

                str += ReplyStringIndentation + o->long_name() + " " + o->format_name() + "\n";
                str += "\n";
                str += ReplyStringIndentation + ReplyStringIndentation + o->description() + "\n";
                str += "\n";

                option_node.push_back(option);
            }

            root[ "command_options" ] = option_node;
        }

        setJSONReply(root, str);

        return true;
    }

    /**
     */
    void RTCommandHelp::collectOptions_impl(OptionsDescription &options,
                                            PosOptionsDescription &positional)
    {
        ADD_RTCOMMAND_OPTIONS(options)
        ("command", po::value<std::string>()->default_value(""), "command to retrieve help information for");

        ADD_RTCOMMAND_POS_OPTION(positional, "command", 1)
    }

    /**
     */
    void RTCommandHelp::assignVariables_impl(const VariablesMap &variables)
    {
        RTCOMMAND_GET_QSTRING_OR_THROW(variables, "command", command)
    }

} // namespace rtcommand
