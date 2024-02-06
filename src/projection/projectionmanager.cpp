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
#include "global.h"
#include "logger.h"
#include "ogrprojection.h"
#include "projectionmanagerwidget.h"
#include "rs2gprojection.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
#include "datasourcemanager.h"
#include "compass.h"

#include <math.h>

#include <cmath>

using namespace std;

const string ProjectionManager::RS2G_NAME = "RS2G";
const string ProjectionManager::OGR_NAME = "OGR";

ProjectionManager::ProjectionManager()
    : Configurable("ProjectionManager", "ProjectionManager0", 0, "projection.json")
{
    loginf << "ProjectionManager: constructor";

    registerParameter("current_projection_name", &current_projection_name_, string("RS2G"));

    createSubConfigurables();

    assert(hasCurrentProjection());
}

ProjectionManager::~ProjectionManager()
{
}

void ProjectionManager::generateSubConfigurable(const string& class_id,
                                                const string& instance_id)
{
    if (class_id == "RS2GProjection")
    {
        string name = getSubConfiguration(class_id, instance_id).getParameterConfigValue<string>("name");

        assert(!projections_.count(name));

        projections_[name].reset(new RS2GProjection(class_id, instance_id, *this));
    }
    else if (class_id == "OGRProjection")
    {
        string name = getSubConfiguration(class_id, instance_id).getParameterConfigValue<string>("name");

        assert(!projections_.count(name));

        projections_[name].reset(new OGRProjection(class_id, instance_id, *this));
    }
    else
        throw runtime_error("DBContent: generateSubConfigurable: unknown class_id " + class_id);
}

void ProjectionManager::checkSubConfigurables()
{
    if (!projections_.count(RS2G_NAME))
    {
        auto configuration = Configuration::create("RS2GProjection");

        configuration->addParameter<string>("name", RS2G_NAME);
        generateSubConfigurableFromConfig(std::move(configuration));
    }

    if (!projections_.count(OGR_NAME))
    {
        auto configuration = Configuration::create("OGRProjection");

        configuration->addParameter<string>("name", OGR_NAME);
        generateSubConfigurableFromConfig(std::move(configuration));
    }
}

string ProjectionManager::currentProjectionName() const { return current_projection_name_; }

void ProjectionManager::currentProjectionName(const string& name)
{
    loginf << "ProjectionManager: currentProjectionName: name " << name;
    current_projection_name_ = name;
}

bool ProjectionManager::hasProjection(const string& name) { return projections_.count(name); }

Projection& ProjectionManager::projection(const string& name)
{
    assert (hasProjection(name));
    return *projections_.at(name);
}

bool ProjectionManager::hasCurrentProjection() { return hasProjection(current_projection_name_); }

Projection& ProjectionManager::currentProjection()
{
    return *projections_.at(current_projection_name_);
}

map<string, unique_ptr<Projection>>& ProjectionManager::projections()
{
    return projections_;
}

OGRProjection& ProjectionManager::ogrProjection()
{
    assert (hasProjection(OGR_NAME));
    assert (projections_.at(OGR_NAME));
    assert (dynamic_cast<OGRProjection*> (projections_.at(OGR_NAME).get()));
    return *dynamic_cast<OGRProjection*> (projections_.at(OGR_NAME).get());
}


