/*
 * RadarPlotPositionCalculatorTask.cpp
 *
 *  Created on: Sep 17, 2013
 *      Author: sk
 */

#include "buffer.h"
#include "atsdb.h"
//#include "DataSource.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbovariableset.h"
#include "radarplotpositioncalculatortask.h"
#include "radarplotpositioncalculatortaskwidget.h"
#include "logger.h"
#include "propertylist.h"
#include "taskmanager.h"
//#include "ProjectionManager.h"

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

//    assert (plot_obj->hasVariable("REC_NUM"));
//    assert (plot_obj->hasVariable("SAC"));
//    assert (plot_obj->hasVariable("SIC"));
//    assert (plot_obj->hasVariable("POS_AZM_DEG"));
//    assert (plot_obj->hasVariable("POS_RANGE_NM"));
//    assert (plot_obj->hasVariable("MODEC_CODE_FT"));

//    DBOVariableSet read_list;
//    read_list.add(plot_obj->getVariable("REC_NUM"));
//    read_list.add(plot_obj->getVariable("SAC"));
//    read_list.add(plot_obj->getVariable("SIC"));
//    read_list.add(plot_obj->getVariable("POS_AZM_DEG"));
//    read_list.add(plot_obj->getVariable("POS_RANGE_NM"));
//    read_list.add(plot_obj->getVariable("MODEC_CODE_FT"));

    //SELECT * FROM sd_radar WHERE pos_range_nm IS NOT NULL AND pos_lat_deg IS NOT NULL;
    //ds 11  sac 50 sic 7 range 6.3508481015218  azm  313.267583462432 alt 29995   lat   47.9696340057652 long    12.9230976753774

    data_sources_ = db_object_->dataSources();

    DBOVariableSet read_set;
    read_set.add(*key_var_);
    read_set.add(*datasource_var_);
    read_set.add(*range_var_);
    read_set.add(*azimuth_var_);
    read_set.add(*altitude_var_);
    read_set.add(*latitude_var_);
    read_set.add(*longitude_var_);

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

    connect (db_object_, SIGNAL(newDataSignal(DBObject&)), this, SLOT(newDataSlot(DBObject&)));
    connect (db_object_, SIGNAL(loadingDoneSignal(DBObject&)), this, SLOT(loadingDoneSlot(DBObject&)));
//    connect (db_object_, SIGNAL(readJobDoneSlot), this, SLOT(readJobObsoleteSlot()));
    db_object_->load (read_set, false, false, nullptr, false);

    //ATSDB::getInstance().startReading (this, DBO_PLOTS, read_list, "DETECTION_TYPE!=0", 0);
}

void RadarPlotPositionCalculatorTask::newDataSlot (DBObject &object)
{
    loginf << "RadarPlotPositionCalculatorTask: newDataSlot";
}

void RadarPlotPositionCalculatorTask::loadingDoneSlot (DBObject &object)
{
    loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot";
}


//void RadarPlotPositionCalculatorTask::readJobIntermediateSlot (std::shared_ptr<Buffer> buffer)
//{
//    assert (buffer);

//    //    if (num_loaded_ == 0)
//    //        emit processSignal (buffer);


//    num_loaded_ += buffer->size();

//    loginf << "RadarPlotPositionCalculatorTask: receive: received buffer, num_loaded " << num_loaded_;

//    //emit processSignal (buffer);
//}

//void RadarPlotPositionCalculatorTask::readJobObsoleteSlot ()
//{
//    assert (false);
//}

//void RadarPlotPositionCalculatorTask::readJobDoneSlot()
//{
//    calculating_=false;
//}


bool RadarPlotPositionCalculatorTask::isCalculating ()
{
    return calculating_;
}

//void RadarPlotPositionCalculatorTask::processSlot (Buffer *buffer)
//{
//    loginf << "RadarPlotPositionCalculatorTask: processSlot";

