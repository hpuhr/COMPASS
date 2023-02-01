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

#include <QMainWindow>

REGISTER_RTCOMMAND(ui_test::RTCommandUISet)
REGISTER_RTCOMMAND(ui_test::RTCommandUIGet)

namespace ui_test
{

/**
 */
bool RTCommandUISet::run_impl() const
{
    auto main_window = rtcommand::mainWindow();
    if (!main_window)
        return false;

    return setUIElement(main_window, obj, value, delay);
}

/**
 */
bool RTCommandUIGet::run_impl() const
{
    auto main_window = rtcommand::mainWindow();
    if (!main_window)
        return false;

    auto res = getUIElement(main_window, obj, value);
    if (!res.has_value())
        return false;

    setResultData(res.value());

    return true;
}

}
