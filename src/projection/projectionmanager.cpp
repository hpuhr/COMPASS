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
#include "fftmanager.h"
#include "compass.h"

#include <math.h>

#include <boost/optional/optional_io.hpp>
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

unsigned int ProjectionManager::calculateRadarPlotPositions (
        std:: string dbcontent_name, std::shared_ptr<Buffer> buffer,
        NullableVector<double>& target_latitudes_vec, NullableVector<double>& target_longitudes_vec)
{
    logdbg << "ProjectionManager: calculateRadarPlotPositions: dbcontent_name " << dbcontent_name;

    bool ret;

    string datasource_var_name;
    string range_var_name;
    string azimuth_var_name;
    string altitude_var_name;
    string latitude_var_name;
    string longitude_var_name;

    string mode_s_address_var_name;
    string mode_a_code_var_name;

    // do radar position projection

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();
    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();
    FFTManager& fft_man = COMPASS::instance().fftManager();

    unsigned int ds_id;
    double azimuth_deg;
    double azimuth_rad;
    double range_nm;
    double range_m;
    double altitude_ft;
    bool has_altitude;
    double lat, lon;

    unsigned int transformation_errors {0};
    unsigned int num_ffts_found {0};

    bool is_from_fft;
    float fft_altitude_ft;

    boost::optional<unsigned int> mode_s_address;
    boost::optional<unsigned int> mode_a_code;
    boost::optional<float> mode_c_code;

    assert (dbcontent_name == "CAT001" || dbcontent_name == "CAT010" || dbcontent_name == "CAT048");

    unsigned int buffer_size = buffer->size();
    assert(buffer_size);

    assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_datasource_id_));
    assert (dbcont_man.canGetVariable(dbcontent_name, DBContent::var_radar_range_));
    assert (dbcont_man.canGetVariable(dbcontent_name, DBContent::var_radar_azimuth_));
    assert (dbcont_man.canGetVariable(dbcontent_name, DBContent::var_radar_altitude_));
    assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_latitude_));
    assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_longitude_));

    datasource_var_name = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_datasource_id_).name();
    range_var_name = dbcont_man.getVariable(dbcontent_name, DBContent::var_radar_range_).name();
    azimuth_var_name = dbcont_man.getVariable(dbcontent_name, DBContent::var_radar_azimuth_).name();
    altitude_var_name = dbcont_man.getVariable(dbcontent_name, DBContent::var_radar_altitude_).name();
    latitude_var_name = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_).name();
    longitude_var_name = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_).name();

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

    // optional data for fft check
    NullableVector<unsigned int>* mode_s_address_vec {nullptr};

    if (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ta_))
    {
        mode_s_address_var_name = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ta_).name();
        assert (buffer->has<unsigned int>(mode_s_address_var_name));

        mode_s_address_vec = &buffer->get<unsigned int>(mode_s_address_var_name);
    }

    NullableVector<unsigned int>* mode_a_code_vec {nullptr};
    if (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_m3a_))
    {
        mode_a_code_var_name = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_).name();
        assert (buffer->has<unsigned int>(mode_a_code_var_name));

        mode_a_code_vec = &buffer->get<unsigned int>(mode_a_code_var_name);
    }

    // set up projections
    assert(hasCurrentProjection());
    Projection& projection = currentProjection();
    assert (projection.radarCoordinateSystemsAdded()); // needs to have been prepared, otherwise multi-threading issue

    for (auto ds_id_it : datasource_vec.distinctValues())
    {
        if (!projection.hasCoordinateSystem(ds_id_it))
        {
            if (ds_man.hasConfigDataSource(ds_id_it) && ds_man.configDataSource(ds_id_it).dsType() != "Radar")
                continue; // ok for non-radars

            logwrn << "ProjectionManager: calculateRadarPlotPositions: data source id "
                   << ds_id_it << " not set up"; // should have been in ASTERIX import task
        }
    }

    double diff, diff_min {10e6}, diff_max{0}, diff_avg {0};
    unsigned int diff_cnt {0};

    for (unsigned int cnt = 0; cnt < buffer_size; cnt++)
    {
        // load buffer data

        if (datasource_vec.isNull(cnt))
        {
            logerr << "ProjectionManager: calculateRadarPlotPositions: data source null";
            continue;
        }
        ds_id = datasource_vec.get(cnt);

        if (azimuth_vec.isNull(cnt) || range_vec.isNull(cnt))
        {
            logdbg << "ProjectionManager: calculateRadarPlotPositions: position null";
            continue;
        }

        if (!target_latitudes_vec.isNull(cnt) && !target_longitudes_vec.isNull(cnt))
        {
            logdbg << "ProjectionManager: calculateRadarPlotPositions: position already set";
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

        // check if fft

        if (mode_s_address_vec && !mode_s_address_vec->isNull(cnt))
            mode_s_address = mode_s_address_vec->get(cnt);
        else
            mode_s_address = boost::none;

        if (mode_a_code_vec && !mode_a_code_vec->isNull(cnt))
            mode_a_code = mode_a_code_vec->get(cnt);
        else
            mode_a_code =  boost::none;

        if (has_altitude)
            mode_c_code = altitude_ft;
        else
            mode_c_code =  boost::none;


        std::tie(is_from_fft, fft_altitude_ft) = fft_man.isFromFFT(
                    lat, lon, mode_s_address, dbcontent_name == "CAT001",
                    mode_a_code, mode_c_code);

        if (is_from_fft) // recalculate position
        {
            ++num_ffts_found;

            double old_lat = lat;
            double old_lon = lon;

            logdbg << "ProjectionManager: calculateRadarPlotPositions: mode_c_code " << mode_c_code
                   << " fft_altitude_ft " << fft_altitude_ft;

            ret = projection.polarToWGS84(ds_id, azimuth_rad, range_m, true,
                                          fft_altitude_ft, lat, lon);

            if (!ret)
            {
                transformation_errors++;
                continue;
            }

            diff = 100 * sqrt(pow(lat-old_lat, 2) + pow(lon-old_lon, 2));

            logdbg << "ProjectionManager: calculateRadarPlotPositions: lat/lon diff "
                   << diff;

            diff_avg += diff;
            diff_min = min(diff_min, diff);
            diff_max = max(diff_max, diff);

            ++diff_cnt;
        }

        target_latitudes_vec.set(cnt, lat);
        target_longitudes_vec.set(cnt, lon);
    }

    logdbg << "ProjectionManager: calculateRadarPlotPositions: dbcontent_name " << dbcontent_name
           << " num_ffts_found " << num_ffts_found << " transformation_errors " << transformation_errors;

    if (diff_cnt)
    {
        logdbg << "ProjectionManager: calculateRadarPlotPositions: lat/lon avg diff "
               << diff_avg / (float) diff_cnt << " min " << diff_min << " max " << diff_max << " cnt " << diff_cnt;
    }

    return transformation_errors;
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
    unsigned int transformation_errors {0};

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();
    string dbcontent_name;

    string latitude_var_name;
    string longitude_var_name;

    for (auto& buf_it : buffers)
    {
        dbcontent_name = buf_it.first;

        if (dbcontent_name != "CAT001" && dbcontent_name != "CAT010" && dbcontent_name != "CAT048")
            continue;

        shared_ptr<Buffer> buffer = buf_it.second;

        assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_latitude_));
        assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_longitude_));

        latitude_var_name = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_).name();
        longitude_var_name = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_).name();

        if (!buffer->has<double>(latitude_var_name))
            buffer->addProperty(latitude_var_name, PropertyDataType::DOUBLE);

        if (!buffer->has<double>(longitude_var_name))
            buffer->addProperty(longitude_var_name, PropertyDataType::DOUBLE);

        NullableVector<double>& latitude_vec = buffer->get<double>(latitude_var_name);
        NullableVector<double>& longitude_vec = buffer->get<double>(longitude_var_name);

        transformation_errors += calculateRadarPlotPositions (dbcontent_name, buffer, latitude_vec, longitude_vec);
    }

    return transformation_errors;
}

