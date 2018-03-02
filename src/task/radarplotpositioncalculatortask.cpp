/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
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
#include "jobmanager.h"
#include "stringconv.h"

#include <QCoreApplication>
#include <QMessageBox>

using namespace Utils;

RadarPlotPositionCalculatorTask::RadarPlotPositionCalculatorTask(const std::string& class_id,
                                                                 const std::string& instance_id,
                                                                 TaskManager* task_manager)
    : Configurable (class_id, instance_id, task_manager)
{
    qRegisterMetaType<std::shared_ptr<Buffer>>("std::shared_ptr<Buffer>");
    //qRegisterMetaType<DBObject>("DBObject");

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

void RadarPlotPositionCalculatorTask::dbObjectStr(const std::string& db_object_str)
{
    loginf << "RadarPlotPositionCalculatorTask: dbObjectStr: " << db_object_str;

    db_object_str_ = db_object_str;

    assert (ATSDB::instance().objectManager().existsObject(db_object_str_));
    db_object_ = &ATSDB::instance().objectManager().object(db_object_str_);
}

std::string RadarPlotPositionCalculatorTask::keyVarStr() const
{
    return key_var_str_;
}

void RadarPlotPositionCalculatorTask::keyVarStr(const std::string& key_var_str)
{
    loginf << "RadarPlotPositionCalculatorTask: keyVarStr: " << key_var_str;

    key_var_str_ = key_var_str;

    if (db_object_ && key_var_str_.size())
    {
        assert (db_object_->hasVariable(key_var_str_));
        key_var_ = &db_object_->variable(key_var_str_);
    }
    else
        key_var_ = nullptr;
}

std::string RadarPlotPositionCalculatorTask::datasourceVarStr() const
{
    return datasource_var_str_;
}

void RadarPlotPositionCalculatorTask::datasourceVarStr(const std::string& datasource_var_str)
{
    loginf << "RadarPlotPositionCalculatorTask: datasourceVarStr: " << datasource_var_str;

    datasource_var_str_ = datasource_var_str;

    if (db_object_ && datasource_var_str_.size())
    {
        assert (db_object_->hasVariable(datasource_var_str_));
        datasource_var_ = &db_object_->variable(datasource_var_str_);
    }
    else
        datasource_var_ = nullptr;
}

std::string RadarPlotPositionCalculatorTask::rangeVarStr() const
{
    return range_var_str_;
}

void RadarPlotPositionCalculatorTask::rangeVarStr(const std::string& range_var_str)
{
    loginf << "RadarPlotPositionCalculatorTask: rangeVarStr: " << range_var_str;

    range_var_str_ = range_var_str;

    if (db_object_ && range_var_str_.size())
    {
        assert (db_object_->hasVariable(range_var_str_));
        range_var_ = &db_object_->variable(range_var_str_);
    }
    else
        range_var_ = nullptr;
}

std::string RadarPlotPositionCalculatorTask::azimuthVarStr() const
{
    return azimuth_var_str_;
}

void RadarPlotPositionCalculatorTask::azimuthVarStr(const std::string& azimuth_var_str)
{
    loginf << "RadarPlotPositionCalculatorTask: azimuthVarStr: " << azimuth_var_str;

    azimuth_var_str_ = azimuth_var_str;

    if (db_object_ && azimuth_var_str_.size())
    {
        assert (db_object_->hasVariable(azimuth_var_str_));
        azimuth_var_ = &db_object_->variable(azimuth_var_str_);
    }
    else
        azimuth_var_ = nullptr;
}

std::string RadarPlotPositionCalculatorTask::altitudeVarStr() const
{
    return altitude_var_str_;
}

void RadarPlotPositionCalculatorTask::altitudeVarStr(const std::string& altitude_var_str)
{
    loginf << "RadarPlotPositionCalculatorTask: altitudeVarStr: " << altitude_var_str;

    altitude_var_str_ = altitude_var_str;

    if (db_object_ && altitude_var_str_.size())
    {
        assert (db_object_->hasVariable(altitude_var_str_));
        altitude_var_ = &db_object_->variable(altitude_var_str_);
    }
    else
        altitude_var_ = nullptr;
}

std::string RadarPlotPositionCalculatorTask::latitudeVarStr() const
{
    return latitude_var_str_;
}

void RadarPlotPositionCalculatorTask::latitudeVarStr(const std::string& latitude_var_str)
{
    loginf << "RadarPlotPositionCalculatorTask: latitudeVarStr: " << latitude_var_str;

    latitude_var_str_ = latitude_var_str;

    if (db_object_ && latitude_var_str_.size())
    {
        assert (db_object_->hasVariable(latitude_var_str_));
        latitude_var_ = &db_object_->variable(latitude_var_str_);
    }
    else
        latitude_var_ = nullptr;
}

std::string RadarPlotPositionCalculatorTask::longitudeVarStr() const
{
    return longitude_var_str_;
}

void RadarPlotPositionCalculatorTask::longitudeVarStr(const std::string& longitude_var_str)
{
    loginf << "RadarPlotPositionCalculatorTask: longitudeVarStr: " << longitude_var_str;

    longitude_var_str_ = longitude_var_str;

    if (db_object_ && longitude_var_str_.size())
    {
        assert (db_object_->hasVariable(longitude_var_str_));
        longitude_var_ = &db_object_->variable(longitude_var_str_);
    }
    else
        longitude_var_ = nullptr;
}

void RadarPlotPositionCalculatorTask::checkAndSetVariable (std::string& name_str, DBOVariable** var)
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

    std::string msg = "Loading object data.";
    msg_box_ = new QMessageBox;
    assert (msg_box_);
    msg_box_->setText(msg.c_str());
    msg_box_->setStandardButtons(QMessageBox::NoButton);
    msg_box_->show();

    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

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

    target_report_count_ = db_object_->count();

    assert (key_var_);
    assert (datasource_var_);
    assert (range_var_);
    assert (azimuth_var_);
    assert (altitude_var_);
    assert (latitude_var_);
    assert (longitude_var_);

    DBOVariableSet read_set;
    read_set.add(*key_var_);
    read_set.add(*datasource_var_);
    read_set.add(*range_var_);
    read_set.add(*azimuth_var_);
    read_set.add(*altitude_var_);
    read_set.add(*latitude_var_);
    read_set.add(*longitude_var_);

    connect (db_object_, SIGNAL(newDataSignal(DBObject&)), this, SLOT(newDataSlot(DBObject&)));
    connect (db_object_, SIGNAL(loadingDoneSignal(DBObject&)), this, SLOT(loadingDoneSlot(DBObject&)));

    db_object_->load (read_set, false, false, nullptr, false); //"0,100000"
}