unsigned int ProjectionManager::doRadarPlotPositionCalculations (
        map<string, shared_ptr<Buffer>> buffers)
{
    bool ret;

    string dbcontent_name;

    string datasource_var_name;
    string range_var_name;
    string azimuth_var_name;
    string altitude_var_name;
    string latitude_var_name;
    string longitude_var_name;

    // do radar position projection

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();
    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    assert(hasCurrentProjection());
    Projection& projection = currentProjection();

    unsigned int ds_id;
    double azimuth_deg;
    double azimuth_rad;
    double range_nm;
    double range_m;
    double altitude_ft;
    bool has_altitude;
    double lat, lon;

    unsigned int transformation_errors = 0;

    for (auto& buf_it : buffers)
    {
        dbcontent_name = buf_it.first;

        if (dbcontent_name != "CAT001" && dbcontent_name != "CAT010" && dbcontent_name != "CAT048")
            continue;

        shared_ptr<Buffer> buffer = buf_it.second;
        unsigned int buffer_size = buffer->size();
        assert(buffer_size);

        assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_datasource_id_));
        assert (dbcont_man.canGetVariable(dbcontent_name, DBContent::var_radar_range_));
        assert (dbcont_man.canGetVariable(dbcontent_name, DBContent::var_radar_azimuth_));
        assert (dbcont_man.canGetVariable(dbcontent_name, DBContent::var_radar_altitude_));
        assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_latitude_));
        assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_longitude_));

        dbContent::Variable& datasource_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_datasource_id_);
        dbContent::Variable& range_var = dbcont_man.getVariable(dbcontent_name, DBContent::var_radar_range_);
        dbContent::Variable& azimuth_var = dbcont_man.getVariable(dbcontent_name, DBContent::var_radar_azimuth_);
        dbContent::Variable& altitude_var = dbcont_man.getVariable(dbcontent_name, DBContent::var_radar_altitude_);
        dbContent::Variable& latitude_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_);
        dbContent::Variable& longitude_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_);

        datasource_var_name = datasource_var.name();
        range_var_name = range_var.name();
        azimuth_var_name = azimuth_var.name();
        altitude_var_name = altitude_var.name();
        latitude_var_name = latitude_var.name();
        longitude_var_name = longitude_var.name();

        assert (datasource_var.dataType() == PropertyDataType::UINT);
        assert (range_var.dataType() == PropertyDataType::DOUBLE);
        assert (azimuth_var.dataType() == PropertyDataType::DOUBLE);
        assert (altitude_var.dataType() == PropertyDataType::FLOAT);
        assert (latitude_var.dataType() == PropertyDataType::DOUBLE);
        assert (longitude_var.dataType() == PropertyDataType::DOUBLE);

        assert (buffer->has<unsigned int>(datasource_var_name));
        assert (buffer->has<double>(range_var_name));
        assert (buffer->has<double>(azimuth_var_name));
        assert (buffer->has<float>(altitude_var_name));

        if (!buffer->has<double>(latitude_var_name))
            buffer->addProperty(latitude_var_name, PropertyDataType::DOUBLE);

        if (!buffer->has<double>(longitude_var_name))
            buffer->addProperty(longitude_var_name, PropertyDataType::DOUBLE);

        NullableVector<unsigned int>& datasource_vec = buffer->get<unsigned int>(datasource_var_name);
        NullableVector<double>& range_vec = buffer->get<double>(range_var_name);
        NullableVector<double>& azimuth_vec = buffer->get<double>(azimuth_var_name);
        NullableVector<float>& altitude_vec = buffer->get<float>(altitude_var_name);
        NullableVector<double>& latitude_vec = buffer->get<double>(latitude_var_name);
        NullableVector<double>& longitude_vec = buffer->get<double>(longitude_var_name);

        // set up projections

        for (auto ds_id_it : datasource_vec.distinctValues())
        {
            if (!projection.hasCoordinateSystem(ds_id_it))
            {
                if (ds_man.hasConfigDataSource(ds_id_it) && ds_man.configDataSource(ds_id_it).dsType() != "Radar")
                    continue; // ok for non-radars

                logwrn << "ProjectionManager: doRadarPlotPositionCalculations: data source id "
                       << ds_id_it << " not set up"; // should have been in ASTERIX import task
            }
        }

        for (unsigned int cnt = 0; cnt < buffer_size; cnt++)
        {
            // load buffer data

            if (datasource_vec.isNull(cnt))
            {
                logerr << "ProjectionManager: doRadarPlotPositionCalculations: data source null";
                continue;
            }
            ds_id = datasource_vec.get(cnt);

            if (azimuth_vec.isNull(cnt) || range_vec.isNull(cnt))
            {
                logdbg << "ProjectionManager: doRadarPlotPositionCalculations: position null";
                continue;
            }

            if (!latitude_vec.isNull(cnt) && !longitude_vec.isNull(cnt))
            {
                logdbg << "ProjectionManager: doRadarPlotPositionCalculations: position already set";
                continue;
            }

            azimuth_deg = azimuth_vec.get(cnt);
            range_nm = range_vec.get(cnt);

            //loginf << "azimuth_deg " << azimuth_deg << " range_nm " << range_nm;

            has_altitude = !altitude_vec.isNull(cnt);
            if (has_altitude)
                altitude_ft = altitude_vec.get(cnt);
            else
                altitude_ft = 0.0;  // has to assumed in projection later on

            azimuth_rad = azimuth_deg * DEG2RAD;

            range_m = 1852.0 * range_nm;

            if (!projection.hasCoordinateSystem(ds_id))
            {
                transformation_errors++;
                continue;
            }

            ret = projection.polarToWGS84(ds_id, azimuth_rad, range_m, has_altitude,
                                          altitude_ft, lat, lon);

            if (!ret)
            {
                transformation_errors++;
                continue;
            }

            latitude_vec.set(cnt, lat);
            longitude_vec.set(cnt, lon);
        }
    }

    return transformation_errors;
}

