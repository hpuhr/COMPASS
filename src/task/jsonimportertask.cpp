#include "jsonimportertask.h"
#include "jsonimportertaskwidget.h"
#include "taskmanager.h"
#include "atsdb.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbovariable.h"
#include "sqlitefile.h"
#include "files.h"
#include "stringconv.h"
#include "metadbtable.h"
#include "dbtable.h"
#include "dbtablecolumn.h"
#include "propertylist.h"
#include "buffer.h"

#include <stdexcept>
#include <fstream>
#include <memory>
#include <algorithm>

#include <QDateTime>
#include <QCoreApplication>
#include <QThread>
#include <QMessageBox>

#include "boost/date_time/posix_time/posix_time.hpp"

//#include <jsoncpp/json/json.h>
//#include "json.hpp"

#include <archive.h>
#include <archive_entry.h>

using namespace Utils;
using namespace nlohmann;

JSONImporterTask::JSONImporterTask(const std::string& class_id, const std::string& instance_id,
                                   TaskManager* task_manager)
    : Configurable (class_id, instance_id, task_manager)
{
    registerParameter("last_filename", &last_filename_, "");
    //registerParameter("db_object_str", &db_object_str_, "ADSB");

    registerParameter("join_data_sources", &join_data_sources_, false);
    registerParameter("separate_mlat_data", &separate_mlat_data_, false);

    registerParameter("use_time_filter", &use_time_filter_, false);
    registerParameter("time_filter_min", &time_filter_min_, 0);
    registerParameter("time_filter_max", &time_filter_max_, 24*3600);

    registerParameter("use_position_filter", &use_position_filter_, false);
    registerParameter("pos_filter_lat_min", &pos_filter_lat_min_, -90);
    registerParameter("pos_filter_lat_max", &pos_filter_lat_max_, 90);
    registerParameter("pos_filter_lon_min", &pos_filter_lon_min_, -180);
    registerParameter("pos_filter_lon_max", &pos_filter_lon_max_, 180);

    createSubConfigurables();
}

JSONImporterTask::~JSONImporterTask()
{
    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    for (auto it : file_list_)
        delete it.second;

    file_list_.clear();
}

void JSONImporterTask::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    if (class_id == "JSONFile")
    {
        SavedFile *file = new SavedFile (class_id, instance_id, this);
        assert (file_list_.count (file->name()) == 0);
        file_list_.insert (std::pair <std::string, SavedFile*> (file->name(), file));
    }
    else
        throw std::runtime_error ("JSONImporterTask: generateSubConfigurable: unknown class_id "+class_id );
}


JSONImporterTaskWidget* JSONImporterTask::widget()
{
    if (!widget_)
    {
        widget_ = new JSONImporterTaskWidget (*this);
    }

    assert (widget_);
    return widget_;
}

void JSONImporterTask::addFile (const std::string &filename)
{
    if (file_list_.count (filename) != 0)
        throw std::invalid_argument ("JSONImporterTask: addFile: name '"+filename+"' already in use");

    std::string instancename = filename;
    instancename.erase (std::remove(instancename.begin(), instancename.end(), '/'), instancename.end());

    Configuration &config = addNewSubConfiguration ("JSONFile", "JSONFile"+instancename);
    config.addParameterString("name", filename);
    generateSubConfigurable ("JSONFile", "JSONFile"+instancename);

    if (widget_)
        widget_->updateFileListSlot();
}

void JSONImporterTask::removeFile (const std::string &filename)
{
    if (file_list_.count (filename) != 1)
        throw std::invalid_argument ("JSONImporterTask: addFile: name '"+filename+"' not in use");

    delete file_list_.at(filename);
    file_list_.erase(filename);

    if (widget_)
        widget_->updateFileListSlot();
}

bool JSONImporterTask::useTimeFilter() const
{
    return use_time_filter_;
}

void JSONImporterTask::useTimeFilter(bool value)
{
    use_time_filter_ = value;
}

float JSONImporterTask::timeFilterMin() const
{
    return time_filter_min_;
}

void JSONImporterTask::timeFilterMin(float value)
{
    time_filter_min_ = value;
}

float JSONImporterTask::timeFilterMax() const
{
    return time_filter_max_;
}

void JSONImporterTask::timeFilterMax(float value)
{
    time_filter_max_ = value;
}

bool JSONImporterTask::joinDataSources() const
{
    return join_data_sources_;
}

void JSONImporterTask::joinDataSources(bool value)
{
    join_data_sources_ = value;
}

bool JSONImporterTask::separateMLATData() const
{
    return separate_mlat_data_;
}

void JSONImporterTask::separateMLATData(bool value)
{
    separate_mlat_data_ = value;
}

