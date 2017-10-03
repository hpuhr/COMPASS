/*
 * RadarPlotPositionCalculatorTask.cpp
 *
 *  Created on: Sep 17, 2013
 *      Author: sk
 */

#include "buffer.h"
#include "atsdb.h"
#include "dbodatasource.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbovariable.h"
#include "dbovariableset.h"
#include "dbtablecolumn.h"
#include "radarplotpositioncalculatortask.h"
#include "radarplotpositioncalculatortaskwidget.h"
#include "logger.h"
#include "propertylist.h"
#include "taskmanager.h"
#include "projectionmanager.h"
#include "updatebufferdbjob.h"
#include "jobmanager.h"

using namespace Utils;

RadarPlotPositionCalculatorTask::RadarPlotPositionCalculatorTask(const std::string &class_id, const std::string &instance_id, TaskManager* task_manager)
    : Configurable (class_id, instance_id, task_manager)
{
    qRegisterMetaType<std::shared_ptr<Buffer>>("std::shared_ptr<Buffer>");

    registerParameter("db_object_str", &db_object_str_, "");
    registerParameter("key_var_str", &key_var_str_, "");
    registerParameter("datasource_var_str", &datasource_var_str_, "");
    registerParameter("range_var_str", &range_var_str_, "");
    registerParameter("azimuth_var_str", &azimuth_var_str_, "");
    registerParameter("altitude_var_str", &altitude_var_str_, "");
    registerParameter("latitude_var_str", &latitude_var_str_, "");
    registerParameter("longitude_var_str", &longitude_var_str_, "");
}

RadarPlotPositionCalculatorTask::~RadarPlotPositionCalculatorTask()
{
    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

}

RadarPlotPositionCalculatorTaskWidget* RadarPlotPositionCalculatorTask::widget()
{
    if (!widget_)
    {
        widget_ = new RadarPlotPositionCalculatorTaskWidget (*this);
    }

    assert (widget_);
    return widget_;
}

std::string RadarPlotPositionCalculatorTask::dbObjectStr() const
{
    return db_object_str_;
}

void RadarPlotPositionCalculatorTask::dbObjectStr(const std::string &db_object_str)
{
    db_object_str_ = db_object_str;

    assert (ATSDB::instance().objectManager().existsObject(db_object_str_));
    db_object_ = &ATSDB::instance().objectManager().object(db_object_str_);
}

std::string RadarPlotPositionCalculatorTask::keyVarStr() const
{
    return key_var_str_;
}

void RadarPlotPositionCalculatorTask::keyVarStr(const std::string &key_var_str)
{
    key_var_str_ = key_var_str;
    if (db_object_)
    {
        assert (db_object_->hasVariable(key_var_str_));
        key_var_ = &db_object_->variable(key_var_str_);
    }
}

std::string RadarPlotPositionCalculatorTask::datasourceVarStr() const
{
    return datasource_var_str_;
}

void RadarPlotPositionCalculatorTask::datasourceVarStr(const std::string &datasource_var_str)
{
    datasource_var_str_ = datasource_var_str;
    if (db_object_)
    {
        assert (db_object_->hasVariable(datasource_var_str_));
        datasource_var_ = &db_object_->variable(datasource_var_str_);
    }
}

std::string RadarPlotPositionCalculatorTask::rangeVarStr() const
{
    return range_var_str_;
}

void RadarPlotPositionCalculatorTask::rangeVarStr(const std::string &range_var_str)
{
    range_var_str_ = range_var_str;
    if (db_object_)
    {
        assert (db_object_->hasVariable(range_var_str_));
        range_var_ = &db_object_->variable(range_var_str_);
    }
}

std::string RadarPlotPositionCalculatorTask::azimuthVarStr() const
{
    return azimuth_var_str_;
}

void RadarPlotPositionCalculatorTask::azimuthVarStr(const std::string &azimuth_var_str)
{
    azimuth_var_str_ = azimuth_var_str;
    if (db_object_)
    {
        assert (db_object_->hasVariable(azimuth_var_str_));
        azimuth_var_ = &db_object_->variable(azimuth_var_str_);
    }
}

std::string RadarPlotPositionCalculatorTask::altitudeVarStr() const
{
    return altitude_var_str_;
}

void RadarPlotPositionCalculatorTask::altitudeVarStr(const std::string &altitude_var_str)
{
    altitude_var_str_ = altitude_var_str;
    if (db_object_)
    {
        assert (db_object_->hasVariable(altitude_var_str_));
        altitude_var_ = &db_object_->variable(altitude_var_str_);
    }
}

std::string RadarPlotPositionCalculatorTask::latitudeVarStr() const
{
    return latitude_var_str_;
}

void RadarPlotPositionCalculatorTask::latitudeVarStr(const std::string &latitude_var_str)
{
    latitude_var_str_ = latitude_var_str;
    if (db_object_)
    {
        assert (db_object_->hasVariable(latitude_var_str_));
        latitude_var_ = &db_object_->variable(latitude_var_str_);
    }
}

