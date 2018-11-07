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

#include <archive.h>
#include <archive_entry.h>

using namespace Utils;
using namespace nlohmann;

JSONImporterTask::JSONImporterTask(const std::string& class_id, const std::string& instance_id,
                                   TaskManager* task_manager)
    : Configurable (class_id, instance_id, task_manager)
{
    registerParameter("last_filename", &last_filename_, "");

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

    return true;
}

void JSONImporterTask::importFile(const std::string& filename, bool test)
{
    loginf << "JSONImporterTask: importFile: filename " << filename << " test " << test;

    if (!canImportFile(filename))
    {
        logerr << "JSONImporterTask: importFile: unable to import";
        return;
    }

    std::ifstream ifs(filename);
    std::stringstream ss;

    char c;
    unsigned int open_count {0};

    while (ifs.get(c))          // loop getting single characters
    {
        if (c == '{')
            ++open_count;
        else if (c == '}')
            --open_count;
        ss << c;

        if (c == '\n') // next lines after objects
            continue;

        if (open_count == 0)
        {
            //loginf << "got part '" << ss.str() << "'";

            json j = json::parse(ss.str());

            parseJSON (j, test);

            ss.str("");

            //loginf << "UGA2 cleared";
        }
    }

    if (!test)
    {
        transformBuffers();
        insertData ();
    }
    else
    {
        transformBuffers();
        clearData();
    }

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

    QMessageBox msg_box;
    std::string msg = "Importing archive '"+filename+"'.";
    msg_box.setText(msg.c_str());
    msg_box.setStandardButtons(QMessageBox::NoButton);
    msg_box.show();


    unsigned int entry_cnt = 0;

    //    size_t archive_read_time = 0;
    //    size_t archive_parse_time = 0;
    //    size_t json_parse_time = 0;
    //    size_t wait_time = 0;
    //    size_t insert_time = 0;

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

        //ss.str("");
        unsigned int open_count {0};

        for (;;)
        {
            r = archive_read_data_block(a, &buff, &size, &offset);

            if (r == ARCHIVE_EOF)
                break;
            if (r != ARCHIVE_OK)
                throw std::runtime_error("JSONImporterTask: importFileArchive: archive error: "
                                         +std::string(archive_error_string(a)));

            std::string str (reinterpret_cast<char const*>(buff), size);

            for (char c : str)
            {
                if (c == '{')
                    ++open_count;
                else if (c == '}')
                    --open_count;
                ss << c;

                if (c == '\n') // next lines after objects
                    continue;

                if (open_count == 0)
                {
                    loginf << "got part '" << ss.str() << "'";

                    json j = json::parse(ss.str());

                    parseJSON (j, test);

                    ss.str("");

                    //loginf << "UGA2 cleared";
                }
            }
        }

        //        archive_read_time += (boost::posix_time::microsec_clock::local_time() - tmp_time).total_milliseconds();
        //        tmp_time = boost::posix_time::microsec_clock::local_time();

        loginf  << "JSONImporterTask: importFileArchive: parsed entry";

        //        try
        //        {
        //            json j = json::parse(ss.str());

        //            archive_parse_time += (boost::posix_time::microsec_clock::local_time() - tmp_time).total_milliseconds();
        //            tmp_time = boost::posix_time::microsec_clock::local_time();

        //            parseJSON (j, test);

        //            json_parse_time += (boost::posix_time::microsec_clock::local_time() - tmp_time).total_milliseconds();
        //            tmp_time = boost::posix_time::microsec_clock::local_time();

        while (insert_active_)
        {
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            QThread::msleep (10);
        }
        //            wait_time += (boost::posix_time::microsec_clock::local_time() - tmp_time).total_milliseconds();
        //            tmp_time = boost::posix_time::microsec_clock::local_time();

        if (!test)
        {
            transformBuffers();
            insertData ();
        }
        else
        {
            transformBuffers();
            clearData();
        }

        //            insert_time += (boost::posix_time::microsec_clock::local_time() - tmp_time).total_milliseconds();

        //            loginf << "JSONImporterTask: importFileArchive: time: archive_read_time " << archive_read_time/1000.0
        //                   << " archive_parse_time " << archive_parse_time/1000.0
        //                   << " json_parse_time " << json_parse_time/1000.0
        //                   << " wait_time " << wait_time/1000.0
        //                   << " insert_time " << insert_time/1000.0;

        entry_cnt++;
        //        }
        //        catch (std::exception e)
        //        {
        //            logerr << "JSONImporterTask: importFileArchive: json parsing error: " << e.what()
        //                   << ", skipping entry ";
        //            continue;
        //        }
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

//        mappings_.push_back(JsonMapping (db_object));
//        mappings_.at(0).JSONKey("*");
//        mappings_.at(0).JSONValue("*");
//        mappings_.at(0).JSONContainerKey("acList");
//        mappings_.at(0).overrideKeyVariable(true);
//        mappings_.at(0).dataSourceVariableName("ds_id");

//        mappings_.at(0).addMapping({"Rcvr", db_object.variable("ds_id"), true});
//        mappings_.at(0).addMapping({"Icao", db_object.variable("target_addr"), true,
//                                    Format(PropertyDataType::STRING, "hexadecimal")});
//        mappings_.at(0).addMapping({"Reg", db_object.variable("callsign"), false});
//        mappings_.at(0).addMapping({"Alt", db_object.variable("alt_baro_ft"), false});
//        mappings_.at(0).addMapping({"GAlt", db_object.variable("alt_geo_ft"), false});
//        mappings_.at(0).addMapping({"Lat", db_object.variable("pos_lat_deg"), true});
//        mappings_.at(0).addMapping({"Long", db_object.variable("pos_long_deg"), true});
//        mappings_.at(0).addMapping({"PosTime", db_object.variable("tod"), true,
//                                    Format(PropertyDataType::STRING, "epoch_tod")});

        mappings_.push_back(JsonMapping (db_object));
        mappings_.at(0).JSONKey("message_type");
        mappings_.at(0).JSONValue("ads-b target");
        //mappings_.at(0).JSONContainerKey("acList");
        mappings_.at(0).overrideKeyVariable(true);
        mappings_.at(0).dataSourceVariableName("ds_id");

        mappings_.at(0).addMapping({"data_source_identifier.value", db_object.variable("ds_id"), true});
        mappings_.at(0).addMapping({"target_address", db_object.variable("target_addr"), true});
        mappings_.at(0).addMapping({"target_identification.value_idt", db_object.variable("callsign"), false});
        mappings_.at(0).addMapping({"mode_c_height.value_ft", db_object.variable("alt_baro_ft"), false});
        //mappings_.at(0).addMapping({"GAlt", db_object.variable("alt_geo_ft"), false});
        mappings_.at(0).addMapping({"wgs84_position.value_lat_rad", db_object.variable("pos_lat_deg"), true});
        mappings_.at(0).addMapping({"wgs84_position.value_lon_rad", db_object.variable("pos_long_deg"), true});
        mappings_.at(0).addMapping({"time_of_report", db_object.variable("tod"), true});
    }

    unsigned int row_cnt = 0;

    for (auto& map_it : mappings_)
    {
        row_cnt += map_it.parseJSON(j, test);
        loginf << "JSONImporterTask: parseJSON: parsed " << map_it.dbObject().name() << " with " << row_cnt << " rows";
    }

}

