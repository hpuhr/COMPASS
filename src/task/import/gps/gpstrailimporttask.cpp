#include "gpstrailimporttask.h"
#include "gpstrailimporttaskwidget.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "savedfile.h"
#include "stringconv.h"
#include "taskmanager.h"
#include "files.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <nmeaparse/nmea.h>

#include <QApplication>

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

    if (widget_)
    {
        widget_->updateFileListSlot();
        widget_->updateText();
    }

    emit statusChangedSignal(name_);
}

bool GPSTrailImportTask::checkPrerequisites()
{
    if (!ATSDB::instance().interface().ready())  // must be connected
        return false;

    if (ATSDB::instance().interface().hasProperty(DONE_PROPERTY_NAME))
        done_ = ATSDB::instance().interface().getProperty(DONE_PROPERTY_NAME) == "1";

    return true;
}

bool GPSTrailImportTask::isRecommended()
{
    if (!checkPrerequisites())
        return false;

    //    if (ATSDB::instance().objectManager().hasData())
    //        return false;

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

    return true;
}

bool GPSTrailImportTask::canRun() { return canImportFile(); }


void GPSTrailImportTask::run()
{
    loginf << "GPSTrailImportTask: run: filename '" << current_filename_;
}

void GPSTrailImportTask::parseCurrentFile ()
{
    loginf << "GPSTrailImportTask: parseCurrentFile: file '" << current_filename_ << "'";

    current_error_ = "";
    current_text_ = "";

    fix_cnt_ = 0;

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

        ++fix_cnt_;

        if (gps.fix.locked() && !current_text_.size())
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

    current_text_ = "\nParsed "+to_string(fix_cnt_)+" fixes.\n\n"+current_text_
            +"\n\nLast message:\n"+gps.fix.toString();

    loginf << "GPSTrailImportTask: parseCurrentFile: parsed " << fix_cnt_ << " fixes";

    QApplication::restoreOverrideCursor();

    //    current_data_.clear();

    //    if (!Files::fileExists(current_filename_))
    //    {
    //        current_error_ = "file '" + current_filename_ + "' does not exist";
    //        logerr << "GPSTrailImportTask: parseCurrentFile: " << current_error_;
    //        return;
    //    }

    //    std::ifstream ifs(current_filename_);

    //    try
    //    {
    //        current_data_ = json::parse(ifs);

    //        checkParsedData();
    //    }
    //    catch (exception& e)
    //    {
    //        current_error_ = "parsing file '" + current_filename_ + "' resulted in error '" + e.what() + "'";
    //        logerr << "GPSTrailImportTask: parseCurrentFile: " << current_error_;
    //    }

    //    if (widget_)
    //        widget_->updateContext();

    //    loginf << "GPSTrailImportTask: parseCurrentFile: done";
}

void GPSTrailImportTask::checkParsedData ()
{
    loginf << "GPSTrailImportTask: checkParsedData";

    //    if (!current_data_.is_object())
    //        throw std::runtime_error("current data is not an object");

    //    if (!current_data_.contains("view_point_context"))
    //        throw std::runtime_error("current data has no context information");

    //    json& context = current_data_.at("view_point_context");

    //    if (!context.contains("version"))
    //        throw std::runtime_error("current data context has no version");

    //    json& version = context.at("version");

    //    if (!version.is_string())
    //        throw std::runtime_error("current data context version is not string");

    //    string version_str = version;

    //    if (version_str != "0.1")
    //        throw std::runtime_error("current data context version '"+version_str+"' is not supported");

    //    if (context.contains("datasets"))
    //    {
    //        if (!context.at("datasets").is_array())
    //            throw std::runtime_error("datasets is not an array");

    //        for (json& ds_it : context.at("datasets").get<json::array_t>())
    //        {
    //            if (!ds_it.contains("name") || !ds_it.at("name").is_string())
    //                throw std::runtime_error("dataset '"+ds_it.dump()+"' does not contain a valid name");

    //            if (!ds_it.contains("filename") || !ds_it.at("filename").is_string())
    //                throw std::runtime_error("dataset '"+ds_it.dump()+"' does not contain a valid filename");

    //            std::string filename = ds_it.at("filename");

    //            bool found = true;

    //            if (!Files::fileExists(filename))
    //            {
    //                found = false;

    //                std::string file = Files::getFilenameFromPath(filename);
    //                std::string dir = Files::getDirectoryFromPath(current_filename_);

    //                loginf << "GPSTrailImportTask: checkParsedData: filename '" << filename
    //                       << "' not found, checking for file '" << file << "' in dir '" << dir << "'";

    //                filename = dir+"/"+file;

    //                if (Files::fileExists(filename))
    //                {
    //                    found = true;

    //                    loginf << "GPSTrailImportTask: checkParsedData: filename '" << filename
    //                           << "' found at different path";
    //                }
    //            }

    //            if (!found)
    //                throw std::runtime_error("dataset '"+ds_it.dump()+"' does not contain a usable filename");
    //        }
    //    }

    //    if (!current_data_.contains("view_points"))
    //        throw std::runtime_error("current data does not contain view points");

    //    json& view_points = current_data_.at("view_points");

    //    if (!view_points.is_array())
    //        throw std::runtime_error("view_points is not an array");

    //    if (!view_points.size())
    //        throw std::runtime_error("view_points is an empty array");

    //    for (auto& vp_it : view_points.get<json::array_t>())
    //    {
    //        if (!vp_it.contains("id") || !vp_it.at("id").is_number())
    //            throw std::runtime_error("view point '"+vp_it.dump()+"' does not contain a valid id");

    //        if (!vp_it.contains("type") || !vp_it.at("type").is_string())
    //            throw std::runtime_error("view point '"+vp_it.dump()+"' does not contain a valid type");
    //    }

    //    loginf << "GPSTrailImportTask: checkParsedData: current data seems to be valid, contains " << view_points.size()
    //           << " view points";
}