std::string RadarPlotPositionCalculatorTask::longitudeVarStr() const
{
    return longitude_var_str_;
}

void RadarPlotPositionCalculatorTask::longitudeVarStr(const std::string &longitude_var_str)
{
    longitude_var_str_ = longitude_var_str;
    if (db_object_)
    {
        assert (db_object_->hasVariable(longitude_var_str_));
        longitude_var_ = &db_object_->variable(longitude_var_str_);
    }
}

void RadarPlotPositionCalculatorTask::checkAndSetVariable (std::string &name_str, DBOVariable** var)
{
    if (db_object_)
    {
        if (!db_object_->hasVariable(name_str))
        {
            loginf << "RadarPlotPositionCalculatorTask::checkAndSetVariable: var " << name_str << " does not exist";
            name_str = "";
            var = nullptr;
        }
        else
        {
            *var = &db_object_->variable(name_str);
            loginf << "RadarPlotPositionCalculatorTask::checkAndSetVariable: var " << name_str << " set";
            assert (var);
        }
    }
    else
        loginf << "RadarPlotPositionCalculatorTask::checkAndSetVariable: dbobject null";
}

void RadarPlotPositionCalculatorTask::calculate ()
{
    loginf << "RadarPlotPositionCalculatorTask: calculate: start";

    calculating_=true;

    num_loaded_=0;

    if (db_object_str_.size())
    {
        if (!ATSDB::instance().objectManager().existsObject(db_object_str_))
            db_object_str_="";
        else
            db_object_ = &ATSDB::instance().objectManager().object(db_object_str_);
    }
    assert (db_object_);

    if (key_var_str_.size())
        checkAndSetVariable (key_var_str_, &key_var_);
    if (datasource_var_str_.size())
        checkAndSetVariable (datasource_var_str_, &datasource_var_);
    if (range_var_str_.size())
        checkAndSetVariable (range_var_str_, &range_var_);
    if (azimuth_var_str_.size())
        checkAndSetVariable (azimuth_var_str_, &azimuth_var_);
    if (altitude_var_str_.size())
        checkAndSetVariable (altitude_var_str_, &altitude_var_);
    if (latitude_var_str_.size())
        checkAndSetVariable (latitude_var_str_, &latitude_var_);
    if (longitude_var_str_.size())
        checkAndSetVariable (longitude_var_str_, &longitude_var_);

    assert (db_object_);

    assert (key_var_);
    assert (datasource_var_);
    assert (range_var_);
    assert (azimuth_var_);
    assert (altitude_var_);
    assert (latitude_var_);
    assert (longitude_var_);

    data_sources_ = db_object_->dataSources();

    DBOVariableSet read_set;
    read_set.add(*key_var_);
    read_set.add(*datasource_var_);
    read_set.add(*range_var_);
    read_set.add(*azimuth_var_);
    read_set.add(*altitude_var_);
    read_set.add(*latitude_var_);
    read_set.add(*longitude_var_);

    //connect (db_object_, SIGNAL(newDataSignal(DBObject&)), this, SLOT(newDataSlot(DBObject&)));
    connect (db_object_, SIGNAL(loadingDoneSignal(DBObject&)), this, SLOT(loadingDoneSlot(DBObject&)));

    db_object_->load (read_set, false, false, nullptr, false); //"0,100000"

    //ATSDB::getInstance().startReading (this, DBO_PLOTS, read_list, "DETECTION_TYPE!=0", 0);
}

//void RadarPlotPositionCalculatorTask::newDataSlot (DBObject &object)
//{
//    loginf << "RadarPlotPositionCalculatorTask: newDataSlot";
//}