void JSONImporterTask::transformBuffers ()
{
    loginf << "JSONImporterTask: transformBuffers";

    for (auto& map_it : mappings_)
            if (map_it.buffer() != nullptr && map_it.buffer()->size() != 0)
                map_it.transformBuffer();
}

void JSONImporterTask::insertData ()
{
    loginf << "JSONImporterTask: insertData: inserting into database";

    for (auto& map_it : mappings_)
    {
        if (map_it.buffer() != nullptr && map_it.buffer()->size() != 0)
        {
            map_it.transformBuffer();

            insert_active_ = true;

            DBObject& db_object = map_it.dbObject();
            std::shared_ptr<Buffer> buffer = map_it.buffer();

            loginf << "JSONImporterTask: insertData: " << db_object.name() << " buffer " << buffer->size();

            connect (&db_object, &DBObject::insertDoneSignal, this, &JSONImporterTask::insertDoneSlot,
                     Qt::UniqueConnection);
            connect (&db_object, &DBObject::insertProgressSignal, this, &JSONImporterTask::insertProgressSlot,
                     Qt::UniqueConnection);


            if (map_it.dataSourceVariableName() != "")
            {
                logdbg << "JSONImporterTask: insertData: adding new data sources";

                std::string data_source_var_name = map_it.dataSourceVariableName();

                std::map <int, DBODataSource> datasources_existing;
                if (db_object.hasDataSources())
                    datasources_existing = db_object.dataSources();

                std::map <int, std::string> datasources_to_add;

                assert (buffer->properties().hasProperty(data_source_var_name));
                assert (buffer->properties().get(data_source_var_name).dataType() == PropertyDataType::INT);

                assert(buffer->has<int>(data_source_var_name));
                ArrayListTemplate<int>& data_source_key_list = buffer->get<int> (data_source_var_name);
                std::set<int> data_source_keys = data_source_key_list.distinctValues();

                for (auto ds_key_it : data_source_keys)
                    if (datasources_existing.count(ds_key_it) == 0 && added_data_sources_.count(ds_key_it) == 0)
                    {
                        if (datasources_to_add.count(ds_key_it) == 0)
                        {
                            logdbg << "JSONImporterTask: insertData: adding new data source "
                                   << std::to_string(ds_key_it);
                            datasources_to_add[ds_key_it] = std::to_string(ds_key_it);
                            added_data_sources_.insert(ds_key_it);
                        }
                    }

                db_object.addDataSources(datasources_to_add);
            }

            logdbg << "JSONImporterTask: insertData: " << db_object.name() << " inserting";

            db_object.insertData(map_it.variableList(), buffer);

            logdbg << "JSONImporterTask: insertData: " << db_object.name() << " clearing";
            map_it.clearBuffer();
        }
        else
            logdbg << "JSONImporterTask: insertData: emtpy buffer for " << map_it.dbObject().name();
    }

    loginf << "JSONImporterTask: insertData: done";
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

