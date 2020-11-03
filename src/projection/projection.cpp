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

#include "projection.h"

#include "logger.h"
#include "projectionmanager.h"

Projection::Projection(const std::string& class_id, const std::string& instance_id,
                       ProjectionManager& proj_manager)
    : Configurable(class_id, instance_id, &proj_manager), proj_manager_(proj_manager)
{
    registerParameter("name", &name_, "");

    assert(name_.size());

    // createSubConfigurables called in subclasses
}

Projection::~Projection() {}

void Projection::generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id)
{
}

std::string Projection::name() const { return name_; }

void Projection::name(const std::string& name) { name_ = name; }

void Projection::checkSubConfigurables() {}
