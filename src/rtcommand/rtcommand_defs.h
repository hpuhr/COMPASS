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

#include "json.h"

namespace rtcommand
{

/**
 * The execution state a command can be in.
 */
enum class CmdState
{
    Fresh = 0,
    BadConfig,
    ExecFailed,
    ResultCheckFailed,
    Executed,
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
 * Describes a runtime command.
*/
struct RTCommandDescription
{
    QString name;
    QString description;
};


enum class FindObjectErrCode
{
    NoError = 0,
    Invalid,
    NotFound,
    WrongType
};


} // namespace rtcommand
