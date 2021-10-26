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

#include "radarplotpositioncalculatortask.h"

#include <QApplication>
#include <QCoreApplication>
#include <QMessageBox>

#include "compass.h"
#include "buffer.h"
#include "dbinterface.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbodatasource.h"
#include "dbovariable.h"
#include "dbovariableset.h"
#include "logger.h"
#include "projection.h"
#include "projectionmanager.h"
#include "propertylist.h"
#include "radarplotpositioncalculatortaskwidget.h"
#include "stringconv.h"
#include "taskmanager.h"

using namespace Utils;

const std::string RadarPlotPositionCalculatorTask::DONE_PROPERTY_NAME =
    "radar_plot_positions_calculated";

RadarPlotPositionCalculatorTask::RadarPlotPositionCalculatorTask(const std::string& class_id,
                                                                 const std::string& instance_id,
                                                                 TaskManager& task_manager)
    : Task("RadarPlotPositionCalculatorTask", "Calculate Radar Plot Positions", true, false,
           task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_calc_radar_pos.json")
{
    tooltip_ =
        "Allows calculation of Radar plot position information based on the defined data sources.";

    qRegisterMetaType<std::shared_ptr<Buffer>>("std::shared_ptr<Buffer>");
    // qRegisterMetaType<DBObject>("DBObject");

    registerParameter("db_object_str", &db_object_str_, "");
    registerParameter("key_var_str", &key_var_str_, "");
    registerParameter("datasource_var_str", &datasource_var_str_, "");
    registerParameter("range_var_str", &range_var_str_, "");
    registerParameter("azimuth_var_str", &azimuth_var_str_, "");
    registerParameter("altitude_var_str", &altitude_var_str_, "");
    registerParameter("latitude_var_str", &latitude_var_str_, "");
    registerParameter("longitude_var_str", &longitude_var_str_, "");
}

RadarPlotPositionCalculatorTask::~RadarPlotPositionCalculatorTask() {}

TaskWidget* RadarPlotPositionCalculatorTask::widget()
{
    if (!widget_)
    {
        widget_.reset(new RadarPlotPositionCalculatorTaskWidget(*this));

        connect(&task_manager_, &TaskManager::expertModeChangedSignal, widget_.get(),
                &RadarPlotPositionCalculatorTaskWidget::expertModeChangedSlot);
    }

    assert(widget_);
    return widget_.get();
}

void RadarPlotPositionCalculatorTask::deleteWidget() { widget_.reset(nullptr); }

std::string RadarPlotPositionCalculatorTask::dbObjectStr() const { return db_object_str_; }

void RadarPlotPositionCalculatorTask::dbObjectStr(const std::string& db_object_str)
{
    loginf << "RadarPlotPositionCalculatorTask: dbObjectStr: " << db_object_str;

    db_object_str_ = db_object_str;

    assert(COMPASS::instance().objectManager().existsObject(db_object_str_));
    db_object_ = &COMPASS::instance().objectManager().object(db_object_str_);

    emit statusChangedSignal(name_);
}

std::string RadarPlotPositionCalculatorTask::keyVarStr() const { return key_var_str_; }

void RadarPlotPositionCalculatorTask::keyVarStr(const std::string& key_var_str)
{
    loginf << "RadarPlotPositionCalculatorTask: keyVarStr: " << key_var_str;

    key_var_str_ = key_var_str;

    if (db_object_ && key_var_str_.size())
    {
        assert(db_object_->hasVariable(key_var_str_));
        key_var_ = &db_object_->variable(key_var_str_);
    }
    else
        key_var_ = nullptr;

    emit statusChangedSignal(name_);
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
        assert(db_object_->hasVariable(datasource_var_str_));
        datasource_var_ = &db_object_->variable(datasource_var_str_);
    }
    else
        datasource_var_ = nullptr;

    emit statusChangedSignal(name_);
}

std::string RadarPlotPositionCalculatorTask::rangeVarStr() const { return range_var_str_; }