bool JSONImporterTask::usePositionFilter() const
{
    return use_position_filter_;
}

void JSONImporterTask::usePositionFilter(bool use_position_filter)
{
    use_position_filter_ = use_position_filter;
}

float JSONImporterTask::positionFilterLatitudeMin() const
{
    return pos_filter_lat_min_;
}

void JSONImporterTask::positionFilterLatitudeMin(float value)
{
    pos_filter_lat_min_ = value;
}

float JSONImporterTask::positionFilterLatitudeMax() const
{
    return pos_filter_lat_max_;
}

void JSONImporterTask::positionFilterLatitudeMax(float value)
{
    pos_filter_lat_max_ = value;
}

float JSONImporterTask::positionFilterLongitudeMin() const
{
    return pos_filter_lon_min_;
}

void JSONImporterTask::positionFilterLongitudeMin(float value)
{
    pos_filter_lon_min_ = value;
}

float JSONImporterTask::positionFilterLongitudeMax() const
{
    return pos_filter_lon_max_;
}

void JSONImporterTask::positionFilterLongitudeMax(float value)
{
    pos_filter_lon_max_ = value;
}

//std::string JSONImporterTask::dbObjectStr() const
//{
//    return db_object_str_;
//}

//void JSONImporterTask::dbObjectStr(const std::string& db_object_str)
//{
//    loginf << "JSONImporterTask: dbObjectStr: " << db_object_str;

//    db_object_str_ = db_object_str;

//    assert (ATSDB::instance().objectManager().existsObject(db_object_str_));
//    db_object_ = &ATSDB::instance().objectManager().object(db_object_str_);
//}

bool JSONImporterTask::canImportFile (const std::string& filename)
{
    if (!Files::fileExists(filename))
    {
        loginf << "JSONImporterTask: canImportFile: not possible since file does not exist";
        return false;
    }

    if (!ATSDB::instance().objectManager().existsObject("ADSB"))
    {
        loginf << "JSONImporterTask: canImportFile: not possible since DBObject does not exist";
        return false;
    }

//    if (!key_var_str_.size()
//            || !dsid_var_str_.size()
//            || !target_addr_var_str_.size()
//            || !callsign_var_str_.size()
//            || !altitude_baro_var_str_.size()
//            || !altitude_geo_var_str_.size()
//            || !latitude_var_str_.size()
//            || !longitude_var_str_.size()
//            || !tod_var_str_.size())
//    {
//        loginf << "JSONImporterTask: canImportFile: not possible since variables are not set";
//        return false;
//    }

//    if (!object.hasVariable(key_var_str_)
//            || !object.hasVariable(dsid_var_str_)
//            || !object.hasVariable(target_addr_var_str_)
//            || !object.hasVariable(callsign_var_str_)
//            || !object.hasVariable(altitude_baro_var_str_)
//            || !object.hasVariable(altitude_geo_var_str_)
//            || !object.hasVariable(latitude_var_str_)
//            || !object.hasVariable(longitude_var_str_)
//            || !object.hasVariable(tod_var_str_))
//    {
//        loginf << "JSONImporterTask: canImportFile: not possible since variables not in DBObject";
//        return false;
//    }

//    if (use_time_filter_ && time_filter_min_ >= time_filter_max_)
//    {
//        loginf << "JSONImporterTask: canImportFile: time filter values wrong";
//        return false;
//    }

    return true;
}

void JSONImporterTask::importFile(const std::string& filename, bool test)
{
    loginf << "JSONImporterTask: importFile: filename " << filename << " test " << test;

//    if (db_object_str_.size())
//    {
//        if (!ATSDB::instance().objectManager().existsObject(db_object_str_))
//            db_object_str_="";
//        else
//            db_object_ = &ATSDB::instance().objectManager().object(db_object_str_);
//    }
//    assert (db_object_);

    if (!canImportFile(filename))
    {
        logerr << "JSONImporterTask: importFile: unable to import";
        return;
    }

    std::ifstream ifs(filename);
//    Json::Reader reader;
//    Json::Value obj;
//    reader.parse(ifs, obj); // reader can also read strings
    json j = json::parse(ifs);

    parseJSON (j, test);

    if (!test)
        insertData ();
    else
        clearData();

    QMessageBox msgBox;
    std::string msg = "Reading archive " + filename + " finished successfully.\n";
    if (all_cnt_)
        msg +=  + "# of updates: " + std::to_string(all_cnt_)
                + "\n# of skipped updates: " + std::to_string(skipped_cnt_)
                + " (" +String::percentToString(100.0 * skipped_cnt_/all_cnt_) + "%)"
                + "\n# of inserted updates: " + std::to_string(inserted_cnt_)
                + " (" +String::percentToString(100.0 * inserted_cnt_/all_cnt_) + "%)";
    msgBox.setText(msg.c_str());
    msgBox.exec();


    return;
}

