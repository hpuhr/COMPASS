#include "asterixpostprocessjob.h"
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

    const string datasource_var_str {"DS ID"};
    const string range_var_str {"Range"}; // "Nautical Mile"
    const string azimuth_var_str {"Azimuth"};  // Deg
    const string altitude_var_str {"Mode C Code"};  // ft
    const string latitude_var_str {"Latitude"};
    const string longitude_var_str {"Longitude"};

    string datasource_col_str;
    string range_col_str;
    string azimuth_col_str;
    string altitude_col_str;
    string latitude_col_str;
    string longitude_col_str;

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

        assert (db_object.hasVariable(datasource_var_str));
        assert (db_object.hasVariable(range_var_str));
        assert (db_object.hasVariable(azimuth_var_str));
        assert (db_object.hasVariable(altitude_var_str));
        assert (db_object.hasVariable(latitude_var_str));
        assert (db_object.hasVariable(longitude_var_str));

        DBOVariable& datasource_var_ = db_object.variable(datasource_var_str);
        DBOVariable& range_var_ = db_object.variable(range_var_str);
        DBOVariable& azimuth_var_ = db_object.variable(azimuth_var_str);
        DBOVariable& altitude_var_ = db_object.variable(altitude_var_str);
        DBOVariable& latitude_var_ = db_object.variable(latitude_var_str);
        DBOVariable& longitude_var_ = db_object.variable(longitude_var_str);

        datasource_col_str = datasource_var_.dbColumnName();
        range_col_str = range_var_.dbColumnName();
        azimuth_col_str = azimuth_var_.dbColumnName();
        altitude_col_str = altitude_var_.dbColumnName();
        latitude_col_str = latitude_var_.dbColumnName();
        longitude_col_str = longitude_var_.dbColumnName();

        assert (datasource_var_.dataType() == PropertyDataType::UINT);
        assert (range_var_.dataType() == PropertyDataType::DOUBLE);
        assert (azimuth_var_.dataType() == PropertyDataType::DOUBLE);
        assert (altitude_var_.dataType() == PropertyDataType::FLOAT);
        assert (latitude_var_.dataType() == PropertyDataType::DOUBLE);
        assert (longitude_var_.dataType() == PropertyDataType::DOUBLE);

        assert (buffer->has<unsigned int>(datasource_col_str));
        assert (buffer->has<double>(range_col_str));
        assert (buffer->has<double>(azimuth_col_str));
        assert (buffer->has<float>(altitude_col_str));
        assert (!buffer->has<double>(latitude_col_str));
        assert (!buffer->has<double>(longitude_col_str));

        buffer->addProperty(latitude_col_str, PropertyDataType::DOUBLE);
        buffer->addProperty(longitude_col_str, PropertyDataType::DOUBLE);

        transformation_errors = 0;

        NullableVector<unsigned int>& datasource_vec = buffer->get<unsigned int>(datasource_col_str);
        NullableVector<double>& range_vec = buffer->get<double>(range_col_str);
        NullableVector<double>& azimuth_vec = buffer->get<double>(azimuth_col_str);
        NullableVector<float>& altitude_vec = buffer->get<float>(altitude_col_str);
        NullableVector<double>& latitude_vec = buffer->get<double>(latitude_col_str);
        NullableVector<double>& longitude_vec = buffer->get<double>(longitude_col_str);


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