void RadarPlotPositionCalculatorTask::rangeVarStr(const std::string& range_var_str)
{
    loginf << "RadarPlotPositionCalculatorTask: rangeVarStr: " << range_var_str;

    range_var_str_ = range_var_str;

    if (db_object_ && range_var_str_.size())
    {
        assert(db_object_->hasVariable(range_var_str_));
        range_var_ = &db_object_->variable(range_var_str_);
    }
    else
        range_var_ = nullptr;

    emit statusChangedSignal(name_);
}

std::string RadarPlotPositionCalculatorTask::azimuthVarStr() const { return azimuth_var_str_; }

void RadarPlotPositionCalculatorTask::azimuthVarStr(const std::string& azimuth_var_str)
{
    loginf << "RadarPlotPositionCalculatorTask: azimuthVarStr: " << azimuth_var_str;

    azimuth_var_str_ = azimuth_var_str;

    if (db_object_ && azimuth_var_str_.size())
    {
        assert(db_object_->hasVariable(azimuth_var_str_));
        azimuth_var_ = &db_object_->variable(azimuth_var_str_);
    }
    else
        azimuth_var_ = nullptr;

    emit statusChangedSignal(name_);
}

std::string RadarPlotPositionCalculatorTask::altitudeVarStr() const { return altitude_var_str_; }

void RadarPlotPositionCalculatorTask::altitudeVarStr(const std::string& altitude_var_str)
{
    loginf << "RadarPlotPositionCalculatorTask: altitudeVarStr: " << altitude_var_str;

    altitude_var_str_ = altitude_var_str;

    if (db_object_ && altitude_var_str_.size())
    {
        assert(db_object_->hasVariable(altitude_var_str_));
        altitude_var_ = &db_object_->variable(altitude_var_str_);
    }
    else
        altitude_var_ = nullptr;

    emit statusChangedSignal(name_);
}

std::string RadarPlotPositionCalculatorTask::latitudeVarStr() const { return latitude_var_str_; }

void RadarPlotPositionCalculatorTask::latitudeVarStr(const std::string& latitude_var_str)
{
    loginf << "RadarPlotPositionCalculatorTask: latitudeVarStr: " << latitude_var_str;

    latitude_var_str_ = latitude_var_str;

    if (db_object_ && latitude_var_str_.size())
    {
        assert(db_object_->hasVariable(latitude_var_str_));
        latitude_var_ = &db_object_->variable(latitude_var_str_);
    }
    else
        latitude_var_ = nullptr;

    emit statusChangedSignal(name_);
}

std::string RadarPlotPositionCalculatorTask::longitudeVarStr() const { return longitude_var_str_; }

void RadarPlotPositionCalculatorTask::longitudeVarStr(const std::string& longitude_var_str)
{
    loginf << "RadarPlotPositionCalculatorTask: longitudeVarStr: " << longitude_var_str;

    longitude_var_str_ = longitude_var_str;

    if (db_object_ && longitude_var_str_.size())
    {
        assert(db_object_->hasVariable(longitude_var_str_));
        longitude_var_ = &db_object_->variable(longitude_var_str_);
    }
    else
        longitude_var_ = nullptr;

    emit statusChangedSignal(name_);
}

bool RadarPlotPositionCalculatorTask::checkPrerequisites()
{
    if (!COMPASS::instance().interface().ready())
        return false;

    if (COMPASS::instance().interface().hasProperty(DONE_PROPERTY_NAME))
        done_ =
            COMPASS::instance().interface().getProperty(DONE_PROPERTY_NAME) == "1";  // set done flag

    if (!COMPASS::instance().objectManager().existsObject("Radar"))
        return false;

    return COMPASS::instance().objectManager().object("Radar").hasData();
}

bool RadarPlotPositionCalculatorTask::isRecommended()
{
    if (!checkPrerequisites())
        return false;

    return !done_;
}

bool RadarPlotPositionCalculatorTask::isRequired() { return false; }

void RadarPlotPositionCalculatorTask::checkAndSetVariable(std::string& name_str, DBOVariable** var)
{
    // TODO rework to only asserting, check must be done before
    if (db_object_)
    {
        if (!db_object_->hasVariable(name_str))
        {
            loginf << "RadarPlotPositionCalculatorTask: checkAndSetVariable: var " << name_str
                   << " does not exist";
            name_str = "";
            var = nullptr;
        }
        else
        {
            *var = &db_object_->variable(name_str);
            loginf << "RadarPlotPositionCalculatorTask: checkAndSetVariable: var " << name_str
                   << " set";
            assert(var);
        }
    }
    else
    {
        loginf << "RadarPlotPositionCalculatorTask: checkAndSetVariable: dbobject null";
        name_str = "";
        var = nullptr;
    }
}

