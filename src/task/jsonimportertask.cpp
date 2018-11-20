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

    test_ = test;

    start_time_ = boost::posix_time::microsec_clock::local_time();

    read_json_job_ = std::shared_ptr<ReadJSONFilePartJob> (new ReadJSONFilePartJob (filename, false, 10000));
    connect (read_json_job_.get(), SIGNAL(obsoleteSignal()), this, SLOT(readJSONFilePartObsoleteSlot()),
             Qt::QueuedConnection);
    connect (read_json_job_.get(), SIGNAL(doneSignal()), this, SLOT(readJSONFilePartDoneSlot()), Qt::QueuedConnection);

    JobManager::instance().addJob(read_json_job_);

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

    test_ = test;

    start_time_ = boost::posix_time::microsec_clock::local_time();

    read_json_job_ = std::shared_ptr<ReadJSONFilePartJob> (new ReadJSONFilePartJob (filename, true, 10000));
    connect (read_json_job_.get(), SIGNAL(obsoleteSignal()), this, SLOT(readJSONFilePartObsoleteSlot()),
             Qt::QueuedConnection);
    connect (read_json_job_.get(), SIGNAL(doneSignal()), this, SLOT(readJSONFilePartDoneSlot()), Qt::QueuedConnection);

    JobManager::instance().addJob(read_json_job_);

//    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();





//                    read_time += (boost::posix_time::microsec_clock::local_time()
//                                  - tmp_time).total_nanoseconds();
//                    tmp_time = boost::posix_time::microsec_clock::local_time();

//                    json j = json::parse(ss.str());

//                    nloh_parse_time += (boost::posix_time::microsec_clock::local_time()
//                                        - tmp_time).total_nanoseconds();
//                    tmp_time = boost::posix_time::microsec_clock::local_time();

//                    parseJSON (j, test);

//                    ++parsed_objects;
//                    ss.str("");

//                    my_parse_time += (boost::posix_time::microsec_clock::local_time()
//                                      - tmp_time).total_nanoseconds();
//                    tmp_time = boost::posix_time::microsec_clock::local_time();

//                    if (parsed_objects != 0 && parsed_objects % 50000 == 0)
//                    {
//                        float num_secs = (boost::posix_time::microsec_clock::local_time()
//                                          - entry_start_time).total_milliseconds()/1000.0;

//                        std::string info_str = "in " + std::to_string(num_secs) + "s " +
//                                std::to_string(parsed_objects) + " parsed objects, "
//                                + String::doubleToStringPrecision(parsed_objects/num_secs,2) + " e/s"
//                                + " read time "
//                                + String::doubleToStringPrecision(read_time*1e-9,2) + "s"
//                                + " json parse time "
//                                + String::doubleToStringPrecision(nloh_parse_time*1e-9,2) + "s"
//                                + " mapping time "
//                                + String::doubleToStringPrecision(my_parse_time*1e-9,2) + "s"
//                                + " insert time "
//                                + String::doubleToStringPrecision(insert_time*1e-9,2) + "s";

//                        loginf << "JSONImporterTask: importFileArchive: " << info_str;

//                        msg_box.setInformativeText(info_str.c_str());

//                        if (!test)
//                        {
//                            transformBuffers();
//                            insertData ();
//                        }
//                        else
//                        {
//                            transformBuffers();
//                            clearData();
//                        }
//                    }

//                    insert_time += (boost::posix_time::microsec_clock::local_time()
//                                    - tmp_time).total_nanoseconds();
//                    tmp_time = boost::posix_time::microsec_clock::local_time();
//                }
//            }
//        }

//        float num_secs = (boost::posix_time::microsec_clock::local_time()
//                          - entry_start_time).total_milliseconds()/1000.0;

//        loginf << "JSONImporterTask: importFileArchive: final inserting after " << parsed_objects << " parsed objects, "
//               << String::doubleToStringPrecision(parsed_objects/num_secs,2) << " e/s";;

//        if (!test)
//        {
//            transformBuffers();
//            insertData ();
//        }
//        else
//        {
//            transformBuffers();
//            clearData();
//        }

//        entry_cnt++;
//    }


//    msg_box.close();

//    for (unsigned int cnt=0; cnt < 10; cnt++)
//        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

//    boost::posix_time::ptime stop_time = boost::posix_time::microsec_clock::local_time();
//    boost::posix_time::time_duration diff = stop_time - start_time;

//    std::string time_str = std::to_string(diff.hours())+"h "+std::to_string(diff.minutes())
//            +"m "+std::to_string(diff.seconds())+"s";