void JSONImporterTask::importFileArchive (const std::string& filename, bool test)
{
    loginf << "JSONImporterTask: importFileArchive: filename " << filename << " test " << test;

//    if (db_object_str_.size())
//    {
//        if (!ATSDB::instance().objectManager().existsObject(db_object_str_))
//            db_object_str_="";
//        else
//            db_object_ = &ATSDB::instance().objectManager().object(db_object_str_);
//    }
//    assert (db_object_);

    if (!canImportFile(filename))
    {
        logerr << "JSONImporterTask: importFileArchive: unable to import";
        return;
    }

    // if gz but not tar.gz or tgz
    bool raw = String::hasEnding (filename, ".gz") && !String::hasEnding (filename, ".tar.gz");

    loginf  << "JSONImporterTask: importFileArchive: importing " << filename << " raw " << raw;

    if (use_time_filter_)
        loginf  << "JSONImporterTask: importFileArchive: using time filter min " << time_filter_min_
                << " max " << time_filter_max_;

    if (use_position_filter_)
        loginf  << "JSONImporterTask: importFileArchive: using position filter latitude min " << pos_filter_lat_min_
                << " max " << pos_filter_lat_max_ << " longitude min " << pos_filter_lon_min_
                << " max " << pos_filter_lon_max_;

    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

    struct archive *a;
    struct archive_entry *entry;
    int r;

    a = archive_read_new();

    if (raw)
    {
        archive_read_support_filter_gzip(a);
        archive_read_support_filter_bzip2(a);
        archive_read_support_format_raw(a);
    }
    else
    {
        archive_read_support_filter_all(a);
        archive_read_support_format_all(a);

    }
    r = archive_read_open_filename(a, filename.c_str(), 10240); // Note 1

    if (r != ARCHIVE_OK)
        throw std::runtime_error("JSONImporterTask: importFileArchive: archive error: "
                                 +std::string(archive_error_string(a)));

    const void *buff;
    size_t size;
    int64_t offset;

    std::stringstream ss;
//    json j;
//    Json::Reader reader;
//    Json::Value obj;

    QMessageBox msg_box;
    std::string msg = "Importing archive '"+filename+"'.";
    msg_box.setText(msg.c_str());
    msg_box.setStandardButtons(QMessageBox::NoButton);
    msg_box.show();


    unsigned int entry_cnt = 0;

    size_t archive_read_time = 0;
    size_t archive_parse_time = 0;
    size_t json_parse_time = 0;
    size_t wait_time = 0;
    size_t insert_time = 0;

    boost::posix_time::ptime tmp_time;

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
    {
//        for (unsigned int cnt=0; cnt < 10; cnt++)
//            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        loginf << "Archive file found: " << archive_entry_pathname(entry) << " size " << archive_entry_size(entry);

        tmp_time = boost::posix_time::microsec_clock::local_time();

        msg = "Reading archive entry " + std::to_string(entry_cnt) + ": "
                + std::string(archive_entry_pathname(entry)) + ".\n";
        if (all_cnt_)
            msg +=  + "# of updates: " + std::to_string(all_cnt_)
                    + "\n# of skipped updates: " + std::to_string(skipped_cnt_)
                    + " (" +String::percentToString(100.0 * skipped_cnt_/all_cnt_) + "%)"
                    + "\n# of inserted updates: " + std::to_string(inserted_cnt_)
                    + " (" +String::percentToString(100.0 * inserted_cnt_/all_cnt_) + "%)";

        msg_box.setInformativeText(msg.c_str());
        msg_box.show();
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        ss.str("");

        for (;;)
        {
            r = archive_read_data_block(a, &buff, &size, &offset);

            if (r == ARCHIVE_EOF)
                break;
            if (r != ARCHIVE_OK)
                throw std::runtime_error("JSONImporterTask: importFileArchive: archive error: "
                                         +std::string(archive_error_string(a)));

            std::string str (reinterpret_cast<char const*>(buff), size);
            ss << str;
        }

        archive_read_time += (boost::posix_time::microsec_clock::local_time() - tmp_time).total_milliseconds();
        tmp_time = boost::posix_time::microsec_clock::local_time();

        loginf  << "JSONImporterTask: importFileArchive: got entry with " << ss.str().size() << " chars";
        //reader.parse(ss.str(), obj); // reader can also read strings

        try
        {
            json j = json::parse(ss.str());

            archive_parse_time += (boost::posix_time::microsec_clock::local_time() - tmp_time).total_milliseconds();
            tmp_time = boost::posix_time::microsec_clock::local_time();

            parseJSON (j, test);

            json_parse_time += (boost::posix_time::microsec_clock::local_time() - tmp_time).total_milliseconds();
            tmp_time = boost::posix_time::microsec_clock::local_time();

            while (insert_active_)
            {
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                QThread::msleep (10);
            }
            wait_time += (boost::posix_time::microsec_clock::local_time() - tmp_time).total_milliseconds();
            tmp_time = boost::posix_time::microsec_clock::local_time();

            if (!test)
                insertData ();
            else
                clearData();

            insert_time += (boost::posix_time::microsec_clock::local_time() - tmp_time).total_milliseconds();

            loginf << "JSONImporterTask: importFileArchive: time: archive_read_time " << archive_read_time/1000.0
                   << " archive_parse_time " << archive_parse_time/1000.0
                   << " json_parse_time " << json_parse_time/1000.0
                   << " wait_time " << wait_time/1000.0
                   << " insert_time " << insert_time/1000.0;

            entry_cnt++;
        }
        catch (std::exception e)
        {
            logerr << "JSONImporterTask: importFileArchive: json parsing error: " << e.what()
                   << ", skipping entry ";
            continue;
        }
    }

    r = archive_read_close(a);
    if (r != ARCHIVE_OK)
        throw std::runtime_error("JSONImporterTask: importFileArchive: archive read close error: "
                                 +std::string(archive_error_string(a)));

    r = archive_read_free(a);

    if (r != ARCHIVE_OK)
        throw std::runtime_error("JSONImporterTask: importFileArchive: archive read free error: "
                                 +std::string(archive_error_string(a)));

    msg_box.close();

    for (unsigned int cnt=0; cnt < 10; cnt++)
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    boost::posix_time::ptime stop_time = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration diff = stop_time - start_time;

    std::string time_str = std::to_string(diff.hours())+"h "+std::to_string(diff.minutes())
            +"m "+std::to_string(diff.seconds())+"s";

    QMessageBox msgBox;
    msg = "Reading archive " + filename + " with " + std::to_string(entry_cnt) + " entries finished successfully in "
            + time_str+".\n";
    if (all_cnt_)
        msg +=  + "# of updates: " + std::to_string(all_cnt_)
                + "\n# of skipped updates: " + std::to_string(skipped_cnt_)
                + " (" +String::percentToString(100.0 * skipped_cnt_/all_cnt_) + "%)"
                + "\n# of inserted updates: " + std::to_string(inserted_cnt_)
                + " (" +String::percentToString(100.0 * inserted_cnt_/all_cnt_) + "%)";
    msgBox.setText(msg.c_str());
    msgBox.exec();

    return;
}