std::pair<unsigned int, std::map<std::string, std::shared_ptr<Buffer>>>
ProjectionManager::doUpdateRadarPlotPositionCalculations (std::map<std::string, std::shared_ptr<Buffer>> buffers)
{
    unsigned int transformation_errors {0};
    std::map<std::string, std::shared_ptr<Buffer>> update_buffers;

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();
    string dbcontent_name;

    string latitude_var_name;
    string longitude_var_name;
    string rec_num_var_name;

    for (auto& buf_it : buffers)
    {
        dbcontent_name = buf_it.first;

        assert (dbcontent_name == "CAT001" || dbcontent_name == "CAT010" || dbcontent_name == "CAT048");

        shared_ptr<Buffer> read_buffer = buf_it.second;
        unsigned int read_size = read_buffer->size();

        latitude_var_name = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_).name();
        longitude_var_name = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_).name();
        rec_num_var_name = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_rec_num_).name();

        PropertyList update_buffer_list;

        update_buffer_list.addProperty(latitude_var_name, PropertyDataType::DOUBLE);
        update_buffer_list.addProperty(longitude_var_name, PropertyDataType::DOUBLE);
        // must be at last position for update
        update_buffer_list.addProperty(rec_num_var_name, PropertyDataType::ULONGINT);

        std::shared_ptr<Buffer> update_buffer =
                std::make_shared<Buffer>(update_buffer_list, dbcontent_name);

        // copy record number

        NullableVector<unsigned long>& read_rec_num_vec = read_buffer->get<unsigned long> (rec_num_var_name);

        NullableVector<unsigned long>& update_rec_num_vec = update_buffer->get<unsigned long> (rec_num_var_name);

        assert (read_rec_num_vec.isNeverNull());

        for (unsigned int cnt = 0; cnt < read_size; cnt++)
            update_rec_num_vec.set(cnt, read_rec_num_vec.get(cnt));

        NullableVector<double>& latitude_vec = update_buffer->get<double>(latitude_var_name);
        NullableVector<double>& longitude_vec = update_buffer->get<double>(longitude_var_name);

        transformation_errors += calculateRadarPlotPositions (dbcontent_name, read_buffer, latitude_vec, longitude_vec);

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
