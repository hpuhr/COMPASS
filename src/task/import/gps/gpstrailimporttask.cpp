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
#include "gpstrailimporttaskwidget.h"
#include "compass.h"
#include "dbinterface.h"
#include "savedfile.h"
#include "stringconv.h"
#include "taskmanager.h"
#include "files.h"
#include "dbobjectmanager.h"
#include "dbobject.h"
#include "dbovariable.h"
#include "buffer.h"
#include "dbovariableset.h"
#include "util/number.h"
//#include "postprocesstask.h"
//#include "managedatasourcestask.h"

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
    registerParameter("callsign_", &callsign_, "");

    createSubConfigurables();

    if (current_filename_.size())
        parseCurrentFile();
}

GPSTrailImportTask::~GPSTrailImportTask()
{
    for (auto it : file_list_)
        delete it.second;

    file_list_.clear();
}

void GPSTrailImportTask::generateSubConfigurable(const std::string& class_id,
                                                 const std::string& instance_id)
{
    if (class_id == "NMEAFile")
    {
        SavedFile* file = new SavedFile(class_id, instance_id, this);
        assert(file_list_.count(file->name()) == 0);
        file_list_.insert(std::pair<std::string, SavedFile*>(file->name(), file));
    }
    else
        throw std::runtime_error("GPSTrailImportTask: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

TaskWidget* GPSTrailImportTask::widget()
{
    if (!widget_)
    {
        widget_.reset(new GPSTrailImportTaskWidget(*this));

        connect(&task_manager_, &TaskManager::expertModeChangedSignal, widget_.get(),
                &GPSTrailImportTaskWidget::expertModeChangedSlot);
    }

    assert(widget_);
    return widget_.get();
}

void GPSTrailImportTask::deleteWidget() { widget_.reset(nullptr); }


void GPSTrailImportTask::addFile(const std::string& filename)
{
    loginf << "GPSTrailImportTask: addFile: filename '" << filename << "'";

    if (file_list_.count(filename) != 0)
        throw std::invalid_argument("GPSTrailImportTask: addFile: name '" + filename +
                                    "' already in use");

    std::string instancename = filename;
    instancename.erase(std::remove(instancename.begin(), instancename.end(), '/'),
                       instancename.end());

    Configuration& config = addNewSubConfiguration("NMEAFile", "NMEAFile" + instancename);
    config.addParameterString("name", filename);
    generateSubConfigurable("NMEAFile", "NMEAFile" + instancename);

    current_filename_ = filename;  // set as current
    parseCurrentFile();

    emit statusChangedSignal(name_);

    if (widget_)
    {
        widget_->updateFileListSlot();
        widget_->updateText();
    }
}

void GPSTrailImportTask::removeCurrentFilename()
{
    loginf << "GPSTrailImportTask: removeCurrentFilename: filename '" << current_filename_ << "'";

    assert(current_filename_.size());
    assert(hasFile(current_filename_));

    if (file_list_.count(current_filename_) != 1)
        throw std::invalid_argument("GPSTrailImportTask: addFile: name '" + current_filename_ +
                                    "' not in use");

    delete file_list_.at(current_filename_);
    file_list_.erase(current_filename_);
    current_filename_ = "";

    emit statusChangedSignal(name_);

    if (widget_)
    {
        widget_->updateFileListSlot();
        widget_->updateText();
    }
}

void GPSTrailImportTask::removeAllFiles ()
{
    loginf << "GPSTrailImportTask: removeAllFiles";

    while (file_list_.size())
    {
        delete file_list_.begin()->second;
        file_list_.erase(file_list_.begin());
    }

    current_filename_ = "";

    emit statusChangedSignal(name_);

    if (widget_)
    {
        widget_->updateFileListSlot();
        widget_->updateText();
    }
}

void GPSTrailImportTask::currentFilename(const std::string& filename)
{
    loginf << "GPSTrailImportTask: currentFilename: filename '" << filename << "'";

    current_filename_ = filename;

    parseCurrentFile();

    //    if (widget_)
    //    {
    //        widget_->updateFileListSlot();
    //        widget_->updateText();
    //    }

    emit statusChangedSignal(name_);
}

bool GPSTrailImportTask::checkPrerequisites()
{
    if (!COMPASS::instance().interface().ready())  // must be connected
        return false;

    if (COMPASS::instance().interface().hasProperty(DONE_PROPERTY_NAME))
        done_ = COMPASS::instance().interface().getProperty(DONE_PROPERTY_NAME) == "1";

    if (!COMPASS::instance().objectManager().existsObject("RefTraj"))
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

    DBObjectManager& obj_man = COMPASS::instance().objectManager();

    assert (obj_man.existsObject("RefTraj"));

    DBObject& reftraj_obj = obj_man.object("RefTraj");

    assert (reftraj_obj.hasVariable("sac"));
    assert (reftraj_obj.hasVariable("sic"));
    assert (reftraj_obj.hasVariable("ds_id"));
    assert (reftraj_obj.hasVariable("tod"));
    assert (reftraj_obj.hasVariable("pos_lat_deg"));
    assert (reftraj_obj.hasVariable("pos_long_deg"));

    assert (reftraj_obj.hasVariable("mode3a_code"));
    assert (reftraj_obj.hasVariable("target_addr"));
    assert (reftraj_obj.hasVariable("callsign"));

    assert (reftraj_obj.hasVariable("heading_deg"));
    assert (reftraj_obj.hasVariable("groundspeed_kt"));

    loginf << "GPSTrailImportTask: run: getting variables";

    DBOVariable& sac_var = reftraj_obj.variable("sac");
    DBOVariable& sic_var = reftraj_obj.variable("sic");
    DBOVariable& ds_id_var = reftraj_obj.variable("ds_id");
    DBOVariable& tod_var = reftraj_obj.variable("tod");
    DBOVariable& lat_var = reftraj_obj.variable("pos_lat_deg");
    DBOVariable& long_var = reftraj_obj.variable("pos_long_deg");

    DBOVariable& m3a_var = reftraj_obj.variable("mode3a_code");
    DBOVariable& ta_var = reftraj_obj.variable("target_addr");
    DBOVariable& cs_var = reftraj_obj.variable("callsign");

    DBOVariable& head_var = reftraj_obj.variable("heading_deg");
    DBOVariable& spd_var = reftraj_obj.variable("groundspeed_kt");


    DBOVariableSet var_set;

    var_set.add(sac_var);
    var_set.add(sic_var);
    var_set.add(ds_id_var);
    var_set.add(tod_var);
    var_set.add(lat_var);
    var_set.add(long_var);

    if (set_mode_3a_code_)
        var_set.add(m3a_var);

    if (set_target_address_)
        var_set.add(ta_var);

    if (set_callsign_)
        var_set.add(cs_var);

    var_set.add(head_var);
    var_set.add(spd_var);

    PropertyList properties;
    properties.addProperty(sac_var.name(), PropertyDataType::UCHAR);
    properties.addProperty(sic_var.name(), PropertyDataType::UCHAR);
    properties.addProperty(ds_id_var.name(), PropertyDataType::INT);
    properties.addProperty(tod_var.name(), PropertyDataType::FLOAT);
    properties.addProperty(lat_var.name(), PropertyDataType::DOUBLE);
    properties.addProperty(long_var.name(), PropertyDataType::DOUBLE);

    if (set_mode_3a_code_)
        properties.addProperty(m3a_var.name(), PropertyDataType::INT);

    if (set_target_address_)
        properties.addProperty(ta_var.name(), PropertyDataType::INT);

    if (set_callsign_)
        properties.addProperty(cs_var.name(), PropertyDataType::STRING);

    properties.addProperty(head_var.name(), PropertyDataType::DOUBLE);
    properties.addProperty(spd_var.name(), PropertyDataType::DOUBLE);

    loginf << "GPSTrailImportTask: run: creating buffer";

    buffer_ = make_shared<Buffer>(properties, "RefTraj");

    NullableVector<unsigned char>& sac_vec = buffer_->get<unsigned char>("sac");
    NullableVector<unsigned char>& sic_vec = buffer_->get<unsigned char>("sic");
    NullableVector<int>& ds_id_vec = buffer_->get<int>("ds_id");
    NullableVector<float>& tod_vec = buffer_->get<float>("tod");
    NullableVector<double>& lat_vec = buffer_->get<double>("pos_lat_deg");
    NullableVector<double>& long_vec = buffer_->get<double>("pos_long_deg");

    NullableVector<double>& head_vec = buffer_->get<double>("heading_deg");
    NullableVector<double>& spd_vec = buffer_->get<double>("groundspeed_kt");

    unsigned int cnt = 0;
    int ds_id = Number::dsIdFrom(ds_sac_, ds_sic_);

    // config data source
    {
        TODO_ASSERT

//        ManageDataSourcesTask& ds_task = COMPASS::instance().taskManager().manageDataSourcesTask();

//        if (!ds_task.hasDataSource("RefTraj", ds_sac_, ds_sic_)) // add if not existing
//        {
//            loginf << "GPSTrailImportTask: run: adding data source '" << ds_name_ << "' "
//                   << ds_sac_ << "/" << ds_sic_;
//            StoredDBODataSource& new_ds = ds_task.addNewStoredDataSource("RefTraj");
//            new_ds.name(ds_name_);
//            new_ds.sac(ds_sac_);
//            new_ds.sic(ds_sic_);
//        }
//        else // set name if existing
//        {
//            loginf << "GPSTrailImportTask: run: setting data source '" << ds_name_ << "' "
//                   << ds_sac_ << "/" << ds_sic_;
//            StoredDBODataSource& ds = ds_task.getDataSource("RefTraj", ds_sac_, ds_sic_);
//            ds.name(ds_name_);
//        }
    }

    bool has_ds = false; //reftraj_obj.hasDataSources() && reftraj_obj.hasDataSource(ds_id);

    float tod;

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

        tod_vec.set(cnt, tod);
        lat_vec.set(cnt, fix_it.latitude);
        long_vec.set(cnt, fix_it.longitude);

        if (set_mode_3a_code_)
            buffer_->get<int>("mode3a_code").set(cnt, mode_3a_code_);

        if (set_target_address_)
            buffer_->get<int>("target_addr").set(cnt, target_address_);

        if (set_callsign_)
            buffer_->get<string>("callsign").set(cnt, callsign_);

        if (fix_it.travelAngle != 0.0)
            head_vec.set(cnt, fix_it.travelAngle);

        if (fix_it.speed != 0.0)
            spd_vec.set(cnt, fix_it.speed*0.539957); // km/h to knots

        ++cnt;
    }

    if (!has_ds)
    {
        loginf << "GPSTrailImportTask: run: adding data source";

        std::map<int, std::pair<int, int>> datasources_to_add;

        datasources_to_add[ds_id] = {ds_sac_, ds_sic_};

        TODO_ASSERT

        //reftraj_obj.addDataSources(datasources_to_add);
    }

    //void insertData(DBOVariableSet& list, std::shared_ptr<Buffer> buffer, bool emit_change = true);

    loginf << "GPSTrailImportTask: run: inserting data";

//    connect(&reftraj_obj, &DBObject::insertDoneSignal, this, &GPSTrailImportTask::insertDoneSlot,
//            Qt::UniqueConnection);
//    connect(&reftraj_obj, &DBObject::insertProgressSignal, this,
//            &GPSTrailImportTask::insertProgressSlot, Qt::UniqueConnection);

    TODO_ASSERT

    //reftraj_obj.insertData(var_set, buffer_, false);
}

void GPSTrailImportTask::insertProgressSlot(float percent)
{
    loginf << "GPSTrailImportTask: insertProgressSlot: percent " << percent;
}

void GPSTrailImportTask::insertDoneSlot(DBObject& object)
{
    loginf << "GPSTrailImportTask: insertDoneSlot";

    buffer_ = nullptr;

    done_ = true;

    task_manager_.appendSuccess("GPSTrailImportTask: imported " + to_string(gps_fixes_.size())
                                +" GPS fixes");

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

//void GPSTrailImportTask::checkParsedData ()
//{
//    loginf << "GPSTrailImportTask: checkParsedData";
//}