//    ProjectionManager &proj_man = ProjectionManager::getInstance();

//    PropertyList update_buffer_list;
//    update_buffer_list.addProperty("POS_LAT_DEG", P_TYPE_DOUBLE);
//    update_buffer_list.addProperty("POS_LONG_DEG", P_TYPE_DOUBLE);
//    update_buffer_list.addProperty("REC_NUM", P_TYPE_UINT);

//    Buffer *update_buffer = new Buffer (update_buffer_list, DBO_PLOTS);

//    buffer->setIndex(0);
//    update_buffer->setIndex(0);

//    unsigned int size = buffer->getSize();

//    std::vector<void *>* adresses;
//    std::vector<void *>* update_adresses;

//    unsigned int rec_num;
//    unsigned char sac, sic;
//    double pos_azm_deg;
//    double pos_range_nm;
//    double pos_range_m;
//    double altitude_ft;
//    double altitude_m;
//    bool has_altitude;
//    //double altitude_angle;

//    std::map <std::pair<unsigned char, unsigned char>, DataSource* > &data_sources = ATSDB::getInstance().getDataSourceInstances (DBO_PLOTS);

//    std::pair<unsigned char, unsigned char> sac_sic_key;
//    double sys_x, sys_y;
//    double lat, lon;

//    loginf << "RadarPlotPositionCalculatorTask: receive: writing update_buffer";
//    for (unsigned int cnt=0; cnt < size; cnt++)
//    {
//        if (cnt != 0)
//        {
//            buffer->incrementIndex();
//        }

//        adresses = buffer->getAdresses();

//        rec_num= *((unsigned int*)adresses->at(0));
//        sac = *((unsigned char*)adresses->at(1));
//        sic = *((unsigned char*)adresses->at(2));
//        pos_azm_deg =  *((double*)adresses->at(3));
//        pos_range_nm = *((double*)adresses->at(4));
//        altitude_ft= *((int*)adresses->at(5));
//        has_altitude = isNan(P_TYPE_INT, adresses->at(5));

//        if (isNan(P_TYPE_DOUBLE, adresses->at(3)) || isNan(P_TYPE_DOUBLE, adresses->at(4)))
//        {
//            logerr << "RadarPlotPositionCalculatorTask: processSlot: position null";
//            continue;
//        }

//        if (isNan(P_TYPE_INT, adresses->at(5)))
//            altitude_ft=10000.0;

//        sac_sic_key.first = sac;
//        sac_sic_key.second= sic;

//        assert (data_sources.find(sac_sic_key) != data_sources.end());

//        pos_range_m = 1852.0 * pos_range_nm;

//        //loginf << " DB alt ft " << altitude_ft;

//        altitude_m = 0.3048 * altitude_ft;

//        //loginf << " DBO alt m " << altitude_m;
//        //altitude_m -= data_sources [sac_sic_key]->getAltitude ();

//        //altitude_angle = acos (altitude_m/pos_range_m);

//        data_sources [sac_sic_key]->calculateSystemCoordinates(pos_azm_deg, pos_range_m, altitude_m, has_altitude, sys_x, sys_y);
//        proj_man.cart2geo(sys_x, sys_y, lat, lon, false);

//        if (cnt != 0)
//        {
//            update_buffer->incrementIndex();
//        }
//        update_adresses = update_buffer->getAdresses();

//        *((double*)update_adresses->at(0)) = lat;
//        *((double*)update_adresses->at(1)) = lon;
//        *((unsigned int*)update_adresses->at(2)) = rec_num;
//        //loginf << "uga lat " << *((double*)update_adresses->at(1)) << " long " << *((double*)update_adresses->at(2));
//    }

//    loginf << "RadarPlotPositionCalculatorTask: receive: sending update_buffer";
//    ATSDB::getInstance().update(update_buffer);

//    loginf << "RadarPlotPositionCalculatorTask: processSlot: end";
//}
