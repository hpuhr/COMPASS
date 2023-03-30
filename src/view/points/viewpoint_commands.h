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

#ifndef VIEWPOINT_COMMANDS_H
#define VIEWPOINT_COMMANDS_H

#include "rtcommand/rtcommand.h"
#include "viewabledataconfig.h"

#include <memory>

extern void init_view_point_commands();

// set_view_point "{\"id\": 1,\"name\": \"All\",\"status\": \"open\",\"type\": \"Saved\"}"
struct RTCommandSetViewPoint : public rtcommand::RTCommand
{
public:
    std::string vp_json_str_;
    std::unique_ptr<ViewableDataConfig> viewable_data_cfg_;

    RTCommandSetViewPoint();

protected:
    virtual bool run_impl() override;
    virtual bool checkResult_impl() override;

    DECLARE_RTCOMMAND(set_view_point, "sets defined view point")
    DECLARE_RTCOMMAND_OPTIONS
};

#endif // VIEWPOINT_COMMANDS_H
