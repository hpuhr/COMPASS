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

#include "ui_test_cmd.h"
#include "ui_test_setget.h"
#include "rtcommand_registry.h"
#include "json.h"

#include <boost/program_options.hpp>

#include <QMainWindow>
#include <QDialog>

REGISTER_RTCOMMAND(ui_test::RTCommandUISet)
REGISTER_RTCOMMAND(ui_test::RTCommandUIGet)

namespace ui_test
{

/***************************************************************************************
 * RTCommandObject
 ***************************************************************************************/

/**
 */
void RTCommandUIObject::collectOptions_impl(OptionsDescription& options,
                                            PosOptionsDescription& positional)
{
    //add basic command options here
    ADD_RTCOMMAND_OPTIONS(options)
        ("object,o", po::value<std::string>()->default_value(""), "name of an ui element, object names separated by '.', e.g. 'mainwindow.window1.osgview1.toolbar'");

    ADD_RTCOMMAND_POS_OPTION(positional, "object", 1)
}

/**
 */
void RTCommandUIObject::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_QSTRING_OR_THROW(variables, "object", obj)
}

/*************************************************************************
 * RTCommandUIInjection
 *************************************************************************/

/**
 */
void RTCommandUIInjection::collectOptions_impl(OptionsDescription& options,
                                               PosOptionsDescription& positional)
{
    //call base
    RTCommandUIObject::collectOptions_impl(options, positional);

    ADD_RTCOMMAND_OPTIONS(options)
        ("uidelay", po::value<int>()->default_value(-1), "delay added after each injected ui event");
}

/**
 */
void RTCommandUIInjection::assignVariables_impl(const VariablesMap& variables)
{
    //call base
    RTCommandUIObject::assignVariables_impl(variables);

    RTCOMMAND_GET_VAR_OR_THROW(variables, "uidelay", int, injection_delay)
}

/*************************************************************************
 * RTCommandUISet
 *************************************************************************/

/**
 */
bool RTCommandUISet::run_impl() const
{
    auto receiver = rtcommand::getCommandReceiverAs<QWidget>(obj.toStdString());
    if (receiver.first != rtcommand::FindObjectErrCode::NoError)
    {
        setResultMessage("Object '" + obj.toStdString() + "' not found");
        return false;
    }

    bool ok = setUIElement(receiver.second, "", value, injection_delay);
    if (!ok)
    {
        setResultMessage("Value '" + value.toStdString() + "' could not be set in object '" + obj.toStdString() + "'");
        return false;
    }

    return true;
}

/**
 */
void RTCommandUISet::collectOptions_impl(OptionsDescription& options,
                                         PosOptionsDescription& positional)
{
    //call base
    RTCommandUIInjection::collectOptions_impl(options, positional);

    ADD_RTCOMMAND_OPTIONS(options)
        ("value,v", po::value<std::string>()->default_value(""), "new value to set, content depending on the addressed ui element");

    ADD_RTCOMMAND_POS_OPTION(positional, "value", 2)
}

/**
 */
void RTCommandUISet::assignVariables_impl(const VariablesMap& variables)
{
    //call base
    RTCommandUIInjection::assignVariables_impl(variables);

    RTCOMMAND_GET_QSTRING_OR_THROW(variables, "value", value)
}

/*************************************************************************
 * RTCommandUIGet
 *************************************************************************/

/**
 */
bool RTCommandUIGet::run_impl() const
{
    auto receiver = rtcommand::getCommandReceiverAs<QWidget>(obj.toStdString());
    if (receiver.first != rtcommand::FindObjectErrCode::NoError)
    {
        setResultMessage("Object '" + obj.toStdString() + "' not found");
        return false;
    }
    
    auto res = getUIElement(receiver.second, "", what);
    if (!res.has_value())
    {
        setResultMessage("Value could not be retrieved from object '" + obj.toStdString() + "'");
        return false;
    }

    const std::string value = res.value().toStdString();

    nlohmann::json result;
    result[ "value" ] = value;
    setJSONReply(result, value);

    return true;
}

/**
 */
void RTCommandUIGet::collectOptions_impl(OptionsDescription& options,
                                         PosOptionsDescription& positional)
{
    //call base
    RTCommandUIObject::collectOptions_impl(options, positional);

    ADD_RTCOMMAND_OPTIONS(options)
        ("what,w", po::value<std::string>()->default_value(""), "which value to retrieve from the ui element (empty = default behavior)");

    ADD_RTCOMMAND_POS_OPTION(positional, "what", 2)
}

/**
 */
void RTCommandUIGet::assignVariables_impl(const VariablesMap& variables)
{
    //call base
    RTCommandUIObject::assignVariables_impl(variables);

    RTCOMMAND_GET_QSTRING_OR_THROW(variables, "what", what)
}

}
