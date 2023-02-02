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

REGISTER_RTCOMMAND(ui_test::RTCommandUISet)
REGISTER_RTCOMMAND(ui_test::RTCommandUIGet)

namespace ui_test
{

/*************************************************************************
 * RTCommandUIInjection
 *************************************************************************/

/**
 */
bool RTCommandUIInjection::collectOptions_impl(OptionsDescription& options)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("uidelay", po::value<int>()->default_value(-1), "delay added after injected ui events");

    //call base
    return RTCommandObject::collectOptions_impl(options);
}

/**
 */
bool RTCommandUIInjection::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_FAIL(variables, "uidelay", int, injection_delay)

    //call base
    return RTCommandObject::assignVariables_impl(variables);
}

/*************************************************************************
 * RTCommandUISet
 *************************************************************************/

/**
 */
bool RTCommandUISet::run_impl() const
{
    auto main_window = rtcommand::mainWindow();
    if (!main_window)
        return false;

    return setUIElement(main_window, obj, value, injection_delay);
}

/**
 */
bool RTCommandUISet::collectOptions_impl(OptionsDescription& options)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("value,v", po::value<std::string>()->default_value(""), "new value to set");

    //call base
    return RTCommandUIInjection::collectOptions_impl(options);
}

/**
 */
bool RTCommandUISet::assignVariables_impl(const VariablesMap& variables)
{
    for (const auto& v : variables)
        std::cout << "   " << v.first << std::endl;

    RTCOMMAND_GET_QSTRING_OR_FAIL(variables, "value", value)

    //call base
    return RTCommandUIInjection::assignVariables_impl(variables);
}

/*************************************************************************
 * RTCommandUIGet
 *************************************************************************/

/**
 */
bool RTCommandUIGet::run_impl() const
{
    auto main_window = rtcommand::mainWindow();
    if (!main_window)
        return false;

    auto res = getUIElement(main_window, obj, what);
    if (!res.has_value())
        return false;

    setResultData(res.value());

    return true;
}


/**
 */
bool RTCommandUIGet::collectOptions_impl(OptionsDescription& options)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("what,w", po::value<std::string>()->default_value(""), "which value to retrieve from the ui element (empty = default behavior)");

    //call base
    return RTCommandObject::collectOptions_impl(options);
}

/**
 */
bool RTCommandUIGet::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_QSTRING_OR_FAIL(variables, "what", what)

    //call base
    return RTCommandObject::assignVariables_impl(variables);
}

}
