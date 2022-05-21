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

#include "projectionmanager.h"

#include <math.h>

#include <cmath>

#include "global.h"
#include "logger.h"
#include "ogrprojection.h"
#include "projectionmanagerwidget.h"
#include "rs2gprojection.h"

ProjectionManager::ProjectionManager()
    : Configurable("ProjectionManager", "ProjectionManager0", 0, "projection.json")
{
    loginf << "ProjectionManager: constructor";

    registerParameter("current_projection_name", &current_projection_name_, "RS2G");

    //    // init sdl
    //    t_GPos geo_pos;

    //    preset_gpos (&geo_pos);
    //    preset_mapping_info (&sdl_mapping_info_);

    //    geo_pos.latitude = sdl_system_latitude_ * DEG2RAD;
    //    geo_pos.longitude = sdl_system_longitude_ * DEG2RAD;
    //    geo_pos.altitude = 0.0; // TODO check if exists
    //    geo_pos.defined = true;

    //    t_Retc lrc;

    //    lrc = geo_calc_info (geo_pos, &sdl_mapping_info_);

    //    assert (lrc == RC_OKAY);

    createSubConfigurables();

    assert(hasCurrentProjection());

    // init radSlt2Geo
}

ProjectionManager::~ProjectionManager()
{
}

void ProjectionManager::generateSubConfigurable(const std::string& class_id,
                                                const std::string& instance_id)
{
    if (class_id == "RS2GProjection")
    {
        std::string name = configuration()
                               .getSubConfiguration(class_id, instance_id)
                               .getParameterConfigValueString("name");

        assert(!projections_.count(name));

        projections_[name].reset(new RS2GProjection(class_id, instance_id, *this));
    }
    else if (class_id == "OGRProjection")
    {
        std::string name = configuration()
                               .getSubConfiguration(class_id, instance_id)
                               .getParameterConfigValueString("name");

        assert(!projections_.count(name));

        projections_[name].reset(new OGRProjection(class_id, instance_id, *this));
    }
    else
        throw std::runtime_error("DBContent: generateSubConfigurable: unknown class_id " + class_id);
}

void ProjectionManager::checkSubConfigurables()
{
    if (!projections_.count("RS2G"))
    {
        Configuration& configuration = addNewSubConfiguration("RS2GProjection");

        configuration.addParameterString("name", "RS2G");
        generateSubConfigurable("RS2GProjection", configuration.getInstanceId());
    }

    if (!projections_.count("OGR"))
    {
        Configuration& configuration = addNewSubConfiguration("OGRProjection");

        configuration.addParameterString("name", "OGR");
        generateSubConfigurable("OGRProjection", configuration.getInstanceId());
    }
}

std::string ProjectionManager::currentProjectionName() const { return current_projection_name_; }

void ProjectionManager::currentProjectionName(const std::string& name)
{
    loginf << "ProjectionManager: currentProjectionName: name " << name;
    current_projection_name_ = name;
}

bool ProjectionManager::hasProjection(const std::string& name) { return projections_.count(name); }

bool ProjectionManager::hasCurrentProjection() { return hasProjection(current_projection_name_); }

Projection& ProjectionManager::currentProjection()
{
    return *projections_.at(current_projection_name_);
}

std::map<std::string, std::unique_ptr<Projection>>& ProjectionManager::projections()
{
    return projections_;
}


// bool ProjectionManager::sdlGRS2Geo (t_CPos grs_pos, t_GPos& geo_pos)
//{
//    //logdbg << "ProjectionManager: sdlGRS2Geo: x_pos " << x_pos << " y_pos " << y_pos;

//    t_Retc lrtc;
//    //    t_CPos lcl_pos;

//    //    lrtc = geo_grs_to_lcl (sdl_mapping_info_, grs_pos, &lcl_pos);

//    t_GPos tmp_geo_pos;

//    lrtc = geo_grs_to_llh (grs_pos, &tmp_geo_pos);

//    assert (lrtc == RC_OKAY);
//    geo_pos = tmp_geo_pos;

//    return true;
//}

ProjectionManagerWidget* ProjectionManager::widget()
{
    if (!widget_)
    {
        widget_.reset(new ProjectionManagerWidget(*this));
    }
    assert(widget_);
    return widget_.get();
}

void ProjectionManager::deleteWidget()
{
    widget_ = nullptr;
}
