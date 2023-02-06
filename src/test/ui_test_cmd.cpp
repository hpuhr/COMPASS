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

const std::string RTCommandUIObject::ParentMainWindowString  = "mainwindow";
const std::string RTCommandUIObject::ParentModalDialogString = "dialog";

/**
 */
void RTCommandUIObject::collectOptions_impl(OptionsDescription& options)
{
    //add basic command options here
    ADD_RTCOMMAND_OPTIONS(options)
        ("object,o", po::value<std::string>()->default_value(""), "name of an ui object")
        ("parent", po::value<std::string>()->default_value(ParentMainWindowString), "parent ui object");
}

/**
 */
void RTCommandUIObject::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_QSTRING_OR_THROW(variables, "object", obj)

    std::string parent_str;
    RTCOMMAND_GET_VAR_OR_THROW(variables, "parent", std::string, parent_str)
    parent = parentFromString(parent_str);
}

/**
 */
RTCommandUIObject::Parent RTCommandUIObject::parentFromString(const std::string& str)
{
    if (str == ParentMainWindowString)
        return Parent::MainWindow;
    else if (str == ParentModalDialogString)
        return Parent::ModalDialog;

    throw ("Unknown string");
    return Parent::MainWindow;
}

/**
*/
QWidget* RTCommandUIObject::parentWidget() const
{
    if (parent == Parent::MainWindow)
        return rtcommand::mainWindow();
    else if (parent == Parent::ModalDialog)
        return rtcommand::activeDialog();

    return nullptr;
}

/*************************************************************************
 * RTCommandUIInjection
 *************************************************************************/

/**
 */
void RTCommandUIInjection::collectOptions_impl(OptionsDescription& options)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("uidelay", po::value<int>()->default_value(-1), "delay added after injected ui events");

    //call base
    RTCommandUIObject::collectOptions_impl(options);
}

/**
 */
void RTCommandUIInjection::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "uidelay", int, injection_delay)

    //call base
    RTCommandUIObject::assignVariables_impl(variables);
}

/*************************************************************************
 * RTCommandUISet
 *************************************************************************/

/**
 */
bool RTCommandUISet::run_impl() const
{
    QWidget* parent = parentWidget();
    if (!parent)
        return false;

    return setUIElement(parent, obj, value, injection_delay);
}

/**
 */
void RTCommandUISet::collectOptions_impl(OptionsDescription& options)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("value,v", po::value<std::string>()->default_value(""), "new value to set");

    //call base
    RTCommandUIInjection::collectOptions_impl(options);
}

/**
 */
void RTCommandUISet::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_QSTRING_OR_THROW(variables, "value", value)

    //call base
    RTCommandUIInjection::assignVariables_impl(variables);
}

/*************************************************************************
 * RTCommandUIGet
 *************************************************************************/

/**
 */
bool RTCommandUIGet::run_impl() const
{
    QWidget* parent = parentWidget();
    if (!parent)
        return false;

    auto res = getUIElement(parent, obj, what);
    if (!res.has_value())
        return false;

    setResultData(res.value());

    return true;
}

/**
 */
void RTCommandUIGet::collectOptions_impl(OptionsDescription& options)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("what,w", po::value<std::string>()->default_value(""), "which value to retrieve from the ui element (empty = default behavior)");

    //call base
    RTCommandUIObject::collectOptions_impl(options);
}

/**
 */
void RTCommandUIGet::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_QSTRING_OR_THROW(variables, "what", what)

    //call base
    RTCommandUIObject::assignVariables_impl(variables);
}

}