void JSONImporterTask::parseJSON (nlohmann::json& j, bool test)
{
    loginf << "JSONImporterTask: parseJSON";

    if (mappings_.size() == 0)
    {
        assert (ATSDB::instance().objectManager().existsObject("ADSB"));
        DBObject& db_object = ATSDB::instance().objectManager().object("ADSB");

        mappings_.push_back(JsonMapping (db_object));
        mappings_.at(0).JSONKey("*");
        mappings_.at(0).JSONValue("*");
        mappings_.at(0).JSONContainerKey("acList");
        mappings_.at(0).overrideKeyVariable(true);

        //    key_var_str_ = "rec_num";
        //    dsid_var_str_ = "ds_id";
        mappings_.at(0).addMapping({"Rcvr", db_object.variable("ds_id"), true});
        //    target_addr_var_str_ = "target_addr";
        mappings_.at(0).addMapping({"Icao", db_object.variable("target_addr"), true,
                                    Format(PropertyDataType::STRING, "hexadecimal")});
        //    callsign_var_str_ = "callsign";
        mappings_.at(0).addMapping({"Reg", db_object.variable("callsign"), false});
        //    altitude_baro_var_str_ = "alt_baro_ft";
        mappings_.at(0).addMapping({"Alt", db_object.variable("alt_baro_ft"), false});
        //    altitude_geo_var_str_ = "alt_geo_ft";
        mappings_.at(0).addMapping({"Galt", db_object.variable("alt_geo_ft"), false});
        //    latitude_var_str_ = "pos_lat_deg";
        mappings_.at(0).addMapping({"Lat", db_object.variable("pos_lat_deg"), true});
        //    longitude_var_str_ = "pos_long_deg";
        mappings_.at(0).addMapping({"Long", db_object.variable("pos_long_deg"), true});
        //    tod_var_str_ = "tod";
        mappings_.at(0).addMapping({"PosTime", db_object.variable("tod"), true,
                                   Format(PropertyDataType::STRING, "epoch_tod")});
    }


//    for (auto& src_it : db_object_->dataSources())
//        if (datasources_existing_.count(src_it.first) == 0)
//            datasources_existing_[src_it.first] = src_it.second.name();

    unsigned int row_cnt = 0;

    for (auto& map_it : mappings_)
    {
        row_cnt += map_it.parseJSON(j, test);
        loginf << "JSONImporterTask: parseJSON: parsed " << map_it.dbObject().name() << " with " << row_cnt << " rows";
    }

//    assert (buffer_ptr->size() == row_cnt);

//    if (var_list_.getSize() == 0) // fill if empty, only at first time
//    {
//        var_list_.add(*key_var_);
//        var_list_.add(*dsid_var_);
//        var_list_.add(*target_addr_var_);
//        var_list_.add(*callsign_var_);
//        var_list_.add(*altitude_baro_var_);
//        var_list_.add(*altitude_geo_var_);
//        var_list_.add(*latitude_var_);
//        var_list_.add(*longitude_var_);
//        var_list_.add(*tod_var_);
//    }

//        loginf << "JSONImporterTask: parseJSON: all " << all_cnt << " rec_num_cnt " << rec_num_cnt_
//               << " to be inserted " << row_cnt << " (" << String::percentToString(100.0 * row_cnt/all_cnt) << "%)"
//               << " skipped " << skipped<< " (" << String::percentToString(100.0 * skipped/all_cnt) << "%)";
//    }
//    else
//        loginf << "JSONImporterTask: parseJSON: all " << all_cnt << " rec_num_cnt "<< rec_num_cnt_
//               << " to be inserted " << row_cnt << " skipped " << skipped;

}

