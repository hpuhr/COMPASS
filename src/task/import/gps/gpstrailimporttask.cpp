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

#include "gpstrailimporttask.h"
#include "gpstrailimporttaskdialog.h"
#include "compass.h"
#include "dbinterface.h"
#include "savedfile.h"
#include "stringconv.h"
#include "taskmanager.h"
#include "files.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/variable/variable.h"
#include "datasourcemanager.h"
#include "buffer.h"
#include "dbcontent/variable/variableset.h"
#include "dbcontent/variable/metavariable.h"
#include "util/number.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <nmeaparse/nmea.h>

#include <QApplication>
#include <QMessageBox>

const std::string DONE_PROPERTY_NAME = "gps_trail_imported";
const float tod_24h = 24 * 60 * 60;

using namespace Utils;
using namespace std;
using namespace nmea;

GPSTrailImportTask::GPSTrailImportTask(const std::string& class_id, const std::string& instance_id,
                                       TaskManager& task_manager)
    : Task("GPSTrailImportTask", "Import GPS Trail", true, false, task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_import_gps.json")
{
    tooltip_ = "Allows importing of GPS trails as NMEA into the opened database.";

    registerParameter("current_filename", &current_filename_, "");

    registerParameter("ds_name", &ds_name_, "GPS Trail");
    registerParameter("ds_sac", &ds_sac_, 0);
    registerParameter("ds_sic", &ds_sic_, 0);

    registerParameter("tod_offset", &tod_offset_, 0);

    registerParameter("set_mode_3a_code", &set_mode_3a_code_, false);
    registerParameter("mode_3a_code", &mode_3a_code_, 0);

    registerParameter("set_target_address", &set_target_address_, false);
    registerParameter("target_address", &target_address_, 0);

    registerParameter("set_callsign", &set_callsign_, false);
    registerParameter("callsign", &callsign_, "");

    //registerParameter("line_id", &line_id_, 0); always defaults to 0

    createSubConfigurables();

    if (current_filename_.size())
        parseCurrentFile();
}

GPSTrailImportTask::~GPSTrailImportTask()
{
}

void GPSTrailImportTask::generateSubConfigurable(const std::string& class_id,
                                                 const std::string& instance_id)
{
        throw std::runtime_error("GPSTrailImportTask: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

GPSTrailImportTaskDialog* GPSTrailImportTask::dialog()
{
    if (!dialog_)
    {
        dialog_.reset(new GPSTrailImportTaskDialog(*this));

        connect(dialog_.get(), &GPSTrailImportTaskDialog::importSignal,
                this, &GPSTrailImportTask::dialogImportSlot);

        connect(dialog_.get(), &GPSTrailImportTaskDialog::doneSignal,
                this, &GPSTrailImportTask::dialogDoneSlot);
    }

    assert(dialog_);
    return dialog_.get();
}

void GPSTrailImportTask::importFilename(const std::string& filename)
{
    loginf << "GPSTrailImportTask: importFilename: filename '" << filename << "'";

    current_filename_ = filename;

    parseCurrentFile();

        if (dialog_)
        {
//            dialog_->updateFileListSlot();
//            widget_->updateText();
        }

    emit statusChangedSignal(name_);
}

bool GPSTrailImportTask::checkPrerequisites()
{
    if (!COMPASS::instance().interface().ready())  // must be connected
        return false;

    if (COMPASS::instance().interface().hasProperty(DONE_PROPERTY_NAME))
        done_ = COMPASS::instance().interface().getProperty(DONE_PROPERTY_NAME) == "1";

    if (!COMPASS::instance().dbContentManager().existsDBContent("RefTraj"))
        return false;

    return true;
}

bool GPSTrailImportTask::isRecommended()
{
    return false;
}

bool GPSTrailImportTask::isRequired() { return false; }

std::string GPSTrailImportTask::currentError() const
{
    return current_error_;
}

std::string GPSTrailImportTask::dsName() const
{
    return ds_name_;
}

void GPSTrailImportTask::dsName(const std::string& ds_name)
{
    loginf << "GPSTrailImportTask: dsName: value '" << ds_name << "'";

    ds_name_ = ds_name;
}

unsigned int GPSTrailImportTask::dsSAC() const
{
    return ds_sac_;
}

void GPSTrailImportTask::dsSAC(unsigned int ds_sac)
{
    loginf << "GPSTrailImportTask: dsSAC: value " << ds_sac;

    ds_sac_ = ds_sac;
}

unsigned int GPSTrailImportTask::dsSIC() const
{
    return ds_sic_;
}

void GPSTrailImportTask::dsSIC(unsigned int ds_sic)
{
    loginf << "GPSTrailImportTask: dsSIC: value " << ds_sic;

    ds_sic_ = ds_sic;
}

float GPSTrailImportTask::todOffset() const
{
    return tod_offset_;
}

void GPSTrailImportTask::todOffset(float tod_offset)
{
    loginf << "GPSTrailImportTask: todOffset: value " << tod_offset;

    tod_offset_ = tod_offset;
}

bool GPSTrailImportTask::setMode3aCode() const
{
    return set_mode_3a_code_;
}

void GPSTrailImportTask::setMode3aCode(bool value)
{
    loginf << "GPSTrailImportTask: setMode3aCode: value " << value;

    set_mode_3a_code_ = value;
}

unsigned int GPSTrailImportTask::mode3aCode() const
{
    return mode_3a_code_;
}

void GPSTrailImportTask::mode3aCode(unsigned int value)
{
    loginf << "GPSTrailImportTask: mode3aCode: value " << value;

    mode_3a_code_ = value;
}

bool GPSTrailImportTask::setTargetAddress() const
{
    return set_target_address_;
}

void GPSTrailImportTask::setTargetAddress(bool value)
{
    loginf << "GPSTrailImportTask: setTargetAddress: value " << value;

    set_target_address_ = value;
}

unsigned int GPSTrailImportTask::targetAddress() const
{
    return target_address_;
}

void GPSTrailImportTask::targetAddress(unsigned int target_address)
{
    loginf << "GPSTrailImportTask: targetAddress: value " << target_address;

    target_address_ = target_address;
}

bool GPSTrailImportTask::setCallsign() const
{
    return set_callsign_;
}

void GPSTrailImportTask::setCallsign(bool value)
{
    loginf << "GPSTrailImportTask: setCallsign: value " << value;

    set_callsign_ = value;
}

std::string GPSTrailImportTask::callsign() const
{
    return callsign_;
}

void GPSTrailImportTask::callsign(const std::string& callsign)
{
    loginf << "GPSTrailImportTask: callsign: value '" << callsign << "'";

    callsign_ = callsign;
}

unsigned int GPSTrailImportTask::lineID() const
{
    return line_id_;
}

void GPSTrailImportTask::lineID(unsigned int line_id)
{
    loginf << "GPSTrailImportTask: lineID: value " << line_id;

    line_id_ = line_id;
}

std::string GPSTrailImportTask::currentText() const
{
    return current_text_;
}


bool GPSTrailImportTask::canImportFile()
{
    if (!current_filename_.size())
        return false;

    if (!Files::fileExists(current_filename_))
    {
        loginf << "GPSTrailImportTask: canImportFile: not possible since file '" << current_filename_
               << "does not exist";
        return false;
    }

    return gps_fixes_.size(); // only if fixes exist
}

void GPSTrailImportTask::parseCurrentFile ()
{
    loginf << "GPSTrailImportTask: parseCurrentFile: file '" << current_filename_ << "'";

    current_error_ = "";
    current_text_ = "";

    gps_fixes_.clear();
    quality_counts_.clear();
    gps_fixes_cnt_ = 0;
    gps_fixes_skipped_quality_cnt_ = 0;
    gps_fixes_skipped_time_cnt_ = 0;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    NMEAParser parser;
    GPSService gps(parser);
    parser.log = false;

    time_t last_tod;

    //cout << "Fix  Sats  Sig\t\tSpeed    Dir  Lat         , Lon           Accuracy" << endl;
    // Handle any changes to the GPS Fix... This is called whenever it's updated.
    gps.onUpdate += [&gps, this, &last_tod](){
        //        cout << (gps.fix.locked() ? "[*] " : "[ ] ") << setw(2) << setfill(' ') << gps.fix.trackingSatellites << "/" << setw(2) << setfill(' ') << gps.fix.visibleSatellites << " ";
        //        cout << fixed << setprecision(2) << setw(5) << setfill(' ') << gps.fix.almanac.averageSNR() << " dB   ";
        //        cout << fixed << setprecision(2) << setw(6) << setfill(' ') << gps.fix.speed << " km/h [" << GPSFix::travelAngleToCompassDirection(gps.fix.travelAngle, true) << "]  ";
        //        cout << fixed << setprecision(6) << gps.fix.latitude << "\xF8 " "N, " << gps.fix.longitude << "\xF8 " "E" << "  ";
        //        cout << "+/- " << setprecision(1) << gps.fix.horizontalAccuracy() << "m  ";
        //        cout << endl;

        ++gps_fixes_cnt_;

        if (gps.fix.quality == 0)
        {
            ++gps_fixes_skipped_quality_cnt_;
            return;
        }

        if (gps_fixes_.size() && last_tod == gps.fix.timestamp.getTime())
        {
            ++gps_fixes_skipped_time_cnt_;
            return;
        }

        quality_counts_[gps.fix.quality] += 1;

        gps_fixes_.push_back(gps.fix);

        last_tod = gps.fix.timestamp.getTime();
    };

    // From a file
    string line;
    ifstream file(current_filename_);
    unsigned int line_cnt = 0;

    while (getline(file, line))
    {
        try
        {
            if (line.back() == '\r')
                line.pop_back();

            parser.readLine(line);
        }
        catch (NMEAParseError& e)
        {
            logerr << "GPSTrailImportTask: parseCurrentFile: line " << line_cnt << ": error '" << e.message << "'";
            if (!current_error_.size())
                current_error_ = "Line "+to_string(line_cnt)+": "+e.message;
            else
                current_error_ += "\nLine "+to_string(line_cnt)+": "+e.message;
            // You can keep feeding data to the gps service...
            // The previous data is ignored and the parser is reset.
        }
        ++line_cnt;
    }

    // Show the final fix information
    //cout << gps.fix.toString() << endl;

    stringstream ss;

    ss << "Parsed " << line_cnt << " lines.\n";

    if (gps_fixes_cnt_)
    {
        ss << "Read " << gps_fixes_cnt_ << " fixes.\n";
        ss << "Skipped " << gps_fixes_skipped_quality_cnt_
           << " (" << String::percentToString(100.0*gps_fixes_skipped_quality_cnt_/gps_fixes_cnt_) << "%)"
           << " because of quality.\n";
        ss << "Skipped " << gps_fixes_skipped_time_cnt_
           << " (" << String::percentToString(100.0*gps_fixes_skipped_time_cnt_/gps_fixes_cnt_) << "%)"
           << " because of same time.\n";
        ss << "Got " << gps_fixes_.size()
           << " (" << String::percentToString(100.0*gps_fixes_.size()/gps_fixes_cnt_) << "%) fixes.\n";

        if (quality_counts_.size())
        {
            ss << "\nQuality:\n";

            for (auto& qual_it : quality_counts_)
            {
                if (quality_labels.count(qual_it.first))
                    ss << quality_labels.at(qual_it.first) << ": " << qual_it.second
                       << " (" << String::percentToString(100.0*qual_it.second/gps_fixes_.size()) << "%)\n";
                else
                    ss << "Unknown ("<< qual_it.first << "): " << qual_it.second
                       << " (" << String::percentToString(100.0*qual_it.second/gps_fixes_.size()) << "%)\n";
            }
        }
    }
    else
        ss << "Found 0 fixes.";

    current_text_ = ss.str();

    loginf << "GPSTrailImportTask: parseCurrentFile: parsed " << gps_fixes_.size() << " fixes in "
           << line_cnt << " lines";

    QApplication::restoreOverrideCursor();
}

bool GPSTrailImportTask::canRun() { return canImportFile(); }

void GPSTrailImportTask::run()
{
    loginf << "GPSTrailImportTask: run: filename '" << current_filename_ << " fixes " << gps_fixes_.size();

    task_manager_.appendInfo("GPSTrailImportTask: import of file '" + current_filename_ +
                             "' started");

    assert (gps_fixes_.size());
    assert (!buffer_);

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    string dbcontent_name = "RefTraj";
    assert (dbcontent_man.existsDBContent(dbcontent_name));

    assert (dbcontent_man.metaVariable(DBContent::meta_var_sac_id_.name()).existsIn(dbcontent_name));
    assert (dbcontent_man.metaVariable(DBContent::meta_var_sic_id_.name()).existsIn(dbcontent_name));
    assert (dbcontent_man.metaVariable(DBContent::meta_var_datasource_id_.name()).existsIn(dbcontent_name));
    assert (dbcontent_man.metaVariable(DBContent::meta_var_line_id_.name()).existsIn(dbcontent_name));
    assert (dbcontent_man.metaVariable(DBContent::meta_var_tod_.name()).existsIn(dbcontent_name));
    assert (dbcontent_man.metaVariable(DBContent::meta_var_latitude_.name()).existsIn(dbcontent_name));
    assert (dbcontent_man.metaVariable(DBContent::meta_var_longitude_.name()).existsIn(dbcontent_name));
    assert (dbcontent_man.metaVariable(DBContent::meta_var_m3a_.name()).existsIn(dbcontent_name));
    assert (dbcontent_man.metaVariable(DBContent::meta_var_ta_.name()).existsIn(dbcontent_name));
    assert (dbcontent_man.metaVariable(DBContent::meta_var_ti_.name()).existsIn(dbcontent_name));
    assert (dbcontent_man.metaVariable(DBContent::meta_var_vx_.name()).existsIn(dbcontent_name));
    assert (dbcontent_man.metaVariable(DBContent::meta_var_vy_.name()).existsIn(dbcontent_name));
    assert (dbcontent_man.metaVariable(DBContent::meta_var_ground_speed_.name()).existsIn(dbcontent_name));
    assert (dbcontent_man.metaVariable(DBContent::meta_var_track_angle_.name()).existsIn(dbcontent_name));

    loginf << "GPSTrailImportTask: run: getting variables";

    using namespace dbContent;

    Variable& sac_var = dbcontent_man.metaVariable(DBContent::meta_var_sac_id_.name()).getFor(dbcontent_name);
    Variable& sic_var = dbcontent_man.metaVariable(DBContent::meta_var_sic_id_.name()).getFor(dbcontent_name);
    Variable& ds_id_var = dbcontent_man.metaVariable(DBContent::meta_var_datasource_id_.name()).getFor(dbcontent_name);
    Variable& line_id_var = dbcontent_man.metaVariable(DBContent::meta_var_line_id_.name()).getFor(dbcontent_name);
    Variable& tod_var = dbcontent_man.metaVariable(DBContent::meta_var_tod_.name()).getFor(dbcontent_name);
    Variable& lat_var = dbcontent_man.metaVariable(DBContent::meta_var_latitude_.name()).getFor(dbcontent_name);
    Variable& long_var = dbcontent_man.metaVariable(DBContent::meta_var_longitude_.name()).getFor(dbcontent_name);
    Variable& m3a_var = dbcontent_man.metaVariable(DBContent::meta_var_m3a_.name()).getFor(dbcontent_name);
    Variable& ta_var = dbcontent_man.metaVariable(DBContent::meta_var_ta_.name()).getFor(dbcontent_name);
    Variable& ti_var = dbcontent_man.metaVariable(DBContent::meta_var_ti_.name()).getFor(dbcontent_name);
    Variable& vx_var = dbcontent_man.metaVariable(DBContent::meta_var_vx_.name()).getFor(dbcontent_name);
    Variable& vy_var = dbcontent_man.metaVariable(DBContent::meta_var_vy_.name()).getFor(dbcontent_name);
    Variable& speed_var = dbcontent_man.metaVariable(DBContent::meta_var_ground_speed_.name()).getFor(dbcontent_name);
    Variable& track_angle_var = dbcontent_man.metaVariable(DBContent::meta_var_track_angle_.name()).getFor(dbcontent_name);



    PropertyList properties;
    properties.addProperty(sac_var.name(), PropertyDataType::UCHAR);
    properties.addProperty(sic_var.name(), PropertyDataType::UCHAR);
    properties.addProperty(ds_id_var.name(), PropertyDataType::UINT);
    properties.addProperty(line_id_var.name(), PropertyDataType::UINT);
    properties.addProperty(tod_var.name(), PropertyDataType::FLOAT);
    properties.addProperty(lat_var.name(), PropertyDataType::DOUBLE);
    properties.addProperty(long_var.name(), PropertyDataType::DOUBLE);

    if (set_mode_3a_code_)
        properties.addProperty(m3a_var.name(), PropertyDataType::UINT);

    if (set_target_address_)
        properties.addProperty(ta_var.name(), PropertyDataType::UINT);

    if (set_callsign_)
        properties.addProperty(ti_var.name(), PropertyDataType::STRING);

    properties.addProperty(vx_var.name(), PropertyDataType::DOUBLE);
    properties.addProperty(vy_var.name(), PropertyDataType::DOUBLE);
    properties.addProperty(speed_var.name(), PropertyDataType::DOUBLE);
    properties.addProperty(track_angle_var.name(), PropertyDataType::DOUBLE);

    loginf << "GPSTrailImportTask: run: creating buffer";

    buffer_ = make_shared<Buffer>(properties, dbcontent_name);

    NullableVector<unsigned char>& sac_vec = buffer_->get<unsigned char>(sac_var.name());
    NullableVector<unsigned char>& sic_vec = buffer_->get<unsigned char>(sic_var.name());
    NullableVector<unsigned int>& ds_id_vec = buffer_->get<unsigned int>(ds_id_var.name());
    NullableVector<unsigned int>& line_id_vec = buffer_->get<unsigned int>(line_id_var.name());
    NullableVector<float>& tod_vec = buffer_->get<float>(tod_var.name());
    NullableVector<double>& lat_vec = buffer_->get<double>(lat_var.name());
    NullableVector<double>& long_vec = buffer_->get<double>(long_var.name());

    NullableVector<double>& vx_vec = buffer_->get<double>(vx_var.name());
    NullableVector<double>& vy_vec = buffer_->get<double>(vy_var.name());
    NullableVector<double>& speed_vec = buffer_->get<double>(speed_var.name());
    NullableVector<double>& track_angle_vec = buffer_->get<double>(track_angle_var.name());

    unsigned int cnt = 0;
    unsigned int ds_id = Number::dsIdFrom(ds_sac_, ds_sic_);

    // config data source
    {
        DataSourceManager& src_man = COMPASS::instance().dataSourceManager();

        if (!src_man.hasDBDataSource(ds_id))
            src_man.addNewDataSource(ds_id);

        assert (src_man.hasDBDataSource(ds_id));

        dbContent::DBDataSource& src = src_man.dbDataSource(ds_id);

        src.name(ds_name_);
        src.dsType(dbcontent_name); // same as dstype

    }

    float tod;
    double speed_ms, track_angle_rad, vx, vy;

    loginf << "GPSTrailImportTask: run: filling buffer";

    for (auto& fix_it : gps_fixes_)
    {
        tod = fix_it.timestamp.hour*3600.0 + fix_it.timestamp.min*60.0+fix_it.timestamp.sec;

        tod += tod_offset_;

        // check for out-of-bounds because of midnight-jump
        while (tod < 0.0f)
            tod += tod_24h;
        while (tod > tod_24h)
            tod -= tod_24h;

        assert(tod >= 0.0f);
        assert(tod <= tod_24h);

        sac_vec.set(cnt, ds_sac_);
        sic_vec.set(cnt, ds_sic_);
        ds_id_vec.set(cnt, ds_id);
        line_id_vec.set(cnt, line_id_);

        tod_vec.set(cnt, tod);
        lat_vec.set(cnt, fix_it.latitude);
        long_vec.set(cnt, fix_it.longitude);

        if (set_mode_3a_code_)
            buffer_->get<unsigned int>(m3a_var.name()).set(cnt, mode_3a_code_);

        if (set_target_address_)
            buffer_->get<unsigned int>(ta_var.name()).set(cnt, target_address_);

        if (set_callsign_)
            buffer_->get<string>(ti_var.name()).set(cnt, callsign_);

        if (fix_it.travelAngle != 0.0 && fix_it.speed != 0.0)
        {
            track_angle_rad = DEG2RAD * fix_it.travelAngle;
            speed_ms = fix_it.speed * 0.27778;

            track_angle_vec.set(cnt, fix_it.travelAngle);
            speed_vec.set(cnt, speed_ms * M_S2KNOTS);

            vx = sin(track_angle_rad) * speed_ms;
            vy = cos(track_angle_rad) * speed_ms;

            loginf << "UGA track_angle_rad " << track_angle_rad << " spd " << speed_ms << " vx " << vx << " vy " << vy;

            vx_vec.set(cnt, vx);
            vy_vec.set(cnt, vy);
        }

//        if (fix_it.travelAngle != 0.0) TODO
//            head_vec.set(cnt, fix_it.travelAngle);

//        if (fix_it.speed != 0.0)
//            spd_vec.set(cnt, fix_it.speed*0.539957); // km/h to knots

        ++cnt;
    }

    //void insertData(DBOVariableSet& list, std::shared_ptr<Buffer> buffer, bool emit_change = true);

    loginf << "GPSTrailImportTask: run: inserting data";

    connect(&dbcontent_man, &DBContentManager::insertDoneSignal, this, &GPSTrailImportTask::insertDoneSlot,
            Qt::UniqueConnection);

    dbcontent_man.insertData({{dbcontent_name, buffer_}});
}

void GPSTrailImportTask::insertDoneSlot()
{
    loginf << "GPSTrailImportTask: insertDoneSlot";

    buffer_ = nullptr;

    done_ = true;

    //COMPASS::instance().interface().setProperty(PostProcessTask::DONE_PROPERTY_NAME, "0");

    COMPASS::instance().interface().setProperty(DONE_PROPERTY_NAME, "1");

//    COMPASS::instance().interface().databaseContentChanged();
//    object.updateToDatabaseContent();

    QMessageBox msg_box;

    msg_box.setWindowTitle("Import GPS Trail");
    msg_box.setText("Import of "+QString::number(gps_fixes_.size())+" GPS fixes done.");
    msg_box.setStandardButtons(QMessageBox::Ok);

    if (show_done_summary_)
        msg_box.exec();

    emit doneSignal(name_);
}

void GPSTrailImportTask::dialogImportSlot()
{
    assert (canRun());

    assert (dialog_);
    dialog_->hide();

    run();
}

void GPSTrailImportTask::dialogDoneSlot()
{
    assert (dialog_);
    dialog_->hide();
}

//void GPSTrailImportTask::checkParsedData ()
//{
//    loginf << "GPSTrailImportTask: checkParsedData";
//}
