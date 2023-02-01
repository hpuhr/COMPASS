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

#include "singleton.h"
#include "rtcommand.h"

#include <map>
#include <functional>
#include <iostream>

#include <QString>

namespace rtcommand
{

/**
*/
class RTCommandRegistry : public Singleton
{
public:
    typedef std::function<RTCommand*()> CreatorFunc;

    virtual ~RTCommandRegistry() = default;

    static RTCommandRegistry& instance()
    {
        static RTCommandRegistry instance;
        return instance;
    }

    bool registerCommand(const QString& name, CreatorFunc func)
    {
        std::cout << "Registering command '" << name.toStdString() << "'" << std::endl;
        return commands_.insert(std::make_pair(name, func)).second;
    }

protected:
    RTCommandRegistry() = default;

    std::map<QString, CreatorFunc> commands_;
};

template <typename T>
struct RTCommandRegistrator
{
    RTCommandRegistrator(const QString& name)
    {
        std::cout << "RTCommandRegistrator: '" << name.toStdString() << "'" << std::endl;
        RTCommandRegistry::instance().registerCommand(name, [] () { return new T; });
    };
};

} // namespace rtcommand
