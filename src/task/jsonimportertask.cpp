#include "jsonimportertask.h"
#include "jsonimportertaskwidget.h"
#include "taskmanager.h"
#include "atsdb.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "sqlitefile.h"
#include "files.h"
#include "stringconv.h"

#include <stdexcept>
#include <fstream>

#include <QDateTime>

#include <jsoncpp/json/json.h>

using namespace Utils;

JSONImporterTask::JSONImporterTask(const std::string& class_id, const std::string& instance_id,
                                   TaskManager* task_manager)
    : Configurable (class_id, instance_id, task_manager)
{
    registerParameter("last_filename", &last_filename_, "");
    registerParameter("db_object_str", &db_object_str_, "");

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

bool JSONImporterTask::importFile(const std::string& filename, bool test)
{
    loginf << "JSONImporterTask: importFile: filename " << filename << " test " << test;

    if (!Files::fileExists(filename))
    {
        logerr << "JSONImporterTask: importFile: filename " << filename << " does not exist";
        return false;
    }

    std::ifstream ifs(filename);
    Json::Reader reader;
    Json::Value obj;
    reader.parse(ifs, obj); // reader can also read strings

    unsigned int rec_num = 0;
    unsigned int receiver;
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

    unsigned int skipped = 0;
    unsigned int inserted = 0;

    for (Json::Value::const_iterator it = obj.begin(); it != obj.end(); ++it)
    {
        loginf << it.key().asString(); // << ':' << it->asInt() << '\n';

        if (it.key().asString() == "acList")
        {
            Json::Value ac_list = obj["acList"];
            //Json::Value& ac_list = it.v;
            for (Json::Value::const_iterator tr_it = ac_list.begin(); tr_it != ac_list.end(); ++tr_it)
            {
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
                assert (!(*tr_it)["Rcvr"].isNull());
                receiver = (*tr_it)["Rcvr"].asUInt();

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

                assert (!(*tr_it)["Icao"].isNull());
                target_address = String::intFromHexString((*tr_it)["Icao"].asString());

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

                //    Long (float) – The aircraft’s longitude over the ground.
                longitude_valid =!(*tr_it)["Long"].isNull();
                if (longitude_valid)
                    longitude_deg = (*tr_it)["Long"].asFloat();

                //    PosTime (epoch milliseconds) – The time (at UTC in JavaScript ticks, UNIX epoch format in
                // milliseconds) that the position was last reported by the aircraft. This field is the time at which
                // the aircraft was at the lat/long/altitude reported above. https://www.epochconverter.com/ may be
                // helpful.
                time_valid = !(*tr_it)["PosTime"].isNull();
                if (time_valid)
                {
                    epoch_ms = (*tr_it)["PosTime"].asLargestUInt();
                    date_time.setMSecsSinceEpoch(epoch_ms);
                }

                //    Mlat (boolean) – True if the latitude and longitude appear to have been calculated by an MLAT
                // (multilateration) server and were not transmitted by the aircraft. Multilateration is based on the
                // time difference that specific receivers detect the signal and a mathematical calculation.  It is
                // significantly less accurate than ADS-B, which is based on GPS, and more likely to result in jagged
                // aircraft tracks. Aircraft that have Mode S (and have not upgraded to ADS-B) can sometimes be tracked
                // via multilateration.  It requires 3-4 ground stations in different locations to be receiving the
                // aircraft signal simultaneously in order to allow the calculation.

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

                if (latitude_valid && longitude_valid && time_valid)
                {
                    loginf << "\t rn " << rec_num
                           << " rc " << receiver
                           << " ta " << target_address
                           << " cs " << (callsign_valid ? callsign : "NULL")
                           << " alt_baro " << (altitude_baro_valid ? std::to_string(altitude_baro_ft) : "NULL")
                           << " alt_geo " << (altitude_geo_valid ? std::to_string(altitude_geo_ft) : "NULL")
                           << " lat " << (latitude_valid ? std::to_string(latitude_deg) : "NULL")
                           << " lon " << (longitude_valid ? std::to_string(longitude_deg) : "NULL")
                           << " dt " << (time_valid ? date_time.toString("yyyy.MM.dd hh:mm:ss.zzz").toStdString()
                                                    : "NULL");
                    inserted++;
                }
                else
                    skipped++;

                rec_num++;
            }
        }
    }
    if (rec_num != 0)
        loginf << "JSONImporterTask: importFile: all " << rec_num
               << " inserted " << inserted << " (" << String::percentToString(100.0 * inserted/rec_num) << "%)"
               << " skipped " << skipped<< " (" << String::percentToString(100.0 * skipped/rec_num) << "%)";
    else
        loginf << "JSONImporterTask: importFile: all "<< rec_num << " inserted " << inserted << " skipped " << skipped;

    //    cout << "Book: " << obj["book"].asString() << endl;
    //    cout << "Year: " << obj["year"].asUInt() << endl;
    //    const Json::Value& characters = obj["characters"]; // array of characters
    //    for (int i = 0; i < characters.size(); i++)
    //    {
    //        cout << "    name: " << characters[i]["name"].asString();
    //        cout << " chapter: " << characters[i]["chapter"].asUInt();
    //    }

    return true;
}
