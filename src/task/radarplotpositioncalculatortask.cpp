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
//#include "ProjectionManager.h"

using namespace Utils;

RadarPlotPositionCalculatorTask::RadarPlotPositionCalculatorTask()
{
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

void RadarPlotPositionCalculatorTask::calculate (void)
{
    loginf << "RadarPlotPositionCalculatorTask: calculate: start";

    calculating_=true;

    num_loaded_=0;

//    assert (DBObjectManager::getInstance().existsDBObject(DBO_PLOTS));
//    DBObject *plot_obj = DBObjectManager::getInstance().getDBObject (DBO_PLOTS);

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

//    std::map <std::pair<unsigned char, unsigned char>, DataSource* > &data_sources =
//            ATSDB::getInstance().getDataSourceInstances (DBO_PLOTS);
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

    //ATSDB::getInstance().startReading (this, DBO_PLOTS, read_list, "DETECTION_TYPE!=0", 0);

    //connect (this, SIGNAL(processSignal(Buffer*)), this, SLOT(processSlot(Buffer*)));
}

void RadarPlotPositionCalculatorTask::readJobIntermediateSlot (std::shared_ptr<Buffer> buffer)
{
    assert (buffer);

    //    if (num_loaded_ == 0)
    //        emit processSignal (buffer);


    num_loaded_ += buffer->size();

    loginf << "RadarPlotPositionCalculatorTask: receive: received buffer, num_loaded " << num_loaded_;

    //emit processSignal (buffer);
}

void RadarPlotPositionCalculatorTask::readJobObsoleteSlot ()
{
    assert (false);
}

void RadarPlotPositionCalculatorTask::readJobDoneSlot()
{
    calculating_=false;
}


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
