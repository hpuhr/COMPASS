#include "asterixpostprocessjob.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
#include "buffer.h"
#include "compass.h"
#include "mainwindow.h"
#include "projectionmanager.h"
#include "projection.h"
#include "json.hpp"
#include "metadbovariable.h"
#include "stringconv.h"

#include "boost/date_time/posix_time/posix_time.hpp"

using namespace std;
using namespace nlohmann;
using namespace Utils;

ASTERIXPostprocessJob::ASTERIXPostprocessJob(map<string, shared_ptr<Buffer>> buffers, bool do_timestamp_checks)
    : Job("ASTERIXPostprocessJob"),
      buffers_(move(buffers)), do_timestamp_checks_(do_timestamp_checks)
{
    network_time_offset_ = COMPASS::instance().mainWindow().importASTERIXFromNetworkTimeOffset();
}

ASTERIXPostprocessJob::~ASTERIXPostprocessJob() { logdbg << "ASTERIXPostprocessJob: dtor"; }

void ASTERIXPostprocessJob::run()
{
    logdbg << "ASTERIXPostprocessJob: run: num buffers " << buffers_.size();

    started_ = true;

    if (do_timestamp_checks_)
        doFutureTimestampsCheck();

    doRadarPlotPositionCalculations();

    //    boost::posix_time::time_duration time_diff = boost::posix_time::microsec_clock::local_time() - start_time;
    //    double ms = time_diff.total_milliseconds();
    //    loginf << "UGA Buffer sort took " << String::timeStringFromDouble(ms / 1000.0, true);

    done_ = true;
}

void ASTERIXPostprocessJob::doFutureTimestampsCheck()
{
    DBContentManager& obj_man = COMPASS::instance().objectManager();

    unsigned int buffer_size;

    using namespace boost::posix_time;

    auto p_time = microsec_clock::universal_time (); // UTC.

    double tod_now_utc = (p_time.time_of_day().total_milliseconds() / 1000.0) + network_time_offset_ + 1.0; // up to 1 sec ok

    loginf << "ASTERIXPostprocessJob: doFutureTimestampsCheck: maximum time is "
           << String::timeStringFromDouble(tod_now_utc);

    for (auto& buf_it : buffers_)
    {
        buffer_size = buf_it.second->size();

        assert (obj_man.metaVariable(DBContent::meta_var_tod_id_.name()).existsIn(buf_it.first));

        DBContentVariable& tod_var = obj_man.metaVariable(DBContent::meta_var_tod_id_.name()).getFor(buf_it.first);

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

    string dbo_name;

    string datasource_var_name;
    string range_var_name;
    string azimuth_var_name;
    string altitude_var_name;
    string latitude_var_name;
    string longitude_var_name;

    // do radar position projection

    DBContentManager& dbo_man = COMPASS::instance().objectManager();
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
        dbo_name = buf_it.first;

        if (dbo_name != "CAT001" && dbo_name != "CAT048")
            continue;

        shared_ptr<Buffer> buffer = buf_it.second;
        unsigned int buffer_size = buffer->size();
        assert(buffer_size);

        assert (dbo_man.existsObject(dbo_name));
        DBContent& db_object = dbo_man.object(dbo_name);

        assert (db_object.hasVariable(DBContent::meta_var_datasource_id_.name()));
        assert (db_object.hasVariable(DBContent::var_radar_range_.name()));
        assert (db_object.hasVariable(DBContent::var_radar_azimuth_.name()));
        assert (db_object.hasVariable(DBContent::var_radar_altitude_.name()));
        assert (db_object.hasVariable(DBContent::meta_var_latitude_.name()));
        assert (db_object.hasVariable(DBContent::meta_var_longitude_.name()));

        DBContentVariable& datasource_var = db_object.variable(DBContent::meta_var_datasource_id_.name());
        DBContentVariable& range_var = db_object.variable(DBContent::var_radar_range_.name());
        DBContentVariable& azimuth_var = db_object.variable(DBContent::var_radar_azimuth_.name());
        DBContentVariable& altitude_var = db_object.variable(DBContent::var_radar_altitude_.name());
        DBContentVariable& latitude_var = db_object.variable(DBContent::meta_var_latitude_.name());
        DBContentVariable& longitude_var = db_object.variable(DBContent::meta_var_longitude_.name());

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
                assert (dbo_man.hasConfigDataSource(ds_id_it)
                        || dbo_man.hasDataSource(ds_id_it)); // creation done after in doDataSourcesBeforeInsert

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

                if (dbo_man.hasConfigDataSource(ds_id_it))
                {
                    dbContent::ConfigurationDataSource& data_source = dbo_man.configDataSource(ds_id_it);

                    if (data_source.info().contains("latitude")
                            && data_source.info().contains("longitude")
                            && data_source.info().contains("altitude"))
                    {
                        loginf << "ASTERIXPostprocessJob: run: adding proj ds " << ds_id_it
                               << " lat/long " << (double) data_source.info().at("latitude") << "," <<
                                  (double) data_source.info().at("longitude")
                               << " alt " << (double) data_source.info().at("altitude");

                        projection.addCoordinateSystem(ds_id_it, data_source.info().at("latitude"),
                                                       data_source.info().at("longitude"),
                                                       data_source.info().at("altitude"));
                    }
                    else
                        logerr << "ASTERIXPostprocessJob: run: config ds " << data_source.name()
                               << " defined but missing position info";
                }
                else if (dbo_man.hasDataSource(ds_id_it))
                {
                    dbContent::DBDataSource& data_source = dbo_man.dataSource(ds_id_it);

                    if (data_source.info().contains("latitude")
                            && data_source.info().contains("longitude")
                            && data_source.info().contains("altitude"))
                    {
                        loginf << "ASTERIXPostprocessJob: run: adding proj ds " << data_source.id()
                               << " lat/long " << (double) data_source.info().at("latitude") << "," <<
                                  (double) data_source.info().at("longitude")
                               << " alt " << (double) data_source.info().at("altitude");

                        projection.addCoordinateSystem(data_source.id(), data_source.info().at("latitude"),
                                                       data_source.info().at("longitude"),
                                                       data_source.info().at("altitude"));
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

    //        assert (dbo_man.existsMetaVariable(DBObject::meta_var_tod_id_.name()));
    //        assert (dbo_man.metaVariable(DBObject::meta_var_tod_id_.name()).existsIn(buf_it.first));

    //        DBOVariable& tod_var = dbo_man.metaVariable(DBObject::meta_var_tod_id_.name()).getFor(buf_it.first);

    //        Property prop {tod_var.name(), tod_var.dataType()};

    //        logdbg << "ASTERIXPostprocessJob: run: sorting by variable " << prop.name() << " " << prop.dataTypeString();

    //        assert (buf_it.second->hasProperty(prop));

    //        buf_it.second->sortByProperty(prop);
    //    }

}
