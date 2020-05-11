#include "gpstrailimporttask.h"
#include "gpstrailimporttaskwidget.h"
#include "atsdb.h"
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
#include "dbtablecolumn.h"
#include "postprocesstask.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <nmeaparse/nmea.h>

#include <QApplication>
#include <QMessageBox>

const std::string DONE_PROPERTY_NAME = "gps_trail_imported";

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
    if (!ATSDB::instance().interface().ready())  // must be connected
        return false;

    if (ATSDB::instance().interface().hasProperty(DONE_PROPERTY_NAME))
        done_ = ATSDB::instance().interface().getProperty(DONE_PROPERTY_NAME) == "1";

    if (!ATSDB::instance().objectManager().existsObject("RefTraj"))
        return false;

    if (!ATSDB::instance().objectManager().object("RefTraj").hasCurrentMetaTable())
        return false;

    return true;
}

bool GPSTrailImportTask::isRecommended()
{
    if (!checkPrerequisites())
        return false;

    //    if (ATSDB::instance().objectManager().hasData())
    //        return false;

    if (done_)
        return false;

    return canImportFile();
}

bool GPSTrailImportTask::isRequired() { return false; }

std::string GPSTrailImportTask::currentError() const
{
    return current_error_;
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

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    NMEAParser parser;
    GPSService gps(parser);
    parser.log = false;

    //cout << "Fix  Sats  Sig\t\tSpeed    Dir  Lat         , Lon           Accuracy" << endl;
    // Handle any changes to the GPS Fix... This is called whenever it's updated.
    gps.onUpdate += [&gps, this](){
//        cout << (gps.fix.locked() ? "[*] " : "[ ] ") << setw(2) << setfill(' ') << gps.fix.trackingSatellites << "/" << setw(2) << setfill(' ') << gps.fix.visibleSatellites << " ";
//        cout << fixed << setprecision(2) << setw(5) << setfill(' ') << gps.fix.almanac.averageSNR() << " dB   ";
//        cout << fixed << setprecision(2) << setw(6) << setfill(' ') << gps.fix.speed << " km/h [" << GPSFix::travelAngleToCompassDirection(gps.fix.travelAngle, true) << "]  ";
//        cout << fixed << setprecision(6) << gps.fix.latitude << "\xF8 " "N, " << gps.fix.longitude << "\xF8 " "E" << "  ";
//        cout << "+/- " << setprecision(1) << gps.fix.horizontalAccuracy() << "m  ";
//        cout << endl;

        if (!gps.fix.locked())
            return; // TODO skip

        gps_fixes_.push_back(gps.fix);

        if (!current_text_.size())
            current_text_ = "First (locked) message:\n"+gps.fix.toString();
    };

    // From a file
    string line;
    ifstream file(current_filename_);
    unsigned int line_cnt = 0;

    while (getline(file, line))
    {
        try
        {
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

    current_text_ = "\nParsed "+to_string(gps_fixes_.size())+" fixes.\n\n"+current_text_
            +"\n\nLast message:\n"+gps.fix.toString();

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

    DBObjectManager& obj_man = ATSDB::instance().objectManager();

    assert (obj_man.existsObject("RefTraj"));

    DBObject& reftraj_obj = obj_man.object("RefTraj");
    assert (reftraj_obj.hasCurrentMetaTable());

    assert (reftraj_obj.hasVariable("sac"));
    assert (reftraj_obj.hasVariable("sic"));
    assert (reftraj_obj.hasVariable("ds_id"));
    assert (reftraj_obj.hasVariable("tod"));
    assert (reftraj_obj.hasVariable("pos_lat_deg"));
    assert (reftraj_obj.hasVariable("pos_long_deg"));

    loginf << "GPSTrailImportTask: run: getting variables";

    DBOVariable& sac_var = reftraj_obj.variable("sac");
    DBOVariable& sic_var = reftraj_obj.variable("sic");
    DBOVariable& ds_id_var = reftraj_obj.variable("ds_id");
    DBOVariable& tod_var = reftraj_obj.variable("tod");
    DBOVariable& lat_var = reftraj_obj.variable("pos_lat_deg");
    DBOVariable& long_var = reftraj_obj.variable("pos_long_deg");

    DBOVariableSet var_set;

    var_set.add(sac_var);
    var_set.add(sic_var);
    var_set.add(ds_id_var);
    var_set.add(tod_var);
    var_set.add(lat_var);
    var_set.add(long_var);

    PropertyList properties;
    properties.addProperty(sac_var.name(), PropertyDataType::UCHAR);
    properties.addProperty(sic_var.name(), PropertyDataType::UCHAR);
    properties.addProperty(ds_id_var.name(), PropertyDataType::INT);
    properties.addProperty(tod_var.name(), PropertyDataType::FLOAT);
    properties.addProperty(lat_var.name(), PropertyDataType::DOUBLE);
    properties.addProperty(long_var.name(), PropertyDataType::DOUBLE);

    loginf << "GPSTrailImportTask: run: creating buffer";

    buffer_ = make_shared<Buffer>(properties, "RefTraj");

    NullableVector<unsigned char>& sac_vec = buffer_->get<unsigned char>("sac");
    NullableVector<unsigned char>& sic_vec = buffer_->get<unsigned char>("sic");
    NullableVector<int>& ds_id_vec = buffer_->get<int>("ds_id");
    NullableVector<float>& tod_vec = buffer_->get<float>("tod");
    NullableVector<double>& lat_vec = buffer_->get<double>("pos_lat_deg");
    NullableVector<double>& long_vec = buffer_->get<double>("pos_long_deg");

    unsigned int cnt = 0;
    int sac = 0;
    int sic = 0;
    int ds_id = sac*255+sic;

    bool has_ds = reftraj_obj.hasDataSources() && reftraj_obj.hasDataSource(ds_id);

    float tod;

    loginf << "GPSTrailImportTask: run: filling buffer";

    for (auto& fix_it : gps_fixes_)
    {

        tod = fix_it.timestamp.hour*3600.0 + fix_it.timestamp.min*60.0+fix_it.timestamp.sec;

        sac_vec.set(cnt, sac);
        sic_vec.set(cnt, sic);
        ds_id_vec.set(cnt, ds_id);

        tod_vec.set(cnt, tod);
        lat_vec.set(cnt, fix_it.latitude);
        long_vec.set(cnt, fix_it.longitude);

        ++cnt;
    }

    if (!has_ds)
    {
        loginf << "GPSTrailImportTask: run: adding data source";

        std::map<int, std::pair<int, int>> datasources_to_add;

        datasources_to_add[ds_id] = {sac,sic};

        reftraj_obj.addDataSources(datasources_to_add);
    }

    //void insertData(DBOVariableSet& list, std::shared_ptr<Buffer> buffer, bool emit_change = true);

    loginf << "GPSTrailImportTask: run: inserting data";

    connect(&reftraj_obj, &DBObject::insertDoneSignal, this, &GPSTrailImportTask::insertDoneSlot,
            Qt::UniqueConnection);
    connect(&reftraj_obj, &DBObject::insertProgressSignal, this,
            &GPSTrailImportTask::insertProgressSlot, Qt::UniqueConnection);

    reftraj_obj.insertData(var_set, buffer_, false);
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

    ATSDB::instance().interface().setProperty(PostProcessTask::DONE_PROPERTY_NAME, "0");

    ATSDB::instance().interface().setProperty(DONE_PROPERTY_NAME, "1");

    ATSDB::instance().interface().databaseContentChanged();
    object.updateToDatabaseContent();

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