void RadarPlotPositionCalculatorTask::loadingDoneSlot (DBObject &object)
{
    loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot: starting calculation";

    //
    //    std::pair<unsigned char, unsigned char> sac_sic_key;
    //
    //    sac_sic_key.first = 50;
    //    sac_sic_key.second = 7;
    ////
    ////        double pos_azm_deg = 180.0;
    ////        double pos_range_m = 1852.0 * 0.5;
    ////        double altitude_m = 0.3048  * 1400.0;
    //
    //    double pos_azm_deg = 313.267583462432;
    //    double pos_range_m = 1852.0 * 6.3508481015218;
    //    double altitude_m = 0.3048  * 29995;
    //    //altitude_m -= data_sources [sac_sic_key]->getAltitude ();
    //
    //    std::pair<double, double> lat_long = data_sources [sac_sic_key]->calculateWorldCoordinates(pos_azm_deg, pos_range_m, altitude_m);
    //
    //    loginf << "UGA  lat " << lat_long.first << " long " << lat_long.second;
    //    loginf << "UGAs lat " << 47.9696340057652 << " long " << 12.9230976753774;

    //    return;

    ProjectionManager &proj_man = ProjectionManager::instance();

    std::shared_ptr<Buffer> read_buffer = object.data();
    unsigned int read_size = read_buffer->size();
    assert (read_size);

    std::string latitude_var_dbname = latitude_var_->currentDBColumn().name();
    std::string longitude_var_dbname = longitude_var_->currentDBColumn().name();
    std::string keyvar_var_dbname = key_var_->currentDBColumn().name();

    PropertyList update_buffer_list;
    update_buffer_list.addProperty(latitude_var_dbname, PropertyDataType::DOUBLE);
    update_buffer_list.addProperty(longitude_var_dbname, PropertyDataType::DOUBLE);
    update_buffer_list.addProperty(keyvar_var_dbname, PropertyDataType::INT);

    std::shared_ptr<Buffer> update_buffer = std::shared_ptr<Buffer> (new Buffer (update_buffer_list,db_object_->name()));

    int rec_num;
    int sensor_id;
    //unsigned char sac, sic;
    //bool has_position;
    double pos_azm_deg;
    double pos_range_nm;
    double pos_range_m;
    double altitude_ft;
    double altitude_m;
    bool has_altitude;
    //double altitude_angle;

    //    std::map <std::pair<unsigned char, unsigned char>, DataSource* > &data_sources = ATSDB::getInstance().getDataSourceInstances (DBO_PLOTS);
    assert (data_sources_.size());

    //    std::pair<unsigned char, unsigned char> sac_sic_key;
    double sys_x, sys_y;
    double lat, lon;
    unsigned int update_cnt=0;

    loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot: writing update_buffer";
    for (unsigned int cnt=0; cnt < read_size; cnt++)
    {
        if (read_buffer->getInt(key_var_str_).isNone(cnt))
        {
            logerr << "RadarPlotPositionCalculatorTask: loadingDoneSlot: key null";
            continue;
        }
        rec_num = read_buffer->getInt(key_var_str_).get(cnt);

        if (read_buffer->getInt(datasource_var_str_).isNone(cnt))
        {
            logerr << "RadarPlotPositionCalculatorTask: loadingDoneSlot: data source null";
            continue;
        }
        sensor_id = read_buffer->getInt(datasource_var_str_).get(cnt);

        //sac = *((unsigned char*)adresses->at(1));
        //sic = *((unsigned char*)adresses->at(2));

        if (read_buffer->getDouble(azimuth_var_str_).isNone(cnt) || read_buffer->getDouble(range_var_str_).isNone(cnt))
        {
            logdbg << "RadarPlotPositionCalculatorTask: loadingDoneSlot: position null";
            continue;
        }

        pos_azm_deg =  read_buffer->getDouble(azimuth_var_str_).get(cnt);
        pos_range_nm =  read_buffer->getDouble(range_var_str_).get(cnt);

        has_altitude = !read_buffer->getInt(altitude_var_str_).isNone(cnt);
        if (has_altitude)
            altitude_ft = read_buffer->getInt(altitude_var_str_).get(cnt);
        else
            altitude_ft = 10000.0; // HACK


    //        sac_sic_key.first = sac;
    //        sac_sic_key.second= sic;


        if (data_sources_.find(sensor_id) == data_sources_.end())
        {
            logerr << "RadarPlotPositionCalculatorTask: loadingDoneSlot: sensor id " << sensor_id << " unkown";
            continue;
        }

        pos_range_m = 1852.0 * pos_range_nm;

        //loginf << " DB alt ft " << altitude_ft;

        altitude_m = 0.3048 * altitude_ft;

    //        //loginf << " DBO alt m " << altitude_m;
        altitude_m -= data_sources_.at(sensor_id).altitude();

            //altitude_angle = acos (altitude_m/pos_range_m);

        data_sources_.at(sensor_id).calculateSystemCoordinates(pos_azm_deg, pos_range_m, altitude_m, has_altitude, sys_x, sys_y);
        proj_man.cart2geo(sys_x, sys_y, lat, lon);

        update_buffer->getDouble(latitude_var_dbname).set(update_cnt, lat);
        update_buffer->getDouble(longitude_var_dbname).set(update_cnt, lon);
        update_buffer->getInt(keyvar_var_dbname).set(update_cnt, rec_num);
        update_cnt++;

        //loginf << "uga cnt " << update_cnt << " rec_num " << rec_num << " lat " << lat << " long " << lon;
    }

    loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot: update_buffer size " << update_buffer->size();

    UpdateBufferDBJob *job = new UpdateBufferDBJob(ATSDB::instance().interface(), object, *key_var_, update_buffer);
    job_ptr_ = std::shared_ptr<UpdateBufferDBJob> (job);
    connect (job, SIGNAL(doneSignal()), this, SLOT(updateDoneSlot()), Qt::QueuedConnection);

    JobManager::instance().addDBJob(job_ptr_);

    loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot: end";
}

void RadarPlotPositionCalculatorTask::updateDoneSlot ()
{
    loginf << "RadarPlotPositionCalculatorTask: updateDoneSlot";

    job_ptr_ = nullptr;
}

bool RadarPlotPositionCalculatorTask::isCalculating ()
{
    return calculating_;
}