bool RadarPlotPositionCalculatorTask::canRun()
{
    if (!db_object_str_.size())
        return false;

    if (!COMPASS::instance().objectManager().existsObject(db_object_str_))
        return false;

    DBObject& object = COMPASS::instance().objectManager().object(db_object_str_);

    if (!object.loadable())
        return false;

    if (!object.count())
        return false;

    if (!key_var_str_.size() || !datasource_var_str_.size() || !range_var_str_.size() ||
        !azimuth_var_str_.size() || !altitude_var_str_.size() || !latitude_var_str_.size() ||
        !longitude_var_str_.size())
        return false;

    if (!object.hasVariable(key_var_str_) || !object.hasVariable(datasource_var_str_) ||
        !object.hasVariable(range_var_str_) || !object.hasVariable(azimuth_var_str_) ||
        !object.hasVariable(altitude_var_str_) || !object.hasVariable(latitude_var_str_) ||
        !object.hasVariable(longitude_var_str_))
        return false;

    return true;
}

void RadarPlotPositionCalculatorTask::run()
{
    loginf << "RadarPlotPositionCalculatorTask: run: start";

    assert(canRun());

    task_manager_.appendInfo("RadarPlotPositionCalculatorTask: started");

    start_time_ = boost::posix_time::microsec_clock::local_time();

    DBObjectManager& obj_man = COMPASS::instance().objectManager();

    calculating_ = true;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    std::string msg = "Loading object data.";
    msg_box_ = new QMessageBox;
    assert(msg_box_);
    msg_box_->setWindowTitle("Calculating Radar Plot Positions");
    msg_box_->setText(msg.c_str());
    msg_box_->setStandardButtons(QMessageBox::NoButton);
    msg_box_->show();

    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    num_loaded_ = 0;

    assert(obj_man.existsObject(db_object_str_));
    db_object_ = &COMPASS::instance().objectManager().object(db_object_str_);
    assert(db_object_);

    if (key_var_str_.size())
        checkAndSetVariable(key_var_str_, &key_var_);
    if (datasource_var_str_.size())
        checkAndSetVariable(datasource_var_str_, &datasource_var_);
    if (range_var_str_.size())
        checkAndSetVariable(range_var_str_, &range_var_);
    if (azimuth_var_str_.size())
        checkAndSetVariable(azimuth_var_str_, &azimuth_var_);
    if (altitude_var_str_.size())
        checkAndSetVariable(altitude_var_str_, &altitude_var_);
    if (latitude_var_str_.size())
        checkAndSetVariable(latitude_var_str_, &latitude_var_);
    if (longitude_var_str_.size())
        checkAndSetVariable(longitude_var_str_, &longitude_var_);

    assert(db_object_);

    target_report_count_ = db_object_->count();

    assert(key_var_);
    assert(datasource_var_);
    assert(range_var_);
    assert(azimuth_var_);
    assert(altitude_var_);
    assert(latitude_var_);
    assert(longitude_var_);

    DBOVariableSet read_set;
    read_set.add(*key_var_);
    read_set.add(*datasource_var_);
    read_set.add(*range_var_);
    read_set.add(*azimuth_var_);
    read_set.add(*altitude_var_);
    read_set.add(*latitude_var_);
    read_set.add(*longitude_var_);

    connect(db_object_, &DBObject::newDataSignal, this,
            &RadarPlotPositionCalculatorTask::newDataSlot);
    connect(db_object_, &DBObject::loadingDoneSignal, this,
            &RadarPlotPositionCalculatorTask::loadingDoneSlot);

    db_object_->load(read_set, false, false, nullptr, false);  //"0,100000"
}

void RadarPlotPositionCalculatorTask::newDataSlot(DBObject& object)
{
    if (target_report_count_ != 0)
    {
        assert(msg_box_);
        size_t loaded_cnt = db_object_->loadedCount();

        float done_percent = 100.0 * loaded_cnt / target_report_count_;
        std::string msg =
            "Loading object data: " + String::doubleToStringPrecision(done_percent, 2) + "%";
        msg_box_->setText(msg.c_str());

        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }
}

