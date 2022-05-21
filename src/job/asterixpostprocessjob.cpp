#include "dbcontent/dbcontent.h"
#include "asterixpostprocessjob.h"
#include "dbcontent/dbcontentmanager.h"
#include "datasourcemanager.h"
#include "buffer.h"
#include "compass.h"
#include "mainwindow.h"
#include "projectionmanager.h"
#include "projection.h"
#include "json.hpp"
#include "dbcontent/variable/metavariable.h"
#include "stringconv.h"

#include "boost/date_time/posix_time/posix_time.hpp"

const float tod_24h = 24 * 60 * 60;

using namespace std;
using namespace nlohmann;
using namespace Utils;

ASTERIXPostprocessJob::ASTERIXPostprocessJob(map<string, shared_ptr<Buffer>> buffers,
                                             bool override_tod_active, float override_tod_offset,
                                             bool do_timestamp_checks)
    : Job("ASTERIXPostprocessJob"),
      buffers_(move(buffers)), override_tod_active_(override_tod_active), override_tod_offset_(override_tod_offset),
      do_timestamp_checks_(do_timestamp_checks)
{
    network_time_offset_ = COMPASS::instance().mainWindow().importASTERIXFromNetworkTimeOffset();
}

ASTERIXPostprocessJob::~ASTERIXPostprocessJob() { logdbg << "ASTERIXPostprocessJob: dtor"; }

void ASTERIXPostprocessJob::run()
{
    logdbg << "ASTERIXPostprocessJob: run: num buffers " << buffers_.size();

    started_ = true;

    if (override_tod_active_)
        doTodOverride();

    if (do_timestamp_checks_)
        doFutureTimestampsCheck();

    doRadarPlotPositionCalculations();
    doGroundSpeedCalculations();

    //    boost::posix_time::time_duration time_diff = boost::posix_time::microsec_clock::local_time() - start_time;
    //    double ms = time_diff.total_milliseconds();
    //    loginf << "UGA Buffer sort took " << String::timeStringFromDouble(ms / 1000.0, true);

    done_ = true;
}

void ASTERIXPostprocessJob::doTodOverride()
{
    assert (override_tod_active_);

    DBContentManager& obj_man = COMPASS::instance().dbContentManager();

    unsigned int buffer_size;

    for (auto& buf_it : buffers_)
    {
        buffer_size = buf_it.second->size();

        assert (obj_man.metaVariable(DBContent::meta_var_tod_.name()).existsIn(buf_it.first));

        dbContent::Variable& tod_var = obj_man.metaVariable(DBContent::meta_var_tod_.name()).getFor(buf_it.first);

        Property tod_prop {tod_var.name(), tod_var.dataType()};

        assert (buf_it.second->hasProperty(tod_prop));

        NullableVector<float>& tod_vec = buf_it.second->get<float>(tod_var.name());

        for (unsigned int index=0; index < buffer_size; ++index)
        {
            if (!tod_vec.isNull(index))
            {
                float& tod_ref = tod_vec.getRef(index);

                tod_ref += override_tod_offset_;

                // check for out-of-bounds because of midnight-jump
                while (tod_ref < 0.0f)
                    tod_ref += tod_24h;
                while (tod_ref > tod_24h)
                    tod_ref -= tod_24h;

                assert(tod_ref >= 0.0f);
                assert(tod_ref <= tod_24h);
            }
        }
    }
}