//    QMessageBox msgBox;
//    msg = "Reading archive " + filename + " with " + std::to_string(entry_cnt) + " entries finished successfully in "
//            + time_str+".\n";
//    if (all_cnt_)
//        msg +=  + "# of updates: " + std::to_string(all_cnt_)
//                + "\n# of skipped updates: " + std::to_string(skipped_cnt_)
//                + " (" +String::percentToString(100.0 * skipped_cnt_/all_cnt_) + "%)"
//                + "\n# of inserted updates: " + std::to_string(inserted_cnt_)
//                + " (" +String::percentToString(100.0 * inserted_cnt_/all_cnt_) + "%)";
//    msgBox.setText(msg.c_str());
//    msgBox.exec();

    return;
}

void JSONImporterTask::parseJSON (nlohmann::json& j, bool test)
{
    logdbg << "JSONImporterTask: parseJSON";

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
        }
    }

    for (auto& map_it : mappings_)
        map_it.parseJSON(j, test);
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

    while (insert_active_)
    {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep (10);
    }

    bool has_sac_sic = false;

    for (auto& map_it : mappings_)
    {
        if (map_it.buffer() != nullptr && map_it.buffer()->size() != 0)
        {
            ++insert_active_;

            DBObject& db_object = map_it.dbObject();
            std::shared_ptr<Buffer> buffer = map_it.buffer();

            has_sac_sic = db_object.hasVariable("sac") && db_object.hasVariable("sic")
                    && buffer->has<char>("sac") && buffer->has<char>("sic");

            loginf << "JSONImporterTask: insertData: " << db_object.name() << " has sac/sic " << has_sac_sic;

            loginf << "JSONImporterTask: insertData: " << db_object.name() << " buffer " << buffer->size();

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
                            loginf << "JSONImporterTask: insertData: found new ds " << key_val << " for sac/sic";

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
                            loginf << "JSONImporterTask: insertData: adding new data source " << ds_key_it;
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
    logdbg << "JSONImporterTask: insertDoneSlot";
    --insert_active_;
}

void JSONImporterTask::readJSONFilePartDoneSlot ()
{
    loginf << "JSONImporterTask: readJSONFilePartDoneSlot";

    //loginf << "got part '" << ss.str() << "'";

    std::vector <std::string> objects = std::move(read_json_job_->objects());
    assert (!read_json_job_->objects().size());

    for (auto& str_it : objects)
    {
        json j = json::parse(str_it);

        parseJSON (j, test_);
    }

    loginf << "JSONImporterTask: readJSONFilePartDoneSlot: inserting " << objects.size() << " parsed objects";
    if (!test_)
    {
        transformBuffers();
        insertData ();
    }
    else
    {
        transformBuffers();
        clearData();
    }

    if (read_json_job_->fileReadDone())
    {
        stop_time_ = boost::posix_time::microsec_clock::local_time();

        boost::posix_time::time_duration diff = stop_time_ - start_time_;

        std::string time_str = std::to_string(diff.hours())+"h "+std::to_string(diff.minutes())
                +"m "+std::to_string(diff.seconds())+"s";

        loginf << "JSONImporterTask: readJSONFilePartDoneSlot: read done after " << time_str;

        QMessageBox msgBox;
        std::string msg = "Reading archive finished successfully.\n";
        msg +=  + "In " + time_str;
        msgBox.setText(msg.c_str());
        msgBox.exec();
    }
    else
    {
        loginf << "JSONImporterTask: readJSONFilePartDoneSlot: read continue";
        read_json_job_->resetDone();
        JobManager::instance().addJob(read_json_job_);
    }

//    QMessageBox msgBox;
//    std::string msg = "Reading archive " + filename + " finished successfully.\n";
//    if (all_cnt_)
//        msg +=  + "# of updates: " + std::to_string(all_cnt_)
//                + "\n# of skipped updates: " + std::to_string(skipped_cnt_)
//                + " (" +String::percentToString(100.0 * skipped_cnt_/all_cnt_) + "%)"
//                + "\n# of inserted updates: " + std::to_string(inserted_cnt_)
//                + " (" +String::percentToString(100.0 * inserted_cnt_/all_cnt_) + "%)";
//    msgBox.setText(msg.c_str());
//    msgBox.exec();
}
void JSONImporterTask::readJSONFilePartObsoleteSlot ()
{
    loginf << "JSONImporterTask: readJSONFilePartObsoleteSlot";
}