void RadarPlotPositionCalculatorTask::loadingDoneSlot(DBObject& object)
{
    loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot: starting calculation";

    disconnect(db_object_, &DBObject::newDataSignal, this,
               &RadarPlotPositionCalculatorTask::newDataSlot);
    disconnect(db_object_, &DBObject::loadingDoneSignal, this,
               &RadarPlotPositionCalculatorTask::loadingDoneSlot);

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
    //    std::pair<double, double> lat_long = data_sources
    //    [sac_sic_key]->calculateWorldCoordinates(pos_azm_deg, pos_range_m, altitude_m);
    //
    //    loginf << "UGA  lat " << lat_long.first << " long " << lat_long.second;
    //    loginf << "UGAs lat " << 47.9696340057652 << " long " << 12.9230976753774;

    //    return;

    ProjectionManager& proj_man = ProjectionManager::instance();

    assert(proj_man.hasCurrentProjection());
    Projection& projection = proj_man.currentProjection();

    loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot: projection method '"
           << projection.name() << "'";

    std::shared_ptr<Buffer> read_buffer = db_object_->data();
    unsigned int read_size = read_buffer->size();
    assert(read_size);

    //    std::string latitude_var_dbname = latitude_var_->currentDBColumn().name();
    //    std::string longitude_var_dbname = longitude_var_->currentDBColumn().name();
    //    std::string keyvar_var_dbname = key_var_->currentDBColumn().name();

    PropertyList update_buffer_list;
    update_buffer_list.addProperty(latitude_var_str_, PropertyDataType::DOUBLE);
    update_buffer_list.addProperty(longitude_var_str_, PropertyDataType::DOUBLE);
    update_buffer_list.addProperty(key_var_str_, PropertyDataType::INT);

    std::shared_ptr<Buffer> update_buffer =
        std::make_shared<Buffer>(update_buffer_list, db_object_->name());

    int rec_num;
    int sensor_id;
    // unsigned char sac, sic;
    // bool has_position;
    double pos_azm_deg;
    double pos_azm_rad;
    double pos_range_nm;
    double pos_range_m;
    double altitude_ft;
    // double altitude_m;
    bool has_altitude;
    // double altitude_angle;

    assert(db_object_->hasDataSources());

    //    std::pair<unsigned char, unsigned char> sac_sic_key;
    // double sys_x, sys_y;
    double lat, lon;
    unsigned int update_cnt = 0;

    // double x1, y1, z1;
    // VecB pos;

    assert(msg_box_);
    float done_percent;
    std::string msg;

    loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot: writing update_buffer";
    bool ret;

    size_t transformation_errors = 0;

    for (unsigned int cnt = 0; cnt < read_size; cnt++)
    {
        if (cnt % 50000 == 0 && target_report_count_ != 0)
        {
            done_percent = 100.0 * cnt / target_report_count_;
            msg =
                "Processing object data: " + String::doubleToStringPrecision(done_percent, 2) + "%";
            msg_box_->setText(msg.c_str());

            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }

        if (read_buffer->get<int>(key_var_str_).isNull(cnt))
        {
            logerr << "RadarPlotPositionCalculatorTask: loadingDoneSlot: key null";
            continue;
        }
        rec_num = read_buffer->get<int>(key_var_str_).get(cnt);

        if (read_buffer->get<int>(datasource_var_str_).isNull(cnt))
        {
            logerr << "RadarPlotPositionCalculatorTask: loadingDoneSlot: data source null";
            continue;
        }
        sensor_id = read_buffer->get<int>(datasource_var_str_).get(cnt);

        // sac = *((unsigned char*)adresses->at(1));
        // sic = *((unsigned char*)adresses->at(2));

        if (read_buffer->get<double>(azimuth_var_str_).isNull(cnt) ||
            read_buffer->get<double>(range_var_str_).isNull(cnt))
        {
            logdbg << "RadarPlotPositionCalculatorTask: loadingDoneSlot: position null";
            continue;
        }

        pos_azm_deg = read_buffer->get<double>(azimuth_var_str_).get(cnt);
        pos_range_nm = read_buffer->get<double>(range_var_str_).get(cnt);

        has_altitude = !read_buffer->get<int>(altitude_var_str_).isNull(cnt);
        if (has_altitude)
            altitude_ft = read_buffer->get<int>(altitude_var_str_).get(cnt);
        else
            altitude_ft = 0.0;  // has to assumed in projection later on

        if (!db_object_->hasDataSource(sensor_id))
        {
            logerr << "RadarPlotPositionCalculatorTask: loadingDoneSlot: sensor id " << sensor_id
                   << " unkown";
            transformation_errors++;
            continue;
        }

        DBODataSource& data_source = db_object_->getDataSource(sensor_id);

        if (!data_source.hasLatitude() || !data_source.hasLongitude())
        {
            transformation_errors++;
            continue;
        }

        pos_azm_rad = pos_azm_deg * DEG2RAD;

        pos_range_m = 1852.0 * pos_range_nm;

        // altitude_m = 0.3048 * altitude_ft;

        //        if (use_ogr_proj)
        //        {
        //            ret = data_source.calculateOGRSystemCoordinates(pos_azm_rad, pos_range_m,
        //            has_altitude, altitude_ft,
        //                                                            sys_x, sys_y);
        //            if (ret)
        //                ret = proj_man.ogrCart2Geo(sys_x, sys_y, lat, lon);
        //        }

        //        if (use_sdl_proj)
        //        {
        //            t_CPos grs_pos;

        //            ret = data_source.calculateSDLGRSCoordinates(pos_azm_rad, pos_range_m,
        //            has_altitude, altitude_ft, grs_pos); if (ret)
        //            {
        //                t_GPos geo_pos;

        //                ret = proj_man.sdlGRS2Geo(grs_pos, geo_pos);

        //                if (ret)
        //                {
        //                    lat = geo_pos.latitude * RAD2DEG;
        //                    lon = geo_pos.longitude * RAD2DEG;
        //                    //lat = geo_pos.latitude; what to do with altitude?
        //                }
        //            }
        //        }

        //        if (use_rs2g_proj)
        //        {
        ////            float rho; // (m)
        ////            float theta; // (deg)

        //            x1 = pos_range_m * sin(pos_azm_rad);
        //            y1 = pos_range_m * cos(pos_azm_rad);

        //            if (has_altitude)
        //                z1 = altitude_ft * FT2M;
        //            else
        //                z1 = -1000.0;

        //            logdbg << "local x " << x1 << " y " << y1 << " z " << z1;

        //            ret = data_source.calculateRadSlt2Geocentric(x1, y1, z1, pos);
        //            if (ret)
        //            {
        //                logdbg << "geoc x " << pos[0] << " y " << pos[1] << " z " << pos[2];

        //                ret = geocentric2Geodesic(pos);

        //                lat = pos [0];
        //                lon = pos [1];

        //                logdbg << "geod x " << pos[0] << " y " << pos[1];
        //                //what to do with altitude?
        //            }
        //        }

        if (!projection.hasCoordinateSystem(data_source.id()))
        {
            assert(data_source.hasLatitude());
            assert(data_source.hasLongitude());
            assert(data_source.hasAltitude());
            projection.addCoordinateSystem(data_source.id(), data_source.latitude(),
                                           data_source.longitude(), data_source.altitude());
        }

        ret = projection.polarToWGS84(data_source.id(), pos_azm_rad, pos_range_m, has_altitude,
                                      altitude_ft, lat, lon);

        if (!ret)
        {
            transformation_errors++;
            continue;
        }

        update_buffer->get<double>(latitude_var_str_).set(update_cnt, lat);
        update_buffer->get<double>(longitude_var_str_).set(update_cnt, lon);
        update_buffer->get<int>(key_var_str_).set(update_cnt, rec_num);
        update_cnt++;

        // loginf << "uga cnt " << update_cnt << " rec_num " << rec_num << " lat " << lat << " long
        // " << lon;
    }

    loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot: update_buffer size "
           << update_buffer->size() << ", " << transformation_errors << " transformation errors";

    msg_box_->close();
    delete msg_box_;
    msg_box_ = nullptr;

    if (!update_buffer->size())
    {
        std::string text =
            "There were " + std::to_string(transformation_errors) +
            " skipped coordinates with transformation errors, no data available for insertion.";

        QMessageBox msgBox;
        msgBox.setText(text.c_str());
        msgBox.exec();

        return;
    }

    if (transformation_errors)
    {
        QApplication::restoreOverrideCursor();

        QMessageBox::StandardButton reply;

        std::string question =
            "There were " + std::to_string(transformation_errors) +
            " skipped coordinates with transformation errors, " +
            std::to_string(update_buffer->size()) +
            " coordinates were projected correctly. Do you want to insert the data?";

        reply = QMessageBox::question(nullptr, "Insert Data", question.c_str(),
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::No)
        {
            loginf
                << "RadarPlotPositionCalculatorTask: loadingDoneSlot: aborted by user because of "
                   "transformation errors";
            task_manager_.appendInfo(
                "RadarPlotPositionCalculatorTask: aborted by user because of "
                "transformation errors");
            return;
        }
        else
            task_manager_.appendWarning(
                "RadarPlotPositionCalculatorTask: continued by user ignoring" +
                std::to_string(transformation_errors) + " transformation errors");

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    }
    else
        task_manager_.appendInfo("RadarPlotPositionCalculatorTask: no transformation errors");

    msg_box_ = new QMessageBox;
    assert(msg_box_);
    msg_box_->setWindowTitle("Calculating Radar Plot Positions");
    msg = "Writing object data";
    msg_box_->setText(msg.c_str());
    msg_box_->setStandardButtons(QMessageBox::NoButton);
    msg_box_->show();

    DBOVariableSet list;
    list.add(*latitude_var_);
    list.add(*longitude_var_);
    list.add(*key_var_);

    db_object_->updateData(*key_var_, list, update_buffer);

    connect(db_object_, &DBObject::updateDoneSignal, this,
            &RadarPlotPositionCalculatorTask::updateDoneSlot);
    connect(db_object_, &DBObject::updateProgressSignal, this,
            &RadarPlotPositionCalculatorTask::updateProgressSlot);

    loginf << "RadarPlotPositionCalculatorTask: loadingDoneSlot: end";
}

