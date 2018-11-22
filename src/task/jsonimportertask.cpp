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
#include "jobmanager.h"
#include "jsonparsejob.h"
#include "jsonmappingjob.h"

#include <stdexcept>
#include <fstream>
#include <memory>
#include <algorithm>

#include <QDateTime>
#include <QCoreApplication>
#include <QThread>
#include <QMessageBox>

using namespace Utils;
using namespace nlohmann;

JSONImporterTask::JSONImporterTask(const std::string& class_id, const std::string& instance_id,
                                   TaskManager* task_manager)
    : Configurable (class_id, instance_id, task_manager)
{
    registerParameter("last_filename", &last_filename_, "");

    createSubConfigurables();
}

JSONImporterTask::~JSONImporterTask()
{
    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    if (msg_box_)
    {
        delete msg_box_;
        msg_box_ = nullptr;
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

    filename_ = filename;
    archive_ = false;
    test_ = test;
    all_done_ = false;

    start_time_ = boost::posix_time::microsec_clock::local_time();

    read_json_job_ = std::shared_ptr<ReadJSONFilePartJob> (new ReadJSONFilePartJob (filename, false, 10000));
    connect (read_json_job_.get(), SIGNAL(obsoleteSignal()), this, SLOT(readJSONFilePartObsoleteSlot()),
             Qt::QueuedConnection);
    connect (read_json_job_.get(), SIGNAL(doneSignal()), this, SLOT(readJSONFilePartDoneSlot()), Qt::QueuedConnection);

    JobManager::instance().addJob(read_json_job_);

    updateMsgBox();

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

    filename_ = filename;
    archive_ = true;
    test_ = test;
    all_done_ = false;

    start_time_ = boost::posix_time::microsec_clock::local_time();

    read_json_job_ = std::shared_ptr<ReadJSONFilePartJob> (new ReadJSONFilePartJob (filename, true, 10000));
    connect (read_json_job_.get(), SIGNAL(obsoleteSignal()), this, SLOT(readJSONFilePartObsoleteSlot()),
             Qt::QueuedConnection);
    connect (read_json_job_.get(), SIGNAL(doneSignal()), this, SLOT(readJSONFilePartDoneSlot()), Qt::QueuedConnection);

    JobManager::instance().addJob(read_json_job_);

    updateMsgBox();

    return;
}

void JSONImporterTask::createMappings ()
{
    logdbg << "JSONImporterTask: createMappings";

    if (mappings_.size() == 0)
    {
        unsigned int index;
        {
            index = mappings_.size();

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
            mappings_.at(index).JSONKey("message_type");
            mappings_.at(index).JSONValue("ads-b target");
            mappings_.at(index).overrideKeyVariable(true);
            mappings_.at(index).dataSourceVariableName("ds_id");

            mappings_.at(index).addMapping({"data_source_identifier.value", db_object.variable("ds_id"), true});
            mappings_.at(index).addMapping({"data_source_identifier.sac", db_object.variable("sac"), true});
            mappings_.at(index).addMapping({"data_source_identifier.sic", db_object.variable("sic"), true});
            mappings_.at(index).addMapping({"target_address", db_object.variable("target_addr"), true});
            mappings_.at(index).addMapping({"target_identification.value_idt", db_object.variable("callsign"), false});
            mappings_.at(index).addMapping({"mode_c_height.value_ft", db_object.variable("alt_baro_ft"), false,
                                            "Height", "Feet"});
            mappings_.at(index).addMapping({"wgs84_position.value_lat_rad", db_object.variable("pos_lat_deg"), true,
                                            "Angle", "Radian"});
            mappings_.at(index).addMapping({"wgs84_position.value_lon_rad", db_object.variable("pos_long_deg"), true,
                                            "Angle", "Radian"});
            mappings_.at(index).addMapping({"time_of_report", db_object.variable("tod"), true, "Time", "Second"});
            mappings_.at(index).initializeKey();
        }

        {
            index = mappings_.size();
            assert (ATSDB::instance().objectManager().existsObject("MLAT"));
            DBObject& db_object = ATSDB::instance().objectManager().object("MLAT");

            mappings_.push_back(JsonMapping (db_object));
            mappings_.at(index).JSONKey("message_type");
            mappings_.at(index).JSONValue("mlat target");
            mappings_.at(index).overrideKeyVariable(true);
            mappings_.at(index).dataSourceVariableName("ds_id");

            mappings_.at(index).addMapping({"data_source_identifier.value", db_object.variable("ds_id"), true});
            mappings_.at(index).addMapping({"data_source_identifier.sac", db_object.variable("sac"), true});
            mappings_.at(index).addMapping({"data_source_identifier.sic", db_object.variable("sic"), true});
            mappings_.at(index).addMapping({"mode_3a_info.code", db_object.variable("mode3a_code"), false});
            mappings_.at(index).addMapping({"target_address", db_object.variable("target_addr"), true});
            mappings_.at(index).addMapping({"target_identification.value_idt", db_object.variable("callsign"), false});
            mappings_.at(index).addMapping({"mode_c_height.value_ft", db_object.variable("flight_level_ft"), false,
                                            "Height", "Feet"});
            mappings_.at(index).addMapping({"wgs84_position.value_lat_rad", db_object.variable("pos_lat_deg"), true,
                                            "Angle", "Radian"});
            mappings_.at(index).addMapping({"wgs84_position.value_lon_rad", db_object.variable("pos_long_deg"), true,
                                            "Angle", "Radian"});
            mappings_.at(index).addMapping({"detection_time", db_object.variable("tod"), true, "Time", "Second"});
            mappings_.at(index).initializeKey();
        }

        {
            index = mappings_.size();
            assert (ATSDB::instance().objectManager().existsObject("Radar"));
            DBObject& db_object = ATSDB::instance().objectManager().object("Radar");

            mappings_.push_back(JsonMapping (db_object));
            mappings_.at(index).JSONKey("message_type");
            mappings_.at(index).JSONValue("radar target");
            mappings_.at(index).overrideKeyVariable(true);
            mappings_.at(index).dataSourceVariableName("ds_id");

            mappings_.at(index).addMapping({"data_source_identifier.value", db_object.variable("ds_id"), true});
            mappings_.at(index).addMapping({"data_source_identifier.sac", db_object.variable("sac"), true});
            mappings_.at(index).addMapping({"data_source_identifier.sic", db_object.variable("sic"), true});
            mappings_.at(index).addMapping({"mode_3_info.code", db_object.variable("mode3a_code"), false});
            mappings_.at(index).addMapping({"target_address", db_object.variable("target_addr"), false});
            mappings_.at(index).addMapping({"aircraft_identification.value_idt", db_object.variable("callsign"), false});
            mappings_.at(index).addMapping({"mode_c_height.value_ft", db_object.variable("modec_code_ft"), false,
                                            "Height", "Feet"});
            mappings_.at(index).addMapping({"measured_azm_rad", db_object.variable("pos_azm_deg"), true,
                                            "Angle", "Radian"});
            mappings_.at(index).addMapping({"measured_rng_m", db_object.variable("pos_range_nm"), true,
                                            "Length", "Meter"});
            mappings_.at(index).addMapping({"detection_time", db_object.variable("tod"), true, "Time", "Second"});
            mappings_.at(index).initializeKey();
        }

        {
            index = mappings_.size();
            assert (ATSDB::instance().objectManager().existsObject("Tracker"));
            DBObject& db_object = ATSDB::instance().objectManager().object("Tracker");

            mappings_.push_back(JsonMapping (db_object));
            mappings_.at(index).JSONKey("message_type");
            mappings_.at(index).JSONValue("track update");
            mappings_.at(index).overrideKeyVariable(true);
            mappings_.at(index).dataSourceVariableName("ds_id");

            mappings_.at(index).addMapping({"server_sacsic.value", db_object.variable("ds_id"), true});
            mappings_.at(index).addMapping({"server_sacsic.sac", db_object.variable("sac"), true});
            mappings_.at(index).addMapping({"server_sacsic.sic", db_object.variable("sic"), true});
            mappings_.at(index).addMapping({"mode_3a_info.code", db_object.variable("mode3a_code"), false});
            mappings_.at(index).addMapping({"aircraft_address", db_object.variable("target_addr"), true});
            mappings_.at(index).addMapping({"aircraft_identification.value_idt", db_object.variable("callsign"), false});
            mappings_.at(index).addMapping({"calculated_track_flight_level.value_feet", db_object.variable("modec_code_ft"),
                                            false, "Height", "Feet"});
            mappings_.at(index).addMapping({"calculated_wgs84_position.value_latitude_rad",
                                            db_object.variable("pos_lat_deg"), true, "Angle", "Radian"});
            mappings_.at(index).addMapping({"calculated_wgs84_position.value_longitude_rad",
                                            db_object.variable("pos_long_deg"), true, "Angle", "Radian"});
            mappings_.at(index).addMapping({"time_of_last_update", db_object.variable("tod"), true, "Time", "Second"});
            mappings_.at(index).initializeKey();
        }
    }
}

void JSONImporterTask::insertData ()
{
    loginf << "JSONImporterTask: insertData: inserting into database";

    while (insert_active_)
    {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep (10);
    }

    bool has_sac_sic = false;

    for (auto& map_it : mappings_)
    {
        if (buffers_.count(map_it.dbObject().name()) != 0)
        {
            ++insert_active_;

            DBObject& db_object = map_it.dbObject();
            std::shared_ptr<Buffer> buffer = buffers_.at(map_it.dbObject().name());

            has_sac_sic = db_object.hasVariable("sac") && db_object.hasVariable("sic")
                    && buffer->has<char>("sac") && buffer->has<char>("sic");

            logdbg << "JSONImporterTask: insertData: " << db_object.name() << " has sac/sic " << has_sac_sic;

            logdbg << "JSONImporterTask: insertData: " << db_object.name() << " buffer " << buffer->size();

            connect (&db_object, &DBObject::insertDoneSignal, this, &JSONImporterTask::insertDoneSlot,
                     Qt::UniqueConnection);
            connect (&db_object, &DBObject::insertProgressSignal, this, &JSONImporterTask::insertProgressSlot,
                     Qt::UniqueConnection);


            if (map_it.dataSourceVariableName() != "")
            {
                logdbg << "JSONImporterTask: insertData: adding new data sources";

                std::string data_source_var_name = map_it.dataSourceVariableName();


                // collect existing datasources
                std::set <int> datasources_existing;
                if (db_object.hasDataSources())
                    for (auto ds_it = db_object.dsBegin(); ds_it != db_object.dsEnd(); ++ds_it)
                        datasources_existing.insert(ds_it->first);

                // getting key list and distinct values
                assert (buffer->properties().hasProperty(data_source_var_name));
                assert (buffer->properties().get(data_source_var_name).dataType() == PropertyDataType::INT);

                assert(buffer->has<int>(data_source_var_name));
                ArrayListTemplate<int>& data_source_key_list = buffer->get<int> (data_source_var_name);
                std::set<int> data_source_keys = data_source_key_list.distinctValues();

                std::map <int, std::pair<char, char>> sac_sics; // keyvar->(sac,sic)
                // collect sac/sics
                if (has_sac_sic)
                {
                    ArrayListTemplate<char>& sac_list = buffer->get<char> ("sac");
                    ArrayListTemplate<char>& sic_list = buffer->get<char> ("sic");

                    size_t size = buffer->size();
                    int key_val;
                    for (unsigned int cnt=0; cnt < size; ++cnt)
                    {
                        key_val = data_source_key_list.get(cnt);

                        if (datasources_existing.count(key_val) != 0)
                            continue;

                        if (sac_sics.count(key_val) == 0)
                        {
                            logdbg << "JSONImporterTask: insertData: found new ds " << key_val << " for sac/sic";

                            assert (!sac_list.isNone(cnt) && !sic_list.isNone(cnt));
                            sac_sics[key_val] = std::pair<char, char> (sac_list.get(cnt), sic_list.get(cnt));

                            loginf << "JSONImporterTask: insertData: source " << key_val
                                   << " sac " << static_cast<int>(sac_list.get(cnt))
                                   << " sic " << static_cast<int>(sic_list.get(cnt));
                        }
                    }

                }

                // adding datasources
                std::map <int, std::pair<int,int>> datasources_to_add;

                for (auto ds_key_it : data_source_keys)
                    if (datasources_existing.count(ds_key_it) == 0 && added_data_sources_.count(ds_key_it) == 0)
                    {
                        if (datasources_to_add.count(ds_key_it) == 0)
                        {
                            logdbg << "JSONImporterTask: insertData: adding new data source " << ds_key_it;
                            if (sac_sics.count(ds_key_it) == 0)
                                datasources_to_add[ds_key_it] = {-1,-1};
                            else
                                datasources_to_add[ds_key_it] = {sac_sics.at(ds_key_it).first,
                                                                 sac_sics.at(ds_key_it).second};

                            added_data_sources_.insert(ds_key_it);
                        }
                    }

                if (datasources_to_add.size())
                {
                    db_object.addDataSources(datasources_to_add);
                }
            }

            logdbg << "JSONImporterTask: insertData: " << db_object.name() << " inserting";

            db_object.insertData(map_it.variableList(), buffer);
            objects_inserted_ += buffer->size();

            logdbg << "JSONImporterTask: insertData: " << db_object.name() << " clearing";
            map_it.clearBuffer();

            buffers_.erase(map_it.dbObject().name());
        }
        else
            logdbg << "JSONImporterTask: insertData: emtpy buffer for " << map_it.dbObject().name();
    }

    assert (buffers_.size() == 0);

    logdbg << "JSONImporterTask: insertData: done";
}

void JSONImporterTask::clearData ()
{
    for (auto& map_it : mappings_)
        map_it.clearBuffer();
}

void JSONImporterTask::updateMsgBox ()
{
    if (!msg_box_)
    {
        msg_box_ = new QMessageBox ();
        assert (msg_box_);
    }

    std::string msg;

    if (test_)
        msg = "Testing import of";
    else
        msg = "Importing";

    if (archive_)
        msg += " archive";
    msg += " file '"+filename_+"'\n";

    stop_time_ = boost::posix_time::microsec_clock::local_time();

    boost::posix_time::time_duration diff = stop_time_ - start_time_;

    std::string time_str = std::to_string(diff.hours())+"h "+std::to_string(diff.minutes())
            +"m "+std::to_string(diff.seconds())+"s";

    msg += "Elapsed Time: "+time_str+"\n";
    msg += "Bytes read: "+String::doubleToStringPrecision(static_cast<float>(bytes_read_)*1e-6,2)+" MB\n";
    msg += "Objects read: "+std::to_string(objects_read_)+"\n";
    msg += "Objects parsed: "+std::to_string(objects_parsed_)+"\n";
    msg += "Objects mapped: "+std::to_string(objects_mapped_)+"\n";
    msg += "Objects inserted: "+std::to_string(objects_inserted_)+"\n";
    if (diff.total_seconds() > 0)
        msg += "Object rate: "+std::to_string(objects_inserted_/diff.total_seconds())+" e/s";

    msg_box_->setText(msg.c_str());
    if (all_done_)
        msg_box_->setStandardButtons(QMessageBox::Ok);
    else
        msg_box_->setStandardButtons(QMessageBox::NoButton);

    msg_box_->show();
}

void JSONImporterTask::insertProgressSlot (float percent)
{
    logdbg << "JSONImporterTask: insertProgressSlot: " << String::percentToString(percent) << "%";
}

void JSONImporterTask::insertDoneSlot (DBObject& object)
{
    logdbg << "JSONImporterTask: insertDoneSlot";
    --insert_active_;

    if (read_json_job_ == nullptr && json_parse_jobs_.size() == 0 && json_map_jobs_.size() == 0 && insert_active_ == 0)
    {
        stop_time_ = boost::posix_time::microsec_clock::local_time();

        boost::posix_time::time_duration diff = stop_time_ - start_time_;

        std::string time_str = std::to_string(diff.hours())+"h "+std::to_string(diff.minutes())
                +"m "+std::to_string(diff.seconds())+"s";

        loginf << "JSONImporterTask: insertDoneSlot: read done after " << time_str;

        all_done_ = true;
    }

    updateMsgBox();
}

void JSONImporterTask::readJSONFilePartDoneSlot ()
{
    logdbg << "JSONImporterTask: readJSONFilePartDoneSlot";

    ReadJSONFilePartJob* read_job = dynamic_cast<ReadJSONFilePartJob*>(QObject::sender());
    assert (read_job);
    bytes_read_ = read_job->bytesRead();
    objects_read_ += read_job->objects().size();
    loginf << "JSONImporterTask: readJSONFilePartDoneSlot: bytes " << bytes_read_;

    //loginf << "got part '" << ss.str() << "'";

    // restart read job
    std::vector <std::string> objects = std::move(read_json_job_->objects());
    assert (!read_json_job_->objects().size());

    if (!read_json_job_->fileReadDone())
    {
        loginf << "JSONImporterTask: readJSONFilePartDoneSlot: read continue";
        read_json_job_->resetDone();
        JobManager::instance().addJob(read_json_job_);
    }
    else
        read_json_job_ = nullptr;

    // start parse job
    std::shared_ptr<JSONParseJob> json_parse_job = std::shared_ptr<JSONParseJob> (new JSONParseJob (std::move(objects)));
    connect (json_parse_job.get(), SIGNAL(obsoleteSignal()), this, SLOT(parseJSONObsoleteSlot()),
             Qt::QueuedConnection);
    connect (json_parse_job.get(), SIGNAL(doneSignal()), this, SLOT(parseJSONDoneSlot()), Qt::QueuedConnection);

    JobManager::instance().addJob(json_parse_job);

    json_parse_jobs_.push_back(json_parse_job);

    updateMsgBox();


}
void JSONImporterTask::readJSONFilePartObsoleteSlot ()
{
    logdbg << "JSONImporterTask: readJSONFilePartObsoleteSlot";
}

void JSONImporterTask::parseJSONDoneSlot ()
{
    logdbg << "JSONImporterTask: parseJSONDoneSlot";

    JSONParseJob* parse_job = dynamic_cast<JSONParseJob*>(QObject::sender());
    assert (parse_job);

    std::vector<json> json_objects = std::move(parse_job->jsonObjects());
    json_parse_jobs_.erase(json_parse_jobs_.begin());

    logdbg << "JSONImporterTask: parseJSONDoneSlot: " << json_objects.size() << " parsed objects";

    objects_parsed_ += json_objects.size();

    createMappings();

//    while (json_objects.size())
//    {
//        size_t count = 2000;

//        if (json_objects.size() < count)
//            count = json_objects.size();

//        std::vector<json> json_objects_part;

//        auto it = std::next(json_objects.begin(), count);

//        std::move(json_objects.begin(), it, std::back_inserter(json_objects_part));  // ##

//        json_objects.erase(json_objects.begin(), it);

//        std::shared_ptr<JSONMappingJob> json_map_job = std::shared_ptr<JSONMappingJob> (new JSONMappingJob (std::move(json_objects_part), mappings_));
//        connect (json_map_job.get(), SIGNAL(obsoleteSignal()), this, SLOT(mapJSONObsoleteSlot()),
//                 Qt::QueuedConnection);
//        connect (json_map_job.get(), SIGNAL(doneSignal()), this, SLOT(mapJSONDoneSlot()), Qt::QueuedConnection);

//        json_map_jobs_.push_back(json_map_job);

//        JobManager::instance().addJob(json_map_job);

//        for (auto& map_it : mappings_) // increase key counts
//            map_it.keyCount(map_it.keyCount()+count);
//    }

    int count = json_objects.size();

    std::shared_ptr<JSONMappingJob> json_map_job = std::shared_ptr<JSONMappingJob> (new JSONMappingJob (std::move(json_objects), mappings_));
    connect (json_map_job.get(), SIGNAL(obsoleteSignal()), this, SLOT(mapJSONObsoleteSlot()),
             Qt::QueuedConnection);
    connect (json_map_job.get(), SIGNAL(doneSignal()), this, SLOT(mapJSONDoneSlot()), Qt::QueuedConnection);

    json_map_jobs_.push_back(json_map_job);

    JobManager::instance().addJob(json_map_job);

    for (auto& map_it : mappings_) // increase key counts
        map_it.keyCount(map_it.keyCount()+count);

    updateMsgBox();
}

void JSONImporterTask::parseJSONObsoleteSlot ()
{
    logdbg << "JSONImporterTask: parseJSONObsoleteSlot";
}

void JSONImporterTask::mapJSONDoneSlot ()
{
    logdbg << "JSONImporterTask: mapJSONDoneSlot";

    JSONMappingJob* map_job = dynamic_cast<JSONMappingJob*>(QObject::sender());
    assert (map_job);

    std::vector <JsonMapping> mappings = map_job->mappings();

    json_map_jobs_.erase(json_map_jobs_.begin());

    for (auto& map_it : mappings)
    {
        if (map_it.hasFilledBuffer())
        {
            objects_mapped_ += map_it.buffer()->size();

            if (buffers_.count(map_it.dbObject().name()) == 0)
                buffers_[map_it.dbObject().name()] = map_it.buffer();
            else
            {
                buffers_.at(map_it.dbObject().name())->seizeBuffer(*map_it.buffer().get());
            }
        }
    }

    updateMsgBox();

 //   insertData();

    for (auto& buf_it : buffers_)
    {
        if (buf_it.second->size() > 10000)
        {
            loginf << "JSONImporterTask: mapJSONDoneSlot: inserting part of parsed objects";
            insertData ();
            return;
        }
    }

    if (read_json_job_ == nullptr && json_parse_jobs_.size() == 0 && json_map_jobs_.size() == 0)
    {
        loginf << "JSONImporterTask: mapJSONDoneSlot: inserting parsed objects at end";
        insertData ();
    }

}
void JSONImporterTask::mapJSONObsoleteSlot ()
{
    logdbg << "JSONImporterTask: mapJSONObsoleteSlot";
}