std::pair<unsigned int, std::map<std::string, std::shared_ptr<Buffer>>>
ProjectionManager::doUpdateRadarPlotPositionCalculations (std::map<std::string, std::shared_ptr<Buffer>> buffers)
{
    assert(hasCurrentProjection());
    Projection& projection = currentProjection();
    projection.clearCoordinateSystems(); // to rebuild from data sources
    projection.addAllRadarCoordinateSystems();

    loginf << "ProjectionManager: doUpdateRadarPlotPositionCalculations: projection method '"
           << projection.name() << "'";

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    string dbcontent_name;
    set<unsigned int> ds_unknown; // to log only once

    unsigned int transformation_errors = 0;
    std::map<std::string, std::shared_ptr<Buffer>> update_buffers;

    for (auto& buf_it : buffers)
    {
        dbcontent_name = buf_it.first;
        auto& read_buffer = buf_it.second;
        unsigned int read_size = read_buffer->size();

        assert (dbcontent_name == "CAT001" || dbcontent_name == "CAT010" || dbcontent_name == "CAT048");

        PropertyList update_buffer_list;

        update_buffer_list.addProperty(
                    dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_).name(),
                    PropertyDataType::DOUBLE);
        update_buffer_list.addProperty(
                    dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_).name(),
                    PropertyDataType::DOUBLE);
        update_buffer_list.addProperty(
                    dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_rec_num_).name(),
                    PropertyDataType::ULONGINT); // must be at last position for update

        std::shared_ptr<Buffer> update_buffer =
                std::make_shared<Buffer>(update_buffer_list, dbcontent_name);

        unsigned long rec_num;
        unsigned int ds_id;

        double pos_azm_deg;
        double pos_azm_rad;
        double pos_range_nm;
        double pos_range_m;
        double altitude_ft;
        bool has_altitude;

        double lat, lon;
        unsigned int update_cnt = 0;

        loginf << "ProjectionManager: doUpdateRadarPlotPositionCalculations: writing update_buffer";
        bool ret;

        NullableVector<unsigned int>& read_ds_id_vec = read_buffer->get<unsigned int> (
                    dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_datasource_id_).name());
        NullableVector<unsigned long>& read_rec_num_vec = read_buffer->get<unsigned long> (
                    dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_rec_num_).name());
        NullableVector<double>& read_range_vec = read_buffer->get<double> (
                    dbcontent_man.getVariable(dbcontent_name, DBContent::var_radar_range_).name());
        NullableVector<double>& read_azimuth_vec = read_buffer->get<double> (
                    dbcontent_man.getVariable(dbcontent_name, DBContent::var_radar_azimuth_).name());
        NullableVector<float>& read_altitude_vec = read_buffer->get<float> (
                    dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_).name());

        NullableVector<double>& write_lat_vec = update_buffer->get<double> (
                    dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_).name());
        NullableVector<double>& write_lon_vec = update_buffer->get<double> (
                    dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_).name());
        NullableVector<unsigned long>& write_rec_num_vec = update_buffer->get<unsigned long> (
                    dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_rec_num_).name());

        assert (read_ds_id_vec.isNeverNull());
        assert (read_rec_num_vec.isNeverNull());

        for (unsigned int cnt = 0; cnt < read_size; cnt++)
        {
            rec_num = read_rec_num_vec.get(cnt);

            ds_id = read_ds_id_vec.get(cnt);

            if (read_azimuth_vec.isNull(cnt) || read_range_vec.isNull(cnt))
                continue;

            pos_azm_deg = read_azimuth_vec.get(cnt);
            pos_range_nm = read_range_vec.get(cnt);

            has_altitude = !read_altitude_vec.isNull(cnt);

            if (has_altitude)
                altitude_ft = read_altitude_vec.get(cnt);
            else
                altitude_ft = 0.0;  // has to assumed in projection later on

            pos_azm_rad = pos_azm_deg * DEG2RAD;

            pos_range_m = 1852.0 * pos_range_nm;

            if (!projection.hasCoordinateSystem(ds_id))
            {
                if (!ds_unknown.count(ds_id))
                {
                    logwrn << "ProjectionManager: doUpdateRadarPlotPositionCalculations: unknown data source " << ds_id
                           << ", skipping";
                    ds_unknown.insert(ds_id);
                }
                continue;
            }

            ret = projection.polarToWGS84(ds_id, pos_azm_rad, pos_range_m, has_altitude,
                                          altitude_ft, lat, lon);

            if (!ret)
            {
                transformation_errors++;
                continue;
            }

            write_lat_vec.set(update_cnt, lat);
            write_lon_vec.set(update_cnt, lon);
            write_rec_num_vec.set(update_cnt, rec_num);

            update_cnt++;

            // loginf << "uga cnt " << update_cnt << " rec_num " << rec_num << " lat " << lat << " long
            // " << lon;
        }

        update_buffers[dbcontent_name] = std::move(update_buffer);
    }

    return {transformation_errors, update_buffers};
}

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
