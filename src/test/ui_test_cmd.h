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

#include "rtcommand.h"

namespace ui_test
{

/** 
 */
struct RTCommandUIInjection : public rtcommand::RTCommandObject
{
    int injection_delay = -1; //delay used for each UI injection
    
    DECLARE_RTCOMMAND_OPTIONS
};

/**
 */
struct RTCommandUISet : public RTCommandUIInjection
{
    QString value;
protected:
    virtual bool run_impl() const override;

    DECLARE_RTCOMMAND(uiset, "sets an ui element to the given value")
    DECLARE_RTCOMMAND_OPTIONS
};

/**
 */
struct RTCommandUIGet : public rtcommand::RTCommandObject
{
    QString what;
protected:
    virtual bool run_impl() const override;

    DECLARE_RTCOMMAND(uiget, "retrieves the value of the given ui element")
    DECLARE_RTCOMMAND_OPTIONS
};

/**
*/
inline void initUITestCommands()
{
    RTCommandUISet::init();
    RTCommandUIGet::init();
}

} // namespace ui_test