void RadarPlotPositionCalculatorTask::updateProgressSlot(float percent)
{
    logdbg << "RadarPlotPositionCalculatorTask: updateProgressSlot: " << percent;

    assert(msg_box_);
    std::string msg = "Writing object data: " + String::doubleToStringPrecision(percent, 2) + "%";
    msg_box_->setText(msg.c_str());
}

void RadarPlotPositionCalculatorTask::updateDoneSlot(DBObject& object)
{
    loginf << "RadarPlotPositionCalculatorTask: updateDoneSlot";

    disconnect(db_object_, &DBObject::updateDoneSignal, this,
               &RadarPlotPositionCalculatorTask::updateDoneSlot);
    disconnect(db_object_, &DBObject::updateProgressSignal, this,
               &RadarPlotPositionCalculatorTask::updateProgressSlot);

    assert(msg_box_);
    msg_box_->close();
    delete msg_box_;
    msg_box_ = nullptr;

    job_ptr_ = nullptr;
    db_object_->clearData();

    stop_time_ = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration time_diff = stop_time_ - start_time_;
    std::string elapsed_time_str =
        String::timeStringFromDouble(time_diff.total_milliseconds() / 1000.0, false);

    done_ = true;
    COMPASS::instance().interface().setProperty(DONE_PROPERTY_NAME, "1");

    task_manager_.appendSuccess("RadarPlotPositionCalculatorTask: done after " + elapsed_time_str);

    QApplication::restoreOverrideCursor();

    msg_box_ = new QMessageBox;
    assert(msg_box_);
    msg_box_->setWindowTitle("Calculating Radar Plot Positions");
    msg_box_->setText("Writing of object data done.");
    msg_box_->setStandardButtons(QMessageBox::Ok);

    if (show_done_summary_)
        msg_box_->exec();

    delete msg_box_;
    msg_box_ = nullptr;

    emit doneSignal(name_);
}

bool RadarPlotPositionCalculatorTask::isCalculating() { return calculating_; }
