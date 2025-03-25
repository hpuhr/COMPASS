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

#include "configurable.h"

#include <QObject>

class COMPASS;

/**
 */
class ResultManager : public QObject, public Configurable
{
    Q_OBJECT

signals:
    void sectorsChangedSignal();

public:
    ResultManager(const std::string& class_id,
                  const std::string& instance_id,
                  COMPASS* parent);
    virtual ~ResultManager();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override {}

protected:
    virtual void checkSubConfigurables() override {}
};
