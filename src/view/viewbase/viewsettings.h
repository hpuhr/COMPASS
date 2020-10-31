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

#ifndef VIEWSETTINGS_H
#define VIEWSETTINGS_H

#include "singleton.h"

/**
@brief Base class for common settings of a view category.

This class is thought as a common settings container for a specific class of views.
There is not much to it at the moment.

@todo Not much purpose for this class at the moment, maybe later...
  */
class ViewSettings : public Singleton
{
  public:
    ViewSettings();
    virtual ~ViewSettings();

    /// @brief Reimplement to init the settings, e.g. load common resources etc.
    virtual void init() {}

    /// @brief Returns the instance of the singleton
    static ViewSettings& getInstance()
    {
        static ViewSettings instance;
        return instance;
    }
};

#endif  // VIEWSETTINGS_H
