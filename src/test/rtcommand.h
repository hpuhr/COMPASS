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
struct RTCommand
{
    bool run() const;
    virtual bool valid() const = 0;
    virtual QString name() const = 0;
    virtual QString description() const { return ""; }

    int                     delay = -1;
    RTCommandWaitConditions conditions;

protected:
    virtual bool run_impl() const = 0;
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