void JSONImporterTask::insertData ()
{
    loginf << "JSONImporterTask: insertData: inserting into database";

    insert_active_ = true;

//    if (datasources_to_add_.size())
//    {
//        loginf << "JSONImporterTask: insertData: inserting " << datasources_to_add_.size() << " data sources";
//        db_object_->addDataSources(datasources_to_add_);

//        for (auto& src_it : datasources_to_add_)
//            datasources_existing_ [src_it.first] = src_it.second;
//        datasources_to_add_.clear();
//    }

    for (auto& map_it : mappings_)
    {
        if (map_it.buffer() != nullptr && map_it.buffer()->size() != 0)
        {
            DBObject& db_object = map_it.dbObject();

            loginf << "JSONImporterTask: insertData: " << db_object.name() << " connecting";

            connect (&db_object, &DBObject::insertDoneSignal, this, &JSONImporterTask::insertDoneSlot,
                     Qt::UniqueConnection);
            connect (&db_object, &DBObject::insertProgressSignal, this, &JSONImporterTask::insertProgressSlot,
                     Qt::UniqueConnection);

            loginf << "JSONImporterTask: insertData: " << db_object.name() << " inserting";

            db_object.insertData(map_it.variableList(), map_it.buffer());

            loginf << "JSONImporterTask: insertData: " << db_object.name() << " clearing";
            map_it.clearBuffer();
        }
        else
            loginf << "JSONImporterTask: insertData: emtpy buffer for " << map_it.dbObject().name();
    }


}

void JSONImporterTask::clearData ()
{
    for (auto& map_it : mappings_)
        map_it.clearBuffer();
}

void JSONImporterTask::insertProgressSlot (float percent)
{
    loginf << "JSONImporterTask: insertProgressSlot: " << String::percentToString(percent) << "%";
}

void JSONImporterTask::insertDoneSlot (DBObject& object)
{
    loginf << "JSONImporterTask: insertDoneSlot";
    insert_active_ = false;
}

void JSONImporterTask::checkAndSetVariable (std::string& name_str, DBOVariable** var)
{
    // TODO rework to only asserting, check must be done before
//    if (db_object_)
//    {
//        if (!db_object_->hasVariable(name_str))
//        {
//            loginf << "JSONImporterTask: checkAndSetVariable: var " << name_str << " does not exist";
//            name_str = "";
//            var = nullptr;
//        }
//        else
//        {
//            *var = &db_object_->variable(name_str);
//            loginf << "JSONImporterTask: checkAndSetVariable: var " << name_str << " set";
//            assert (var);
//            //assert((*var)->existsInDB());
//        }
//    }
//    else
//    {
//        loginf << "JSONImporterTask: checkAndSetVariable: dbobject null";
//        name_str = "";
//        var = nullptr;
//    }
}
