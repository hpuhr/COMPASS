﻿#include "asterixpostprocessjob.h"
#include "dbobjectmanager.h"
#include "dbobject.h"
#include "buffer.h"
#include "compass.h"
#include "projectionmanager.h"
#include "projection.h"
#include "json.hpp"

using namespace std;
using namespace nlohmann;

ASTERIXPostprocessJob::ASTERIXPostprocessJob(map<string, shared_ptr<Buffer>> buffers)
    : Job("ASTERIXPostprocessJob"),
      buffers_(move(buffers))
{

}

ASTERIXPostprocessJob::~ASTERIXPostprocessJob() { logdbg << "ASTERIXPostprocessJob: dtor"; }

void ASTERIXPostprocessJob::run()
{
    logdbg << "ASTERIXPostprocessJob: run";

    started_ = true;

    string dbo_name;

    string datasource_var_name;
    string range_var_name;
    string azimuth_var_name;
    string altitude_var_name;
    string latitude_var_name;
    string longitude_var_name;

    DBObjectManager& dbo_man = COMPASS::instance().objectManager();
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
        DBObject& db_object = dbo_man.object(dbo_name);

        assert (db_object.hasVariable(DBObject::meta_var_datasource_id_.name()));
        assert (db_object.hasVariable(DBObject::var_radar_range_.name()));
        assert (db_object.hasVariable(DBObject::var_radar_azimuth_.name()));
        assert (db_object.hasVariable(DBObject::var_radar_altitude_.name()));
        assert (db_object.hasVariable(DBObject::meta_var_latitude_.name()));
        assert (db_object.hasVariable(DBObject::meta_var_longitude_.name()));

        DBOVariable& datasource_var = db_object.variable(DBObject::meta_var_datasource_id_.name());
        DBOVariable& range_var = db_object.variable(DBObject::var_radar_range_.name());
        DBOVariable& azimuth_var = db_object.variable(DBObject::var_radar_azimuth_.name());
        DBOVariable& altitude_var = db_object.variable(DBObject::var_radar_altitude_.name());
        DBOVariable& latitude_var = db_object.variable(DBObject::meta_var_latitude_.name());
        DBOVariable& longitude_var = db_object.variable(DBObject::meta_var_longitude_.name());

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
                if (!dbo_man.hasDataSource(ds_id_it))
                {
                    if(dbo_man.canAddNewDataSourceFromConfig(ds_id_it))
                        dbo_man.addNewDataSource(ds_id_it);
                    else
                    {
                        logerr << "ASTERIXPostprocessJob: run: ds id " << ds_id_it << " unknown";
                        continue;
                    }
                }

                assert (dbo_man.hasDataSource(ds_id_it));

                DBContent::DBDataSource& data_source = dbo_man.dataSource(ds_id_it);

                if (data_source.info().contains("latitude")
                        && data_source.info().contains("longitude")
                        && data_source.info().contains("altitude"))
                {
                    loginf << "ASTERIXPostprocessJob: run: adding ds " << data_source.id()
                           << "lat/long " << (double) data_source.info().at("latitude") << "," <<
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

    done_ = true;
}