void RadarPlotPositionCalculatorTask::newDataSlot (DBObject& object)
{
    if (target_report_count_ != 0)
    {
        assert (msg_box_);
        size_t loaded_cnt = db_object_->loadedCount();

        float done_percent = 100.0*loaded_cnt/target_report_count_;
        std::string msg = "Loading object data: " + String::doubleToStringPrecision(done_percent, 2) + "%";
        msg_box_->setText(msg.c_str());

        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }
}

void RadarPlotPositionCalculatorTask::loadingDoneSlot (DBObject& object)
{
    loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot: starting calculation";

    if (calculated_) // TODO: done signal comes twice?
        return;

    disconnect (db_object_, SIGNAL(newDataSignal(DBObject&)), this, SLOT(newDataSlot(DBObject&)));
    disconnect (db_object_, SIGNAL(loadingDoneSignal(DBObject&)), this, SLOT(loadingDoneSlot(DBObject&)));

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

    bool use_ogr_proj = proj_man.useOGRProjection();
    bool use_sdl_proj = proj_man.useSDLProjection();
    bool use_rs2g_proj = proj_man.useRS2GProjection();

    loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot: projection method sdl " << use_sdl_proj
           << " ogr " << use_ogr_proj << " rs2g " << use_rs2g_proj;

    assert (use_ogr_proj || use_sdl_proj || use_rs2g_proj);

    std::map<int, DBODataSource>& data_sources = db_object_->dataSources();
    for (auto& ds_it : data_sources)
        assert (ds_it.second.isFinalized()); // has to be done before

    std::shared_ptr<Buffer> read_buffer = db_object_->data();
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
    double pos_azm_rad;
    double pos_range_nm;
    double pos_range_m;
    double altitude_ft;
    //double altitude_m;
    bool has_altitude;
    //double altitude_angle;

    assert (data_sources.size());

    //    std::pair<unsigned char, unsigned char> sac_sic_key;
    double sys_x, sys_y;
    double lat, lon;
    unsigned int update_cnt=0;

    double x1, y1, z1;
    VecB pos;

    assert (msg_box_);
    float done_percent;
    std::string msg;

    loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot: writing update_buffer";
    bool ret;

    size_t transformation_errors = 0;

    for (unsigned int cnt=0; cnt < read_size; cnt++)
    {
        if (cnt % 50000 == 0 && target_report_count_ != 0)
        {
            done_percent = 100.0*cnt/target_report_count_;
            msg = "Processing object data: " + String::doubleToStringPrecision(done_percent, 2) + "%";
            msg_box_->setText(msg.c_str());

            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }

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
            altitude_ft = 0.0; // has to assumed in projection later on

        if (data_sources.find(sensor_id) == data_sources.end())
        {
            logerr << "RadarPlotPositionCalculatorTask: loadingDoneSlot: sensor id " << sensor_id << " unkown";
            continue;
        }

        pos_azm_rad = pos_azm_deg * DEG2RAD;

        pos_range_m = 1852.0 * pos_range_nm;

        //altitude_m = 0.3048 * altitude_ft;

        if (use_ogr_proj)
        {
            ret = data_sources.at(sensor_id).calculateOGRSystemCoordinates(pos_azm_rad, pos_range_m, has_altitude,
                                                                           altitude_ft, sys_x, sys_y);
            if (ret)
                ret = proj_man.ogrCart2Geo(sys_x, sys_y, lat, lon);
        }

        if (use_sdl_proj)
        {
            t_CPos grs_pos;

            ret = data_sources.at(sensor_id).calculateSDLGRSCoordinates(pos_azm_rad, pos_range_m, has_altitude,
                                                                        altitude_ft, grs_pos);
            if (ret)
            {
                t_GPos geo_pos;

                ret = proj_man.sdlGRS2Geo(grs_pos, geo_pos);

                if (ret)
                {
                    lat = geo_pos.latitude * RAD2DEG;
                    lon = geo_pos.longitude * RAD2DEG;
                    //lat = geo_pos.latitude; what to do with altitude?
                }
            }
        }

        if (use_rs2g_proj)
        {
//            float rho; // (m)
//            float theta; // (deg)

            x1 = pos_range_m * sin(pos_azm_rad);
            y1 = pos_range_m * cos(pos_azm_rad);

            if (has_altitude)
                z1 = altitude_ft * FT2M;
            else
                z1 = -1000.0;

            logdbg << "local x " << x1 << " y " << y1 << " z " << z1;

            ret = data_sources.at(sensor_id).calculateRadSlt2Geocentric(x1, y1, z1, pos);
            if (ret)
            {
                logdbg << "geoc x " << pos[0] << " y " << pos[1] << " z " << pos[2];

                ret = geocentric2Geodesic(pos);

                lat = pos [0];
                lon = pos [1];

                logdbg << "geod x " << pos[0] << " y " << pos[1];
                //what to do with altitude?
            }
        }

        if (!ret)
        {
            transformation_errors++;
            continue;
        }

        update_buffer->getDouble(latitude_var_dbname).set(update_cnt, lat);
        update_buffer->getDouble(longitude_var_dbname).set(update_cnt, lon);
        update_buffer->getInt(keyvar_var_dbname).set(update_cnt, rec_num);
        update_cnt++;

        //loginf << "uga cnt " << update_cnt << " rec_num " << rec_num << " lat " << lat << " long " << lon;
    }

    loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot: update_buffer size " << update_buffer->size()
           << ", " <<  transformation_errors << " transformation errors";

    msg_box_->close();
    delete msg_box_;
    msg_box_ = nullptr;

    if (transformation_errors)
    {
        QMessageBox::StandardButton reply;

        std::string question = "There were "+std::to_string(transformation_errors)
                +" skipped coordinates with transformation errors, "
                +std::to_string(update_buffer->size())
                +" coordinates were projected correctly. Do you want to insert the data?";

        reply = QMessageBox::question(nullptr, "Insert Data", question.c_str(), QMessageBox::Yes|QMessageBox::No);

        if (reply == QMessageBox::No)
        {
            loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot: aborted by user because of errors";
            calculated_ = true;
            return;
        }
    }

    msg_box_ = new QMessageBox;
    assert (msg_box_);
    msg = "Writing object data, this can take up to a few minutes.";
    msg_box_->setText(msg.c_str());
    msg_box_->setStandardButtons(QMessageBox::NoButton);
    msg_box_->show();

    db_object_->updateData(*key_var_, update_buffer);

    connect (db_object_, SIGNAL(updateDoneSignal(DBObject&)), this, SLOT(updateDoneSlot(DBObject&)));

//    UpdateBufferDBJob *job = new UpdateBufferDBJob(ATSDB::instance().interface(), *db_object_, *key_var_,
//                                                   update_buffer);
//    job_ptr_ = std::shared_ptr<UpdateBufferDBJob> (job);
//    connect (job, SIGNAL(doneSignal()), this, SLOT(updateDoneSlot()), Qt::QueuedConnection);

//    JobManager::instance().addDBJob(job_ptr_);

    calculated_ = true;
    loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot: end";
}

void RadarPlotPositionCalculatorTask::updateDoneSlot (DBObject& object)
{
    loginf << "RadarPlotPositionCalculatorTask: updateDoneSlot";

    disconnect (db_object_, SIGNAL(updateDoneSignal(DBObject&)), this, SLOT(updateDoneSlot(DBObject&)));

    assert (msg_box_);
    msg_box_->close();
    delete msg_box_;
    msg_box_ = nullptr;

    job_ptr_ = nullptr;
    db_object_->clearData();

    msg_box_ = new QMessageBox;
    assert (msg_box_);
    msg_box_->setText("Plot position calculation successfull.\nIt is recommended to force a post-processing step now.");
    msg_box_->setStandardButtons(QMessageBox::Ok);
    msg_box_->exec();

    delete msg_box_;
    msg_box_ = nullptr;

    if (widget_)
        widget_->calculationDoneSlot();
}

void RadarPlotPositionCalculatorTask::updateBufferJobStatusSlot ()
{

}

bool RadarPlotPositionCalculatorTask::isCalculating ()
{
    return calculating_;
}