void ASTERIXPostprocessJob::doFutureTimestampsCheck()
{
    DBContentManager& obj_man = COMPASS::instance().dbContentManager();

    unsigned int buffer_size;

    using namespace boost::posix_time;

    auto p_time = microsec_clock::universal_time (); // UTC.

    double tod_now_utc = (p_time.time_of_day().total_milliseconds() / 1000.0) + network_time_offset_ + 1.0; // up to 1 sec ok

    loginf << "ASTERIXPostprocessJob: doFutureTimestampsCheck: maximum time is "
           << String::timeStringFromDouble(tod_now_utc);

    for (auto& buf_it : buffers_)
    {
        buffer_size = buf_it.second->size();

        assert (obj_man.metaVariable(DBContent::meta_var_tod_.name()).existsIn(buf_it.first));

        dbContent::Variable& tod_var = obj_man.metaVariable(DBContent::meta_var_tod_.name()).getFor(buf_it.first);

        Property tod_prop {tod_var.name(), tod_var.dataType()};

        assert (buf_it.second->hasProperty(tod_prop));

        NullableVector<float>& tod_vec = buf_it.second->get<float>(tod_var.name());

        std::tuple<bool,float,float> min_max_tod = tod_vec.minMaxValues();

        if (get<0>(min_max_tod))
            loginf << "ASTERIXPostprocessJob: doFutureTimestampsCheck: " << buf_it.first
                   << " min tod " << String::timeStringFromDouble(get<1>(min_max_tod))
                   << " max " << String::timeStringFromDouble(get<2>(min_max_tod));


        for (unsigned int index=0; index < buffer_size; ++index)
        {
            if (!tod_vec.isNull(index) && tod_vec.get(index) > tod_now_utc)
            {
                logwrn << "ASTERIXPostprocessJob: doFutureTimestampsCheck: doing " << buf_it.first
                       << " cutoff tod index " << index
                       << " tod " << String::timeStringFromDouble(tod_vec.get(index));

                buf_it.second->cutToSize(index);

                break;
            }
        }
    }

    // remove empty buffers

    std::map<std::string, std::shared_ptr<Buffer>> tmp_data = buffers_;

    for (auto& buf_it : tmp_data)
        if (!buf_it.second->size())
            buffers_.erase(buf_it.first);
}

void ASTERIXPostprocessJob::doRadarPlotPositionCalculations()
{
    // radar calculations

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
    ProjectionManager& proj_man = ProjectionManager::instance();

    assert(proj_man.hasCurrentProjection());
    Projection& projection = proj_man.currentProjection();

    unsigned int ds_id;
    double azimuth_deg;
    double azimuth_rad;
    double range_nm;
    double range_m;
    double altitude_ft;
    bool has_altitude;
    double lat, lon;

    bool ret;

    unsigned int transformation_errors;

    for (auto& buf_it : buffers_)
    {
        dbcontent_name = buf_it.first;

        if (dbcontent_name != "CAT001" && dbcontent_name != "CAT048")
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
        assert (!buffer->has<double>(latitude_var_name));
        assert (!buffer->has<double>(longitude_var_name));

        buffer->addProperty(latitude_var_name, PropertyDataType::DOUBLE);
        buffer->addProperty(longitude_var_name, PropertyDataType::DOUBLE);

        transformation_errors = 0;

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
                if (!ds_man.hasConfigDataSource(ds_id_it) && !ds_man.hasDBDataSource(ds_id_it))
                {
                    logwrn << "ASTERIXPostprocessJob: doRadarPlotPositionCalculations: data source id "
                           << ds_id_it << " not defined in config or db";
                    continue;
                }

                //                if (!dbo_man.hasDataSource(ds_id_it))
                //                {
                //                    if(dbo_man.canAddNewDataSourceFromConfig(ds_id_it))
                //                        dbo_man.addNewDataSource(ds_id_it);
                //                    else
                //                    {
                //                        logerr << "ASTERIXPostprocessJob: run: ds id " << ds_id_it << " unknown";
                //                        continue;
                //                    }
                //                }

                //                assert (dbo_man.hasDataSource(ds_id_it));

                if (ds_man.hasConfigDataSource(ds_id_it))
                {
                    dbContent::ConfigurationDataSource& data_source = ds_man.configDataSource(ds_id_it);

                    if (data_source.hasFullPosition())
                    {
                        loginf << "ASTERIXPostprocessJob: doRadarPlotPositionCalculations: adding proj ds " << ds_id_it
                               << " lat/long " << data_source.latitude()
                               << "," << data_source.longitude()
                               << " alt " << data_source.altitude();

                        projection.addCoordinateSystem(ds_id_it, data_source.latitude(),
                                                       data_source.longitude(),
                                                       data_source.altitude());
                    }
                    else
                        logerr << "ASTERIXPostprocessJob: doRadarPlotPositionCalculations: config ds "
                               << data_source.name()
                               << " defined but missing position info";
                }
                else if (ds_man.hasDBDataSource(ds_id_it))
                {
                    dbContent::DBDataSource& data_source = ds_man.dbDataSource(ds_id_it);

                    if (data_source.hasFullPosition())
                    {
                        loginf << "ASTERIXPostprocessJob: run: adding proj ds " << ds_id_it
                               << " lat/long " << data_source.latitude()
                               << "," << data_source.longitude()
                               << " alt " << data_source.altitude();

                        projection.addCoordinateSystem(ds_id_it, data_source.latitude(),
                                                       data_source.longitude(),
                                                       data_source.altitude());
                    }
                    else
                        logerr << "ASTERIXPostprocessJob: run: ds " << data_source.name()
                               << " defined but missing position info";
                }
            }

        }

        for (unsigned int cnt = 0; cnt < buffer_size; cnt++)
        {
            // load buffer data

            if (datasource_vec.isNull(cnt))
            {
                logerr << "ASTERIXPostprocessJob: run: data source null";
                continue;
            }
            ds_id = datasource_vec.get(cnt);

            if (azimuth_vec.isNull(cnt) || range_vec.isNull(cnt))
            {
                logdbg << "ASTERIXPostprocessJob: run: position null";
                continue;
            }

            azimuth_deg = azimuth_vec.get(cnt);
            range_nm = range_vec.get(cnt);

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

    // do first buffer sorting

    //    loginf << "ASTERIXPostprocessJob: run: sorting buffers";

    //    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

    //    for (auto& buf_it : buffers_)
    //    {
    //        logdbg << "ASTERIXPostprocessJob: run: sorting buffer " << buf_it.first;

    //        assert (dbo_man.existsMetaVariable(DBContent::meta_var_tod_id_.name()));
    //        assert (dbo_man.metaVariable(DBContent::meta_var_tod_id_.name()).existsIn(buf_it.first));

    //        DBOVariable& tod_var = dbo_man.metaVariable(DBContent::meta_var_tod_id_.name()).getFor(buf_it.first);

    //        Property prop {tod_var.name(), tod_var.dataType()};

    //        logdbg << "ASTERIXPostprocessJob: run: sorting by variable " << prop.name() << " " << prop.dataTypeString();

    //        assert (buf_it.second->hasProperty(prop));

    //        buf_it.second->sortByProperty(prop);
    //    }

}

