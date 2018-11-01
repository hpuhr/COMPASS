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

#include <jsoncpp/json/json.h>

#include <archive.h>
#include <archive_entry.h>

using namespace Utils;

JSONImporterTask::JSONImporterTask(const std::string& class_id, const std::string& instance_id,
                                   TaskManager* task_manager)
    : Configurable (class_id, instance_id, task_manager)
{
    registerParameter("last_filename", &last_filename_, "");
    registerParameter("db_object_str", &db_object_str_, "ADSB");

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

    key_var_str_ = "rec_num";
    dsid_var_str_ = "ds_id";
    target_addr_var_str_ = "target_addr";
    callsign_var_str_ = "callsign";
    altitude_baro_var_str_ = "alt_baro_ft";
    altitude_geo_var_str_ = "alt_geo_ft";
    latitude_var_str_ = "pos_lat_deg";
    longitude_var_str_ = "pos_long_deg";
    tod_var_str_ = "tod";

    createSubConfigurables();
}

JSONImporterTask::~JSONImporterTask()
{
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

std::string JSONImporterTask::dbObjectStr() const
{
    return db_object_str_;
}

void JSONImporterTask::dbObjectStr(const std::string& db_object_str)
{
    loginf << "JSONImporterTask: dbObjectStr: " << db_object_str;

    db_object_str_ = db_object_str;

    assert (ATSDB::instance().objectManager().existsObject(db_object_str_));
    db_object_ = &ATSDB::instance().objectManager().object(db_object_str_);
}

bool JSONImporterTask::canImportFile (const std::string& filename)
{
    if (!Files::fileExists(filename))
    {
        loginf << "JSONImporterTask: canImportFile: not possible since file does not exist";
        return false;
    }

    if (!ATSDB::instance().objectManager().existsObject(db_object_str_))
    {
        loginf << "JSONImporterTask: canImportFile: not possible since DBObject does not exist";
        return false;
    }

    DBObject& object = ATSDB::instance().objectManager().object(db_object_str_);

    if (!key_var_str_.size()
            || !dsid_var_str_.size()
            || !target_addr_var_str_.size()
            || !callsign_var_str_.size()
            || !altitude_baro_var_str_.size()
            || !altitude_geo_var_str_.size()
            || !latitude_var_str_.size()
            || !longitude_var_str_.size()
            || !tod_var_str_.size())
    {
        loginf << "JSONImporterTask: canImportFile: not possible since variables are not set";
        return false;
    }

    if (!object.hasVariable(key_var_str_)
            || !object.hasVariable(dsid_var_str_)
            || !object.hasVariable(target_addr_var_str_)
            || !object.hasVariable(callsign_var_str_)
            || !object.hasVariable(altitude_baro_var_str_)
            || !object.hasVariable(altitude_geo_var_str_)
            || !object.hasVariable(latitude_var_str_)
            || !object.hasVariable(longitude_var_str_)
            || !object.hasVariable(tod_var_str_))
    {
        loginf << "JSONImporterTask: canImportFile: not possible since variables not in DBObject";
        return false;
    }

    if (use_time_filter_ && time_filter_min_ >= time_filter_max_)
    {
        loginf << "JSONImporterTask: canImportFile: time filter values wrong";
        return false;
    }

    return true;
}

void JSONImporterTask::importFile(const std::string& filename, bool test)
{
    loginf << "JSONImporterTask: importFile: filename " << filename << " test " << test;

    if (db_object_str_.size())
    {
        if (!ATSDB::instance().objectManager().existsObject(db_object_str_))
            db_object_str_="";
        else
            db_object_ = &ATSDB::instance().objectManager().object(db_object_str_);
    }
    assert (db_object_);

    if (!canImportFile(filename))
    {
        logerr << "JSONImporterTask: importFile: unable to import";
        return;
    }

    std::ifstream ifs(filename);
    Json::Reader reader;
    Json::Value obj;
    reader.parse(ifs, obj); // reader can also read strings

    std::shared_ptr<Buffer> buffer_ptr = parseJSON (obj, test);

    if (!test)
        insertData (buffer_ptr);

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

    if (db_object_str_.size())
    {
        if (!ATSDB::instance().objectManager().existsObject(db_object_str_))
            db_object_str_="";
        else
            db_object_ = &ATSDB::instance().objectManager().object(db_object_str_);
    }
    assert (db_object_);

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
    Json::Reader reader;
    Json::Value obj;

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
        reader.parse(ss.str(), obj); // reader can also read strings

        archive_parse_time += (boost::posix_time::microsec_clock::local_time() - tmp_time).total_milliseconds();
        tmp_time = boost::posix_time::microsec_clock::local_time();

        std::shared_ptr<Buffer> buffer_ptr = parseJSON (obj, test);

        json_parse_time += (boost::posix_time::microsec_clock::local_time() - tmp_time).total_milliseconds();
        tmp_time = boost::posix_time::microsec_clock::local_time();

        if (buffer_ptr->size())
        {
            while (insert_active_)
            {
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                QThread::msleep (10);
            }
            wait_time += (boost::posix_time::microsec_clock::local_time() - tmp_time).total_milliseconds();
            tmp_time = boost::posix_time::microsec_clock::local_time();

            if (!test)
                insertData (buffer_ptr);

            insert_time += (boost::posix_time::microsec_clock::local_time() - tmp_time).total_milliseconds();

            loginf << "JSONImporterTask: importFileArchive: time: archive_read_time " << archive_read_time/1000.0
                   << " archive_parse_time " << archive_parse_time/1000.0
                   << " json_parse_time " << json_parse_time/1000.0
                   << " wait_time " << wait_time/1000.0
                   << " insert_time " << insert_time/1000.0;
        }
        entry_cnt++;
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

std::shared_ptr<Buffer> JSONImporterTask::parseJSON (Json::Value& object, bool test)
{
    loginf << "JSONImporterTask: parseJSON";

    if (!key_var_ && key_var_str_.size())
        checkAndSetVariable (key_var_str_, &key_var_);
    if (!dsid_var_ && dsid_var_str_.size())
        checkAndSetVariable (dsid_var_str_, &dsid_var_);
    if (!target_addr_var_ && target_addr_var_str_.size())
        checkAndSetVariable (target_addr_var_str_, &target_addr_var_);
    if (!callsign_var_ && callsign_var_str_.size())
        checkAndSetVariable (callsign_var_str_, &callsign_var_);
    if (!altitude_baro_var_ && altitude_baro_var_str_.size())
        checkAndSetVariable (altitude_baro_var_str_, &altitude_baro_var_);
    if (!altitude_geo_var_ && altitude_geo_var_str_.size())
        checkAndSetVariable (altitude_geo_var_str_, &altitude_geo_var_);
    if (!latitude_var_ && latitude_var_str_.size())
        checkAndSetVariable (latitude_var_str_, &latitude_var_);
    if (!longitude_var_ && longitude_var_str_.size())
        checkAndSetVariable (longitude_var_str_, &longitude_var_);
    if (!tod_var_ && tod_var_str_.size())
        checkAndSetVariable (tod_var_str_, &tod_var_);

    assert (key_var_ && key_var_->hasCurrentDBColumn());
    assert (dsid_var_ && dsid_var_->hasCurrentDBColumn());
    assert (target_addr_var_ && target_addr_var_->hasCurrentDBColumn());
    assert (callsign_var_ && callsign_var_->hasCurrentDBColumn());
    assert (altitude_baro_var_ && altitude_baro_var_->hasCurrentDBColumn());
    assert (altitude_geo_var_ && altitude_geo_var_->hasCurrentDBColumn());
    assert (latitude_var_ && latitude_var_->hasCurrentDBColumn());
    assert (longitude_var_ && longitude_var_->hasCurrentDBColumn());
    assert (tod_var_ && tod_var_->hasCurrentDBColumn());

    if (list_.size() == 0) // only if empty, only at first time
    {
        list_.addProperty(key_var_->name(), key_var_->dataType());
        list_.addProperty(dsid_var_->name(), dsid_var_->dataType());
        list_.addProperty(target_addr_var_->name(), target_addr_var_->dataType());
        list_.addProperty(callsign_var_->name(), callsign_var_->dataType());
        list_.addProperty(altitude_baro_var_->name(), altitude_baro_var_->dataType());
        list_.addProperty(altitude_geo_var_->name(), altitude_geo_var_->dataType());
        list_.addProperty(latitude_var_->name(), latitude_var_->dataType());
        list_.addProperty(longitude_var_->name(), longitude_var_->dataType());
        list_.addProperty(tod_var_->name(), tod_var_->dataType());
    }

    std::shared_ptr<Buffer> buffer_ptr = std::shared_ptr<Buffer> (new Buffer (list_, db_object_->name()));

    ArrayListTemplate<int>& key_al = buffer_ptr->get<int>(key_var_->name());
    ArrayListTemplate<int>& dsid_al = buffer_ptr->get<int>(dsid_var_->name());
    ArrayListTemplate<int>& target_addr_al = buffer_ptr->get<int>(target_addr_var_->name());
    ArrayListTemplate<std::string>& callsign_al = buffer_ptr->get<std::string>(callsign_var_->name());
    ArrayListTemplate<int>& altitude_baro_al = buffer_ptr->get<int>(altitude_baro_var_->name());
    ArrayListTemplate<double>& altitude_geo_al = buffer_ptr->get<double>(altitude_geo_var_->name());
    ArrayListTemplate<double>& latitude_al = buffer_ptr->get<double>(latitude_var_->name());
    ArrayListTemplate<double>& longitude_al = buffer_ptr->get<double>(longitude_var_->name());
    ArrayListTemplate<float>& tod_al = buffer_ptr->get<float>(tod_var_->name());

//    unsigned int all_cnt = 0;
    bool receiver_valid;
    int receiver;
    std::string receiver_name;
    bool target_address_valid;
    unsigned int target_address;
    bool callsign_valid;
    std::string callsign;
    bool altitude_baro_valid;
    float altitude_baro_ft;
    bool altitude_geo_valid;
    float altitude_geo_ft;
    bool latitude_valid;
    double latitude_deg;
    bool longitude_valid;
    double longitude_deg;
    bool time_valid;
    unsigned long epoch_ms;
    QDateTime date_time;
    double tod;
    bool mlat_valid;
    bool mlat;

//    unsigned int skipped = 0;
    unsigned int row_cnt = 0;

    for (auto& src_it : db_object_->dataSources())
        if (datasources_existing_.count(src_it.first) == 0)
            datasources_existing_[src_it.first] = src_it.second.name();

    bool skip_this;

    for (auto it = object.begin(); it != object.end(); ++it)
    {
        logdbg << it.key().asString(); // << ':' << it->asInt() << '\n';

        if (it.key().asString() == "acList")
        {
            Json::Value ac_list = object["acList"];
            //Json::Value& ac_list = it.v;
            for (auto tr_it = ac_list.begin(); tr_it != ac_list.end(); ++tr_it)
            {
                skip_this = false;
                //loginf << tr_it.key().asString();

                // from https://www.adsbexchange.com/datafields/
                //
                //    Id (integer) – “Unique Identifier” of the aircraft, as listed in the VRS basestation.sqb database.
                // Likely not very useful.  Do not depend on this field, could change.

                //    Rcvr (integer) – “Receiver ID number”.  Prior to April 27, 2017, this was always “1”.  After that,
                // it is a unique ID referring to which specific receiver logged the data. Receiver number is 5 to 7
                // digit integer, with the format of “RRRYXXX” Brief explanation below:
                //        RRR – Receiving server.  Could be from 1 – 3 digits.
                //            Servers 1-7 are servers that receive load-balanced incoming connections, either in VRS or
                // Beast/RAW format.  Incoming feeds are dynamically assigned to one of these servers, and the receiver
                // ID “XXX” is also dynamically assigned.
                //            Server 100 is the “Custom Feed” server, where feeders can send to a specific port and
                // always get the same feeder ID.
                //        Y – Type of feed.
                //            1 = Beast/RAW
                //            3 = Compressed VRS
                //            5 = Satellite/Inmarsat/JAERO
                //            6 = Aggregated from a group of receivers
                //        XXX – a unique number assigned to feeds.  Static on server 100.  Dynamic on other servers.
                receiver_valid = !(*tr_it)["Rcvr"].isNull();
                if (receiver_valid)
                {
                    receiver = (*tr_it)["Rcvr"].asUInt();
                    receiver_name = std::to_string(receiver);
                }
                else
                    skip_this = true;

                // moved down to mlat
//                if (datasources_existing_.count(receiver) == 0 && datasources_to_add.count(receiver) == 0)
//                    datasources_to_add[receiver] = std::to_string(receiver);

                //    HasSig (boolean) – True if the aircraft has a signal level associated with it. The level will be
                // included in the “Sig” field.

                //    Sig (number) – The signal level for the last message received from the aircraft, as reported by
                // the receiver. Not all receivers pass signal levels. The value’s units are receiver-dependent.

                //    Icao (six-digit hex) – One of the most important fields. This is the six-digit hexadecimal
                // identifier broadcast by the aircraft over the air in order to identify itself.  Blocks of these
                // codes are assigned to countries by the International Civil Aviation Organization (ICAO).  Each
                // country then assigns individual codes to aircraft registered in that country. There should generally
                // be a one to one correlation between an aircraft registration number and ICAO hex code.  The ICAO hex
                // identifier can be used to lookup information such as aircraft registration, type, owner/operator,
                // etc in various databases around the internet such as www.airframes.org.  It should be noted that
                // generally the ICAO hex code remains the same as long as the aircraft’s “Registration number” remains
                // the same.  If the registration number changes, which can happen sometimes when an aircraft is sold
                // to an owner in another country, the ICAO hex code will also change.

                target_address_valid = !(*tr_it)["Icao"].isNull();
                if (target_address_valid)
                    target_address = String::intFromHexString((*tr_it)["Icao"].asString());
                else
                    skip_this = true;

                //    Reg (alphanumeric) – Aircraft registration number.  This is looked up via a database based on the
                // ICAO code.  This information is only as good as the database, and is not pulled off the airwaves. It
                // is not broadcast by the aircraft.
                callsign_valid = !(*tr_it)["Reg"].isNull();

                if (callsign_valid)
                    callsign = (*tr_it)["Reg"].asString();

                //    Fseen (datetime – epoch format) – date and time the receiver first started seeing the aircraft on
                // this flight.  Accurate for a single receiver, but as the plane is detected by different receivers,
                // this will change.

                //    Tsecs (integer) – The number of seconds that the aircraft has been tracked for.  Will change as
                // aircraft roams between receiving servers.

                //    Cmsgs (integer) – The count of messages received for the aircraft.  Will change as aircraft roams
                // between receiving servers.

                //    Alt (integer) – The altitude in feet at standard pressure. (broadcast by the aircraft)
                altitude_baro_valid = !(*tr_it)["Alt"].isNull();
                if (altitude_baro_valid)
                    altitude_baro_ft = (*tr_it)["Alt"].asInt();

                //    Galt (integer) – The altitude adjusted for local air pressure, should be roughly the height above
                // mean sea level.
                altitude_geo_valid = !(*tr_it)["Galt"].isNull();
                if (altitude_geo_valid)
                    altitude_geo_ft = (*tr_it)["Galt"].asInt();

                //    InHG (float) – The air pressure in inches of mercury that was used to calculate the AMSL altitude
                // from the standard pressure altitude.

                //    AltT (boolean) – The type of altitude transmitted by the aircraft: 0 = standard pressure altitude,
                // 1 = indicated altitude (above mean sea level). Default to standard pressure altitude until told
                // otherwise.

                //    Lat (float) – The aircraft’s latitude over the ground.
                latitude_valid = !(*tr_it)["Lat"].isNull();
                if (latitude_valid)
                    latitude_deg = (*tr_it)["Lat"].asFloat();
                else
                    skip_this = true;

                //    Long (float) – The aircraft’s longitude over the ground.
                longitude_valid =!(*tr_it)["Long"].isNull();
                if (longitude_valid)
                    longitude_deg = (*tr_it)["Long"].asFloat();
                else
                    skip_this = true;

                if (!skip_this && use_position_filter_
                        && (latitude_deg < pos_filter_lat_min_ || latitude_deg > pos_filter_lat_max_
                        || longitude_deg < pos_filter_lon_min_ || longitude_deg > pos_filter_lon_max_))
                    skip_this = true;

                //    PosTime (epoch milliseconds) – The time (at UTC in JavaScript ticks, UNIX epoch format in
                // milliseconds) that the position was last reported by the aircraft. This field is the time at which
                // the aircraft was at the lat/long/altitude reported above. https://www.epochconverter.com/ may be
                // helpful.
                time_valid = !(*tr_it)["PosTime"].isNull();
                if (time_valid)
                {
                    epoch_ms = (*tr_it)["PosTime"].asLargestUInt();
                    date_time.setMSecsSinceEpoch(epoch_ms);
                    tod = String::timeFromString(date_time.toString("hh:mm:ss.zzz").toStdString());

                    if (use_time_filter_ && (tod < time_filter_min_ || tod > time_filter_max_))
                        skip_this = true;
                }
                else
                    skip_this = true;

                //    Mlat (boolean) – True if the latitude and longitude appear to have been calculated by an MLAT
                // (multilateration) server and were not transmitted by the aircraft. Multilateration is based on the
                // time difference that specific receivers detect the signal and a mathematical calculation.  It is
                // significantly less accurate than ADS-B, which is based on GPS, and more likely to result in jagged
                // aircraft tracks. Aircraft that have Mode S (and have not upgraded to ADS-B) can sometimes be tracked
                // via multilateration.  It requires 3-4 ground stations in different locations to be receiving the
                // aircraft signal simultaneously in order to allow the calculation.
                mlat_valid = !(*tr_it)["Mlat"].isNull();

                if (mlat_valid)
                    mlat = (*tr_it)["Mlat"].asBool();

                if (join_data_sources_) // init for ADS-B
                {
                    receiver_valid = true;
                    receiver = 0;
                    receiver_name = "ADS-B";

                    if (mlat_valid && separate_mlat_data_ && mlat)
                    {
                        receiver = 1;
                        receiver_name = "MLAT";
                    }
                }

                //    TisB (boolean) – True if the last message received for the aircraft was from a TIS-B source.

                //    Spd (knots, float) – The ground speed in knots.

                //    SpdTyp (integer) – The type of speed that Spd represents. Only used with raw feeds.
                // 0/missing = ground speed, 1 = ground speed reversing, 2 = indicated air speed, 3 = true air speed.

                //    Trak (degrees, float) – Aircraft’s track angle across the ground clockwise from 0° north.

                //    TrkH (boolean) – True if Trak is the aircraft’s heading, false if it’s the ground track. Default
                // to ground track until told otherwise.

                //    Type (alphanumeric) – The aircraft model’s ICAO type code. This is looked up via a database based
                // on the ICAO code.  This information is only as good as the database, and is not pulled off the
                // airwaves. It is not broadcast by the aircraft.
                //        List of ICAO Types
                //        Partial List of ICAO Type codes – Wikipedia

                //    Mdl (string) – A description of the aircraft’s model. Usually also includes the manufacturer’s
                // name. This is looked up via a database based on the ICAO code.  This information is only as good as
                // the database, and is not pulled off the airwaves. It is not broadcast by the aircraft.

                //    Man (string) – The manufacturer’s name. This is looked up via a database based on the ICAO code.
                // This information is only as good as the database, and is not pulled off the airwaves. It is not
                // broadcast by the aircraft.

                //    Year (integer) – The year that the aircraft was manufactured. This information is only as good as
                // the database, and is not pulled off the airwaves. It is not broadcast by the aircraft.

                //    Cnum (alphanumeric) – The aircraft’s construction or serial number. This is looked up via a
                // database based on the ICAO code.  This information is only as good as the database, and is not
                // pulled off the airwaves. It is not broadcast by the aircraft.

                //    Op (String) – The name of the aircraft’s operator. This information is only as good as the
                // database, and is not pulled off the airwaves. It is not broadcast by the aircraft.

                //    OpIcao (string) – The ICAO code of the operator.  Non-exhaustive list here:
                // https://en.wikipedia.org/wiki/List_of_airline_codes

                //    Sqk (4-digit integer) – Transponder code.  This is a 4-digit code (each digit is from 0-7)
                // entered by the pilot, and typically assigned by air traffic control.  A sqwak code of 1200 typically
                // means the aircraft is operation under VFR and not receiving radar services.
                // 7500 = Hijack code, 7600 = Lost Communications, radio problem, 7700 = Emergency.
                // More info on codes here: https://en.wikipedia.org/wiki/Transponder_(aeronautics)

                //    Vsi (integer) – Vertical speed in feet per minute. Broadcast by the aircraft.

                //    VsiT (boolean) – 0 = vertical speed is barometric, 1 = vertical speed is geometric. Default to
                // barometric until told otherwise.

                //    WTC (integer) – Wake Turbulence category.  Broadcast by the aircraft.
                //        0 = None
                //        1 = Light
                //        2 = Medium
                //        3 = Heavy

                //    Species (integer) – General Aircraft Type. This is looked up via a database based on the ICAO
                // code.  This information is only as good as the database, and is not pulled off the airwaves. It is
                // not broadcast by the aircraft.
                //        0 = None
                //        1 = Land Plane
                //        2 = Sea Plane
                //        3 = Amphibian
                //        4 = Helicopter
                //        5 = Gyrocopter
                //        6 = Tiltwing
                //        7 = Ground Vehicle
                //        8 = Tower

                //    EngType (integer) – Type of engine the aircraft uses. This is looked up via a database based on
                // the ICAO code.  This information is only as good as the database, and is not pulled off the airwaves.
                // It is not broadcast by the aircraft.
                //        0 = None
                //        1 = Piston
                //        2 = Turboprop
                //        3 = Jet
                //        4 = Electric

                //    EngMount (integer) – The placement of engines on the aircraft. This is looked up via a database
                // based on the ICAO code.  This information is only as good as the database, and is not pulled off the
                // airwaves. It is not broadcast by the aircraft.
                //        0 = Unknown
                //        1 = Aft Mounted
                //        2 = Wing Buried
                //        3 = Fuselage Buried
                //        4 = Nose Mounted
                //        5 = Wing Mounted

                //    Engines (alphanumeric) – The number of engines the aircraft has. Usually ‘1’, ‘2’ etc. but can
                // also be a string – see ICAO documentation.

                //    Mil (boolean) – True if the aircraft appears to be operated by the military. Based on certain
                // range of ICAO hex codes that the aircraft broadcasts.

                //    Cou (string) – The country that the aircraft is registered to.  Based on the ICAO hex code range
                // the aircraft is broadcasting.

                //    From (string) – The code and name of the departure airport. Based on user-reported routes, and is
                // quite often wrong.  Don’t depend on it.

                //    To (string) – The code and name of the destination airport. Based on user-reported routes, and is
                // quite often wrong.  Don’t depend on it.

                //    Gnd (boolean) – True if the aircraft is on the ground. Broadcast by transponder.

                //    Call (alphanumeric) – The callsign of the aircraft.  Typically, this can be set by the pilot by
                // entering it into the transponder prior to flight.  Some aircraft simply leave it as their
                // registration number.  Occasionally, you might see “interesting” things (like jokes) entered in this
                // field by the flight crew.

                //    CallSus (boolean) – True if the callsign may not be correct. Based on a checksum of the data
                // received over the air.

                //    HasPic (boolean) – True if the aircraft has a picture associated with it in the VRS/ADSBexchange
                // database. Pictures often link to http://www.airport-data.com

                //    FlightsCount (integer) – The number of Flights records the aircraft has in the database. This
                // value is typically zero based on some internal factors.

                //    Interested (boolean) – is the aircraft marked as interesting in the database.

                //    Help (boolean) – True if the aircraft is transmitting an emergency sqwak code (i.e. 7700).

                //    Trt (integer) – Transponder type.  Click here for more detailed explanation, see page 2.
                //        0=Unknown
                //        1=Mode-S
                //        2=ADS-B (unknown version)
                //        3=ADS-B 0 – DO-260
                //        4=ADS-B 1 – DO-260 (A)
                //        5=ADS-B 2 – DO-260 (B)

                //    TT (string) – Trail type – empty for plain trails, ‘a’ for trails that include altitude, ‘s’ for
                // trails that include speed. This is a Virtual Radar Server parameter and is not a property of the
                // aircraft or transmission.


                //    ResetTrail (boolean) – Internal use, do not use.

                //    Talt (number) – The target altitude, in feet, set on the autopilot / FMS etc. Broadcast by the
                // aircraft.

                //    Ttrk (number) – The track or heading currently set on the aircraft’s autopilot or FMS. Broadcast
                // by the aircraft.

                //    Sat (boolean) – True if the data has been received via a SatCom ACARS feed (e.g. a JAERO feed).

                //    PosStale (boolean) – True if the last position update is older than the display timeout value –
                // usually only seen on MLAT aircraft in merged feeds. Internal field, basically means that the position
                // data is > 60 seconds old (unless it’s from Satellite ACARS).

                //    Source (string) – Internal use only.

                if (!skip_this)
                {
                    logdbg << "\t rn " << rec_num_cnt_
                           << " rc " << receiver
                           << " ta " << target_address
                           << " cs " << (callsign_valid ? callsign : "NULL")
                           << " alt_baro " << (altitude_baro_valid ? std::to_string(altitude_baro_ft) : "NULL")
                           << " alt_geo " << (altitude_geo_valid ? std::to_string(altitude_geo_ft) : "NULL")
                           << " lat " << std::to_string(latitude_deg)
                           << " lon " << std::to_string(longitude_deg)
                           << " dt " << date_time.toString("yyyy.MM.dd hh:mm:ss.zzz").toStdString();

                    key_al.set(row_cnt, rec_num_cnt_);
                    dsid_al.set(row_cnt, receiver);
                    target_addr_al.set(row_cnt, target_address);

                    if (callsign_valid)
                        callsign_al.set(row_cnt, callsign);
                    else
                        callsign_al.setNone(row_cnt);

                    if (altitude_baro_valid)
                        altitude_baro_al.set(row_cnt, altitude_baro_ft);
                    else
                        altitude_baro_al.setNone(row_cnt);

                    if (altitude_geo_valid)
                        altitude_geo_al.set(row_cnt, altitude_geo_ft);
                    else
                        altitude_geo_al.setNone(row_cnt);

                    latitude_al.set(row_cnt, latitude_deg);
                    longitude_al.set(row_cnt, longitude_deg);
                    tod_al.set(row_cnt, (int) tod);

                    row_cnt++;
                    rec_num_cnt_++;
                    inserted_cnt_++;

                    if (datasources_existing_.count(receiver) == 0 && datasources_to_add_.count(receiver) == 0)
                        datasources_to_add_[receiver] = receiver_name;
                }
                else
                    skipped_cnt_++;

                all_cnt_++;
            }
        }
    }
    assert (buffer_ptr->size() == row_cnt);

//    if (buffer_ptr->size() != 0)
//    {
//        if (!test)
//        {
            if (var_list_.getSize() == 0) // fill if empty, only at first time
            {
                var_list_.add(*key_var_);
                var_list_.add(*dsid_var_);
                var_list_.add(*target_addr_var_);
                var_list_.add(*callsign_var_);
                var_list_.add(*altitude_baro_var_);
                var_list_.add(*altitude_geo_var_);
                var_list_.add(*latitude_var_);
                var_list_.add(*longitude_var_);
                var_list_.add(*tod_var_);
            }
//        }

//        loginf << "JSONImporterTask: parseJSON: all " << all_cnt << " rec_num_cnt " << rec_num_cnt_
//               << " to be inserted " << row_cnt << " (" << String::percentToString(100.0 * row_cnt/all_cnt) << "%)"
//               << " skipped " << skipped<< " (" << String::percentToString(100.0 * skipped/all_cnt) << "%)";
//    }
//    else
//        loginf << "JSONImporterTask: parseJSON: all " << all_cnt << " rec_num_cnt "<< rec_num_cnt_
//               << " to be inserted " << row_cnt << " skipped " << skipped;

    return buffer_ptr;
}

void JSONImporterTask::insertData (std::shared_ptr<Buffer> buffer)
{
    loginf << "JSONImporterTask: insertData: inserting into database";

    insert_active_ = true;

    if (datasources_to_add_.size())
    {
        loginf << "JSONImporterTask: insertData: inserting " << datasources_to_add_.size() << " data sources";
        db_object_->addDataSources(datasources_to_add_);

        for (auto& src_it : datasources_to_add_)
            datasources_existing_ [src_it.first] = src_it.second;
        datasources_to_add_.clear();
    }

    connect (db_object_, &DBObject::insertDoneSignal, this, &JSONImporterTask::insertDoneSlot,
             Qt::UniqueConnection);
    connect (db_object_, &DBObject::insertProgressSignal, this, &JSONImporterTask::insertProgressSlot,
             Qt::UniqueConnection);

    db_object_->insertData(var_list_, buffer);
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
    if (db_object_)
    {
        if (!db_object_->hasVariable(name_str))
        {
            loginf << "JSONImporterTask: checkAndSetVariable: var " << name_str << " does not exist";
            name_str = "";
            var = nullptr;
        }
        else
        {
            *var = &db_object_->variable(name_str);
            loginf << "JSONImporterTask: checkAndSetVariable: var " << name_str << " set";
            assert (var);
            //assert((*var)->existsInDB());
        }
    }
    else
    {
        loginf << "JSONImporterTask: checkAndSetVariable: dbobject null";
        name_str = "";
        var = nullptr;
    }
}