void ASTERIXPostprocessJob::doGroundSpeedCalculations()
{
    string dbcontent_name;

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    string vx_var_name;
    string vy_var_name;
    string speed_var_name;
    string track_angle_var_name;

    double speed_ms, bearing_rad;

    for (auto& buf_it : buffers_)
    {
        dbcontent_name = buf_it.first;

        if (!dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_vx_)
                || !dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_vy_))
            continue;

        shared_ptr<Buffer> buffer = buf_it.second;
        unsigned int buffer_size = buffer->size();
        assert(buffer_size);

        assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ground_speed_));
        assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_track_angle_));

        dbContent::Variable& vx_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_vx_);
        dbContent::Variable& vy_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_vy_);
        dbContent::Variable& speed_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ground_speed_);
        dbContent::Variable& track_angle_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_angle_);

        vx_var_name = vx_var.name();
        vy_var_name = vy_var.name();
        speed_var_name = speed_var.name();
        track_angle_var_name = track_angle_var.name();

        assert (vx_var.dataType() == PropertyDataType::DOUBLE);
        assert (vy_var.dataType() == PropertyDataType::DOUBLE);
        assert (speed_var.dataType() == PropertyDataType::DOUBLE);
        assert (track_angle_var.dataType() == PropertyDataType::DOUBLE);

        if (!buffer->has<double>(vx_var_name) || !buffer->has<double>(vy_var_name))
            continue; // cant calculate

        if (buffer->has<double>(speed_var_name) && buffer->has<double>(track_angle_var_name))
            continue; // no need for calculation

        if (!buffer->has<double>(speed_var_name))
            buffer->addProperty(speed_var_name, PropertyDataType::DOUBLE); // add if needed

        if (!buffer->has<double>(track_angle_var_name))
            buffer->addProperty(track_angle_var_name, PropertyDataType::DOUBLE); // add if needed

        NullableVector<double>& vx_vec = buffer->get<double>(vx_var_name);
        NullableVector<double>& vy_vec = buffer->get<double>(vy_var_name);
        NullableVector<double>& speed_vec = buffer->get<double>(speed_var_name);
        NullableVector<double>& track_angle_vec = buffer->get<double>(track_angle_var_name);

        for (unsigned int index=0; index < buffer_size; index++)
        {
            if (vx_vec.isNull(index) || vy_vec.isNull(index))
                continue;

            speed_ms = sqrt(pow(vx_vec.get(index), 2)+pow(vy_vec.get(index), 2)) ; // for 1s
            bearing_rad = atan2(vx_vec.get(index), vy_vec.get(index));

            speed_vec.set(index, speed_ms * M_S2KNOTS);
            track_angle_vec.set(index, bearing_rad * RAD2DEG);
        }
    }
}
