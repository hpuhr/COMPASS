﻿#include "dbcontent/dbcontent.h"
#include "asterixpostprocessjob.h"
#include "dbcontent/dbcontentmanager.h"
#include "buffer.h"
#include "compass.h"
#include "projectionmanager.h"
//#include "projection.h"
#include "json.hpp"
#include "dbcontent/variable/metavariable.h"
#include "util/stringconv.h"
#include "util/timeconv.h"
#include "global.h"

#include "boost/date_time/posix_time/posix_time.hpp"

const float tod_24h = 24 * 60 * 60;

using namespace std;
using namespace nlohmann;
using namespace Utils;


std::map<unsigned int, unsigned int> ASTERIXPostprocessJob::obfuscate_m3a_map_;
std::map<unsigned int, unsigned int> ASTERIXPostprocessJob::obfuscate_acad_map_;
std::map<std::string, std::string> ASTERIXPostprocessJob::obfuscate_acid_map_;

bool ASTERIXPostprocessJob::current_date_set_ = false;
boost::posix_time::ptime ASTERIXPostprocessJob::current_date_;
boost::posix_time::ptime ASTERIXPostprocessJob::previous_date_;
bool ASTERIXPostprocessJob::did_recent_time_jump_ = false;
bool ASTERIXPostprocessJob::had_late_time_ = false;

ASTERIXPostprocessJob::ASTERIXPostprocessJob(
    map<string, shared_ptr<Buffer>> buffers,
    boost::posix_time::ptime date,
    bool override_tod_active, float override_tod_offset,
    bool ignore_time_jumps, bool do_timestamp_checks,
    bool filter_tod_active, float filter_tod_min, float filter_tod_max,
    bool filter_position_active,
    float filter_latitude_min, float filter_latitude_max,
    float filter_longitude_min, float filter_longitude_max,
    bool filter_modec_active,
    float filter_modec_min, float filter_modec_max,
    bool do_obfuscate_secondary_info)
    : Job("ASTERIXPostprocessJob"),
    buffers_(std::move(buffers)),
    override_tod_active_(override_tod_active), override_tod_offset_(override_tod_offset),
    ignore_time_jumps_(ignore_time_jumps), do_timestamp_checks_(do_timestamp_checks),
    filter_tod_active_(filter_tod_active), filter_tod_min_(filter_tod_min), filter_tod_max_(filter_tod_max),
    filter_position_active_(filter_position_active),
    filter_latitude_min_(filter_latitude_min), filter_latitude_max_(filter_latitude_max),
    filter_longitude_min_(filter_longitude_min), filter_longitude_max_(filter_longitude_max),
    filter_modec_active_(filter_modec_active),
    filter_modec_min_(filter_modec_min), filter_modec_max_(filter_modec_max),
    do_obfuscate_secondary_info_(do_obfuscate_secondary_info)
{
    if (!current_date_set_) // init if first time
    {
        current_date_ = date;
        previous_date_ = current_date_;

        current_date_set_ = true;
    }

    obfuscate_m3a_map_[512] = 512; // 1000
    obfuscate_m3a_map_[1024] = 1024; // 2000
    obfuscate_m3a_map_[3584] = 3584; // 7000
    obfuscate_m3a_map_[4095] = 4095; // 7777

}

ASTERIXPostprocessJob::ASTERIXPostprocessJob(map<string, shared_ptr<Buffer>> buffers,
                                             boost::posix_time::ptime date)
    : Job("ASTERIXPostprocessJob"),
    buffers_(std::move(buffers))
{
    if (!current_date_set_) // init if first time
    {
        current_date_ = date;
        previous_date_ = current_date_;

        current_date_set_ = true;
    }

    //ignore_time_jumps_ = true; // do if problems with import
}

ASTERIXPostprocessJob::~ASTERIXPostprocessJob() { logdbg << "ASTERIXPostprocessJob: dtor"; }

void ASTERIXPostprocessJob::clearCurrentDate()
{
    current_date_set_ = false;
    current_date_ = {};
    previous_date_ = {};
}

void ASTERIXPostprocessJob::run()
{
    logdbg << "ASTERIXPostprocessJob: run: num buffers " << buffers_.size();

    started_ = true;

    if (override_tod_active_)
        doTodOverride();

    if (do_timestamp_checks_) // out of sync issue during 24h replay
        doFutureTimestampsCheck();

    doTimeStampCalculation();
    doRadarPlotPositionCalculations();
    doADSBPositionPorcessing();
    doGroundSpeedCalculations();

    if (filter_tod_active_ || filter_position_active_ || filter_modec_active_)
        doFilters();

    if (do_obfuscate_secondary_info_)
        doObfuscate();

    done_ = true;
}

void ASTERIXPostprocessJob::doTodOverride()
{
    logdbg << "ASTERIXPostprocessJob: doTodOverride: offset "
           << String::doubleToStringPrecision(override_tod_offset_, 3);

    assert (override_tod_active_);

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    unsigned int buffer_size;

    for (auto& buf_it : buffers_)
    {
        buffer_size = buf_it.second->size();

        assert (dbcont_man.metaVariable(DBContent::meta_var_time_of_day_.name()).existsIn(buf_it.first));

        dbContent::Variable& tod_var = dbcont_man.metaVariable(DBContent::meta_var_time_of_day_.name()).getFor(buf_it.first);

        Property tod_prop {tod_var.name(), tod_var.dataType()};

        assert (buf_it.second->hasProperty(tod_prop));

        NullableVector<float>& tod_vec = buf_it.second->get<float>(tod_var.name());

        for (unsigned int index=0; index < buffer_size; ++index)
        {
            if (!tod_vec.isNull(index))
            {
                float& tod_ref = tod_vec.getRef(index);

                tod_ref += override_tod_offset_;

                // check for out-of-bounds because of midnight-jump
                while (tod_ref < 0.0f)
                    tod_ref += tod_24h;
                while (tod_ref > tod_24h)
                    tod_ref -= tod_24h;

                assert(tod_ref >= 0.0f);
                assert(tod_ref <= tod_24h);
            }
        }
    }
}

const double TMAX_FUTURE_OFFSET = 3*60.0;
const double T24H_OFFSET = 5*60.0;

void ASTERIXPostprocessJob::doFutureTimestampsCheck()
{
    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    unsigned int buffer_size;

    using namespace boost::posix_time;

    auto p_time = microsec_clock::universal_time (); // UTC


    double current_time_utc = p_time.time_of_day().total_milliseconds() / 1000.0;
    double tod_utc_max = (p_time + Time::partialSeconds(TMAX_FUTURE_OFFSET)).time_of_day().total_milliseconds() / 1000.0;

    bool in_vicinity_of_24h_time = current_time_utc <= T24H_OFFSET || current_time_utc >= (tod_24h - T24H_OFFSET);

    loginf << "ASTERIXPostprocessJob: doFutureTimestampsCheck: maximum time is "
           << String::timeStringFromDouble(tod_utc_max) << " 24h vicinity " << in_vicinity_of_24h_time;

    for (auto& buf_it : buffers_)
    {
        buffer_size = buf_it.second->size();

        assert (dbcont_man.metaVariable(DBContent::meta_var_time_of_day_.name()).existsIn(buf_it.first));

        dbContent::Variable& tod_var = dbcont_man.metaVariable(DBContent::meta_var_time_of_day_.name()).getFor(buf_it.first);

        Property tod_prop {tod_var.name(), tod_var.dataType()};

        assert (buf_it.second->hasProperty(tod_prop));

        NullableVector<float>& tod_vec = buf_it.second->get<float>(tod_var.name());

        std::tuple<bool,float,float> min_max_tod = tod_vec.minMaxValues();

        if (get<0>(min_max_tod))
            loginf << "ASTERIXPostprocessJob: doFutureTimestampsCheck: " << buf_it.first
                   << " min tod " << String::timeStringFromDouble(get<1>(min_max_tod))
                   << " max " << String::timeStringFromDouble(get<2>(min_max_tod));

        for (unsigned int index=0; index < buffer_size; ++index)
        {
            if (!tod_vec.isNull(index))
            {
                if (in_vicinity_of_24h_time)
                {
                    // not at end of day and bigger than max
                    if (tod_vec.get(index) < (tod_24h - T24H_OFFSET) && tod_vec.get(index) > tod_utc_max)
                    {
                        logwrn << "ASTERIXPostprocessJob: doFutureTimestampsCheck: vic doing " << buf_it.first
                               << " cutoff tod index " << index
                               << " tod " << String::timeStringFromDouble(tod_vec.get(index));

                        buf_it.second->cutToSize(index);
                        break;
                    }
                }
                else
                {
                    if (tod_vec.get(index) > tod_utc_max)
                    {
                        logwrn << "ASTERIXPostprocessJob: doFutureTimestampsCheck: doing " << buf_it.first
                               << " cutoff tod index " << index
                               << " tod " << String::timeStringFromDouble(tod_vec.get(index));

                        buf_it.second->cutToSize(index);
                        break;
                    }
                }
            }
        }
    }

    // remove empty buffers

    std::map<std::string, std::shared_ptr<Buffer>> tmp_data = buffers_;

    for (auto& buf_it : tmp_data)
        if (!buf_it.second->size())
            buffers_.erase(buf_it.first);
}

void ASTERIXPostprocessJob::doTimeStampCalculation()
{
    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    unsigned int buffer_size;

    for (auto& buf_it : buffers_)
    {
        buffer_size = buf_it.second->size();

        // tod
        assert (dbcont_man.metaVariable(DBContent::meta_var_time_of_day_.name()).existsIn(buf_it.first));
        dbContent::Variable& tod_var = dbcont_man.metaVariable(DBContent::meta_var_time_of_day_.name()).getFor(buf_it.first);

        Property tod_prop {tod_var.name(), tod_var.dataType()};
        assert (buf_it.second->hasProperty(tod_prop));

        NullableVector<float>& tod_vec = buf_it.second->get<float>(tod_var.name());

        // timestamp
        assert (dbcont_man.metaVariable(DBContent::meta_var_timestamp_.name()).existsIn(buf_it.first));
        dbContent::Variable& timestamp_var = dbcont_man.metaVariable(DBContent::meta_var_timestamp_.name()).getFor(buf_it.first);

        Property timestamp_prop {timestamp_var.name(), timestamp_var.dataType()};
        assert (!buf_it.second->hasProperty(timestamp_prop));
        buf_it.second->addProperty(timestamp_prop);

        NullableVector<boost::posix_time::ptime>& timestamp_vec =
            buf_it.second->get<boost::posix_time::ptime>(timestamp_var.name());

        float tod;
        //boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
        boost::posix_time::ptime timestamp;
        bool in_vicinity_of_24h_time = false; // within 5min of 24hrs
        bool outside_vicinity_of_24h_time = false; // outside of 10min to 24hrs

        for (unsigned int index=0; index < buffer_size; ++index)
        {
            if (!tod_vec.isNull(index))
            {
                tod = tod_vec.get(index);

                had_late_time_ |= (tod >= (tod_24h - 300.0)); // within last 5min before midnight

                in_vicinity_of_24h_time = tod <= 300.0 || tod >= (tod_24h - 300.0); // within 10 minutes of midnight

                if (tod > tod_24h/2) // late
                    outside_vicinity_of_24h_time = tod <= tod_24h - 600.0;
                else // early
                    outside_vicinity_of_24h_time = tod >= 600.0 ;

                if (in_vicinity_of_24h_time)
                    logdbg << "ASTERIXPostprocessJob: doTimeStampCalculation: tod " << String::timeStringFromDouble(tod)
                           << " in 24h vicinity, had_late_time_ " << had_late_time_
                           << " did_recent_time_jump " << did_recent_time_jump_;

                if (did_recent_time_jump_ && outside_vicinity_of_24h_time) // clear if set and 10min outside again
                {
                    loginf << "ASTERIXPostprocessJob: doTimeStampCalculation: clearing did_recent_time_jump_";
                    did_recent_time_jump_ = false;
                }

                if (!ignore_time_jumps_ && in_vicinity_of_24h_time) // check if timejump and assign timestamp to correct day
                {
                    // check if timejump (if not yet done)
                    if (!did_recent_time_jump_ && had_late_time_ && tod <= 300.0) // not yet handled timejump
                    {
                        current_date_ += boost::posix_time::seconds((unsigned int) tod_24h);
                        previous_date_ = current_date_ - boost::posix_time::seconds((unsigned int) tod_24h);

                        loginf << "ASTERIXPostprocessJob: doTimeStampCalculation: detected time-jump from"
                               << " previous " << Time::toDateString(previous_date_)
                               << " to current " << Time::toDateString(current_date_);

                        did_recent_time_jump_ = true;
                    }

                    if (tod <= 300.0) // early time of current day
                        timestamp = current_date_ + boost::posix_time::millisec((unsigned int) (tod * 1000));
                    else // late time of previous day
                        timestamp = previous_date_ + boost::posix_time::millisec((unsigned int) (tod * 1000));
                }
                else // normal timestamp
                {
                    timestamp = current_date_ + boost::posix_time::millisec((unsigned int) (tod * 1000));
                }

                if (in_vicinity_of_24h_time)
                    logdbg << "ASTERIXPostprocessJob: doTimeStampCalculation: tod " << String::timeStringFromDouble(tod)
                           << " timestamp " << Time::toString(timestamp);

                if (outside_vicinity_of_24h_time)
                {
                    did_recent_time_jump_ = false;
                    had_late_time_ = false;
                }

                logdbg << "ASTERIXPostprocessJob: doTimeStampCalculation: tod " << String::timeStringFromDouble(tod)
                       << " ts " << Time::toString(timestamp);

                timestamp_vec.set(index, timestamp);
            }
        }
    }
}

void ASTERIXPostprocessJob::doRadarPlotPositionCalculations()
{
    // radar calculations
    ProjectionManager::instance().doRadarPlotPositionCalculations(buffers_);
}

void ASTERIXPostprocessJob::doADSBPositionPorcessing()
{
    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    string dbcontent_name = "CAT021";

    if (!buffers_.count(dbcontent_name))
        return;

    shared_ptr<Buffer> buffer = buffers_.at(dbcontent_name);
    unsigned int buffer_size = buffer->size();

    if (!buffer_size)
        return;

    assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_latitude_));
    assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_longitude_));


    assert (dbcont_man.canGetVariable(dbcontent_name, DBContent::var_cat021_latitude_hr_));
    assert (dbcont_man.canGetVariable(dbcontent_name, DBContent::var_cat021_longitude_hr_));

    dbContent::Variable& lat_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_);
    dbContent::Variable& lon_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_);

    dbContent::Variable& lat_hr_var = dbcont_man.getVariable(dbcontent_name, DBContent::var_cat021_latitude_hr_);
    dbContent::Variable& lon_hr_var = dbcont_man.getVariable(dbcontent_name, DBContent::var_cat021_longitude_hr_);

    assert (lat_var.dataType() == PropertyDataType::DOUBLE);
    assert (lon_var.dataType() == PropertyDataType::DOUBLE);
    assert (lat_hr_var.dataType() == PropertyDataType::DOUBLE);
    assert (lon_hr_var.dataType() == PropertyDataType::DOUBLE);

    string lat_var_name = lat_var.name();
    string lon_var_name = lon_var.name();
    string lat_hr_var_name = lat_hr_var.name();
    string lon_hr_var_name = lon_hr_var.name();

    if (!buffer->has<double>(lat_hr_var_name) || !buffer->has<double>(lon_hr_var_name)) // can not copy
        return;

    if (!buffer->has<double>(lat_var_name))
        buffer->addProperty(lat_var_name, PropertyDataType::DOUBLE); // add if needed

    if (!buffer->has<double>(lon_var_name))
        buffer->addProperty(lon_var_name, PropertyDataType::DOUBLE); // add if needed

    NullableVector<double>& lat_vec = buffer->get<double>(lat_var_name);
    NullableVector<double>& lon_vec = buffer->get<double>(lon_var_name);
    NullableVector<double>& lat_hr_vec = buffer->get<double>(lat_hr_var_name);
    NullableVector<double>& lon_hr_vec = buffer->get<double>(lon_hr_var_name);

    for (unsigned int index=0; index < buffer_size; index++)
    {
        if (!lat_vec.isNull(index) || !lon_vec.isNull(index)) // no need to copy
            continue;

        if (lat_hr_vec.isNull(index) || lon_hr_vec.isNull(index)) // can not copy
            continue;

        lat_vec.set(index, lat_hr_vec.get(index));
        lon_vec.set(index, lon_hr_vec.get(index));
    }
}

void ASTERIXPostprocessJob::doGroundSpeedCalculations()
{
    // general vx/vy to ground speed/track angle conversion

    string dbcontent_name;

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    string vx_var_name;
    string vy_var_name;
    string speed_var_name;
    string track_angle_var_name;

    double speed_ms, track_angle_rad, track_angle_deg;

    for (auto& buf_it : buffers_)
    {
        dbcontent_name = buf_it.first;

        if (!dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_vx_)
            || !dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_vy_))
            continue;

        shared_ptr<Buffer> buffer = buf_it.second;
        unsigned int buffer_size = buffer->size();
        assert(buffer_size);

        assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ground_speed_));
        assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_track_angle_));

        dbContent::Variable& vx_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_vx_);
        dbContent::Variable& vy_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_vy_);
        dbContent::Variable& speed_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ground_speed_);
        dbContent::Variable& track_angle_var =
            dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_angle_);

        vx_var_name = vx_var.name();
        vy_var_name = vy_var.name();
        speed_var_name = speed_var.name();
        track_angle_var_name = track_angle_var.name();

        assert (vx_var.dataType() == PropertyDataType::DOUBLE);
        assert (vy_var.dataType() == PropertyDataType::DOUBLE);
        assert (speed_var.dataType() == PropertyDataType::DOUBLE);
        assert (track_angle_var.dataType() == PropertyDataType::DOUBLE);

        if (!buffer->has<double>(vx_var_name) || !buffer->has<double>(vy_var_name))
            continue; // cant calculate

        if (buffer->has<double>(speed_var_name) && buffer->has<double>(track_angle_var_name)
            && buffer->get<double>(speed_var_name).isNeverNull()
            && buffer->get<double>(track_angle_var_name).isNeverNull())
        {
            logdbg << "ASTERIXPostprocessJob: doGroundSpeedCalculations: "
                   << dbcontent_name << " speed and track angle already set";

            continue; // no need for calculation
        }

        if (!buffer->has<double>(speed_var_name))
            buffer->addProperty(speed_var_name, PropertyDataType::DOUBLE); // add if needed

        if (!buffer->has<double>(track_angle_var_name))
            buffer->addProperty(track_angle_var_name, PropertyDataType::DOUBLE); // add if needed

        NullableVector<double>& vx_vec = buffer->get<double>(vx_var_name);
        NullableVector<double>& vy_vec = buffer->get<double>(vy_var_name);
        NullableVector<double>& speed_vec = buffer->get<double>(speed_var_name);
        NullableVector<double>& track_angle_vec = buffer->get<double>(track_angle_var_name);

        unsigned int cnt = 0;

        for (unsigned int index=0; index < buffer_size; index++)
        {
            if (vx_vec.isNull(index) || vy_vec.isNull(index)) // can not calculate
                continue;

            if (!speed_vec.isNull(index) && !track_angle_vec.isNull(index)) // already set
                continue;

            speed_ms = sqrt(pow(vx_vec.get(index), 2)+pow(vy_vec.get(index), 2)) ; // for 1s
            track_angle_rad = atan2(vx_vec.get(index), vy_vec.get(index));

            track_angle_deg = track_angle_rad * RAD2DEG;

            if (track_angle_deg < 0)
                track_angle_deg += 360.0;

            speed_vec.set(index, speed_ms * M_S2KNOTS);
            track_angle_vec.set(index, track_angle_deg);

            ++cnt;
        }

        logdbg << "ASTERIXPostprocessJob: doGroundSpeedCalculations: "
               << dbcontent_name << " speed and track angle calc " << cnt << " / " << buffer_size;
    }


    // cat021 sgv conversion

    dbcontent_name = "CAT021";

    unsigned int spd_already_set {0}, sgv_spd_no_val {0}, sgv_hgt_no_value {0},
        sgv_is_heading {0}, sgv_is_magnetic {0}, sgv_usable {0};

    logdbg << "ASTERIXPostprocessJob: doGroundSpeedCalculations: got ads-b "
           << (buffers_.count(dbcontent_name) && buffers_.at(dbcontent_name)->size());

    if (buffers_.count(dbcontent_name) && buffers_.at(dbcontent_name)->size())
    {
        auto& buffer = buffers_.at(dbcontent_name);

        unsigned int buffer_size = buffer->size();
        assert(buffer_size);

        logdbg << "ASTERIXPostprocessJob: doGroundSpeedCalculations: got ads-b sgv gss "
               << buffer->has<float>(DBContent::var_cat021_sgv_gss_.name())
               << " hgt " << buffer->has<double>(DBContent::var_cat021_sgv_hgt_.name())
               << " htt " << buffer->has<bool>(DBContent::var_cat021_sgv_htt_.name())
               << " hrd " << buffer->has<bool>(DBContent::var_cat021_sgv_hrd_.name());

        if (!buffer->has<float>(DBContent::var_cat021_sgv_gss_.name())
            || !buffer->has<double>(DBContent::var_cat021_sgv_hgt_.name())
            || !buffer->has<bool>(DBContent::var_cat021_sgv_htt_.name())
            || !buffer->has<bool>(DBContent::var_cat021_sgv_hrd_.name()))
            return;

        dbContent::Variable& speed_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ground_speed_);
        dbContent::Variable& track_angle_var =
            dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_angle_);

        speed_var_name = speed_var.name();
        track_angle_var_name = track_angle_var.name();

        assert (speed_var.dataType() == PropertyDataType::DOUBLE);
        assert (track_angle_var.dataType() == PropertyDataType::DOUBLE);

        NullableVector<double>& speed_vec = buffer->get<double>(speed_var_name);
        NullableVector<double>& track_angle_vec = buffer->get<double>(track_angle_var_name);

        NullableVector<float>& sgv_gss_vec = buffer->get<float>(DBContent::var_cat021_sgv_gss_.name());
        NullableVector<double>& sgv_hgt_vec = buffer->get<double>(DBContent::var_cat021_sgv_hgt_.name());
        NullableVector<bool>& sgv_htt_vec = buffer->get<bool>(DBContent::var_cat021_sgv_htt_.name());
        NullableVector<bool>& sgv_hrd_vec = buffer->get<bool>(DBContent::var_cat021_sgv_hrd_.name());

        for (unsigned int index=0; index < buffer_size; index++)
        {
            if (!speed_vec.isNull(index) && !track_angle_vec.isNull(index)) // already set
            {
                spd_already_set++;
                continue;
            }

            if (sgv_gss_vec.isNull(index)) // speed not set
            {
                ++sgv_spd_no_val;
                continue;
            }

            speed_vec.set(index, sgv_gss_vec.get(index));

            if (sgv_hgt_vec.isNull(index) // heading/track not set or cannot distingush
                || sgv_htt_vec.isNull(index) || sgv_hrd_vec.isNull(index))
            {
                ++sgv_hgt_no_value;
                continue;
            }

            if (sgv_htt_vec.get(index) == 0)
            {
                ++sgv_is_heading;
                continue;
            }

            if (sgv_hrd_vec.get(index) == 1)
            {
                ++sgv_is_magnetic;
                continue;
            }

            track_angle_vec.set(index, sgv_hgt_vec.get(index));

            sgv_usable++; // there
        }

        logdbg << "ASTERIXPostprocessJob: doGroundSpeedCalculations: CAT021 spd_already_set " << spd_already_set
               << " sgv_spd_no_val " << sgv_spd_no_val << " sgv_hgt_no_value " << sgv_hgt_no_value
               << " sgv_is_heading " << sgv_is_heading << " sgv_is_magnetic " << sgv_is_magnetic
               << " sgv_usable " << sgv_usable;
    }
}

void ASTERIXPostprocessJob::doFilters()
{
    string dbcontent_name;

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    // do time based filtering first
    if (filter_tod_active_)
    {
        string tod_var_name;

        for (auto& buf_it : buffers_)
        {
            dbcontent_name = buf_it.first;

            shared_ptr<Buffer> buffer = buf_it.second;
            unsigned int buffer_size = buffer->size();

            if(!buffer_size)
                continue;

            assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_time_of_day_));

            dbContent::Variable& tod_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_time_of_day_);

            tod_var_name = tod_var.name();

            assert (buffer->has<float>(tod_var_name));

            NullableVector<float>& tod_vec = buffer->get<float>(tod_var_name);

            std::vector<size_t> to_be_removed;

            for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
            {
                if (filter_tod_active_ && !tod_vec.isNull(cnt)
                    && (tod_vec.get(cnt) < filter_tod_min_ || tod_vec.get(cnt) > filter_tod_max_))
                {
                    to_be_removed.push_back(cnt);
                    continue;
                }
            }

            buffer->removeIndexes(to_be_removed);
        }
    }


    // others
    if (filter_modec_active_ || filter_modec_active_)
    {
        string lat_var_name;
        string lon_var_name;
        string mc_var_name;

        for (auto& buf_it : buffers_)
        {
            dbcontent_name = buf_it.first;

            if (!dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_latitude_)
                || !dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_longitude_)
                || !dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_mc_))
                continue;

            shared_ptr<Buffer> buffer = buf_it.second;
            unsigned int buffer_size = buffer->size();

            if(!buffer_size)
                continue;

            assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_latitude_));
            assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_longitude_));
            assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_mc_));

            dbContent::Variable& lat_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_);
            dbContent::Variable& lon_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_);
            dbContent::Variable& mc_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_);

            lat_var_name = lat_var.name();
            lon_var_name = lon_var.name();
            mc_var_name = mc_var.name();

            assert (buffer->has<double>(lat_var_name));
            assert (buffer->has<double>(lon_var_name));
            assert (buffer->has<float>(mc_var_name));

            NullableVector<double>& lat_vec = buffer->get<double>(lat_var_name);
            NullableVector<double>& lon_vec = buffer->get<double>(lon_var_name);
            NullableVector<float>& mc_vec = buffer->get<float>(mc_var_name);

            NullableVector<float>* mc_vec2 {nullptr};

            if (dbcontent_name == "CAT062")
            {
                assert (dbcont_man.canGetVariable(dbcontent_name, DBContent::var_cat062_fl_measured_));
                dbContent::Variable& mc_var2 = dbcont_man.getVariable(dbcontent_name, DBContent::var_cat062_fl_measured_);

                if (buffer->has<float>(mc_var2.name()))
                    mc_vec2 = &buffer->get<float>(mc_var2.name());
            }

            std::vector<size_t> to_be_removed;

            for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
            {
                if (filter_position_active_ && !lat_vec.isNull(cnt) && !lon_vec.isNull(cnt)
                    && (lat_vec.get(cnt) < filter_latitude_min_ || lat_vec.get(cnt) > filter_latitude_max_
                        || lon_vec.get(cnt) < filter_longitude_min_ || lon_vec.get(cnt) > filter_longitude_max_))
                {
                    to_be_removed.push_back(cnt);
                    continue;
                }

                if (filter_modec_active_)
                {
                    if (!mc_vec.isNull(cnt)
                        && (mc_vec.get(cnt) < filter_modec_min_ || mc_vec.get(cnt) > filter_modec_max_))
                    {
                        to_be_removed.push_back(cnt);
                        continue;
                    }
                    else if (mc_vec2 && !mc_vec2->isNull(cnt)
                             && (mc_vec2->get(cnt) < filter_modec_min_ || mc_vec2->get(cnt) > filter_modec_max_))
                    {
                        to_be_removed.push_back(cnt);
                        continue;
                    }
                }
            }

            buffer->removeIndexes(to_be_removed);
        }
    }

    // delete empty ones

    for (auto it = buffers_.cbegin(); it != buffers_.cend() /* not hoisted */; /* no increment */)
    {
        if (!it->second->size())
            buffers_.erase(it++);    // or "it = m.erase(it)" since C++11
        else
            ++it;
    }
}

void ASTERIXPostprocessJob::doObfuscate()
{

    assert (do_obfuscate_secondary_info_);

    string dbcontent_name;

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    // filter / change mode 3/a codes
    {
        string var_name;

        for (auto& buf_it : buffers_)
        {
            dbcontent_name = buf_it.first;

            shared_ptr<Buffer> buffer = buf_it.second;
            unsigned int buffer_size = buffer->size();

            if(!buffer_size)
                continue;

            if (!dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_m3a_))
                continue;

            dbContent::Variable& var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_);

            var_name = var.name();

            assert (buffer->has<unsigned int>(var_name));

            NullableVector<unsigned int>& var_vec = buffer->get<unsigned int>(var_name);

            std::vector<size_t> to_be_removed;

            for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
            {
                if (var_vec.isNull(cnt))
                    continue;

                if ((var_vec.get(cnt) >= 832 && var_vec.get(cnt) <= 895) // 1500 - 1577
                    || (var_vec.get(cnt) >= 2560 && var_vec.get(cnt) <= 3071)) // 5000 - 5777
                {
                    to_be_removed.push_back(cnt);
                    continue;
                }
                else // obfuscate
                    obfuscateM3A(var_vec.getRef(cnt));
            }

            buffer->removeIndexes(to_be_removed);
        }
    }

    // change acads
    {
        string var_name;

        for (auto& buf_it : buffers_)
        {
            dbcontent_name = buf_it.first;

            shared_ptr<Buffer> buffer = buf_it.second;
            unsigned int buffer_size = buffer->size();

            if(!buffer_size)
                continue;

            if (!dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_acad_))
                continue;

            dbContent::Variable& var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_acad_);

            var_name = var.name();

            assert (buffer->has<unsigned int>(var_name));

            NullableVector<unsigned int>& var_vec = buffer->get<unsigned int>(var_name);

            for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
            {
                if (var_vec.isNull(cnt))
                    continue;

                // obfuscate
                obfuscateACAD(var_vec.getRef(cnt));
            }
        }
    }

    // change acids
    {
        string var_name;

        for (auto& buf_it : buffers_)
        {
            dbcontent_name = buf_it.first;

            shared_ptr<Buffer> buffer = buf_it.second;
            unsigned int buffer_size = buffer->size();

            if(!buffer_size)
                continue;

            if (!dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_acid_))
                continue;

            dbContent::Variable& var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_acid_);

            var_name = var.name();

            assert (buffer->has<string>(var_name));

            NullableVector<string>& var_vec = buffer->get<string>(var_name);

            for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
            {
                if (var_vec.isNull(cnt))
                    continue;

                // obfuscate
                obfuscateACID(var_vec.getRef(cnt));
            }
        }
    }
}

void ASTERIXPostprocessJob::obfuscateM3A (unsigned int& value)
{
    if (!obfuscate_m3a_map_.count(value))
    {
        // generate new value

        unsigned int obfuscated_val = obfuscate_m3a_map_.size();
        while (std::find_if(obfuscate_m3a_map_.begin(), obfuscate_m3a_map_.end(),
                            [obfuscated_val](const auto& mo) {return mo.second == obfuscated_val; })
               != obfuscate_m3a_map_.end())
        {
            ++obfuscated_val;
        }

        assert (obfuscated_val <= 4095);
        obfuscate_m3a_map_[value] = obfuscated_val;
    }

    value = obfuscate_m3a_map_.at(value);
}

void ASTERIXPostprocessJob::obfuscateACAD (unsigned int& value)
{
    if (!obfuscate_acad_map_.count(value))
    {
        // generate new value

        unsigned int obfuscated_val = obfuscate_acad_map_.size()+1;

        while (std::find_if(obfuscate_acad_map_.begin(), obfuscate_acad_map_.end(),
                            [obfuscated_val](const auto& mo) {return mo.second == obfuscated_val; })
               != obfuscate_acad_map_.end())
        {
            ++obfuscated_val;
        }

        obfuscate_acad_map_[value] = obfuscated_val;
    }

    value = obfuscate_acad_map_.at(value);
}

// List of Starfleet ship names (prefixes)
static const std::vector<std::string> ship_names = {
    "ENTE",  // Enterprise
    "VOYA",  // Voyager
    "DEFI",  // Defiant
    "DISC",  // Discovery
    "RELI",  // Reliant
    "EXCE",  // Excelsior
    "EQUI",  // Equinox
    "INTR",  // Intrepid
    "TITA",  // Titan
    "ODYS",  // Odyssey
    "CONS",  // Constellation
    "STAR",  // Stargazer
    "GRIS",  // Grissom
    "SARA",  // Saratoga
    "COCH",  // Cochrane
    "KIRK",  // Kirk (NX-01 refit named after him)
    "PICA",  // Picard
    "JANE",  // Janeway
    "SULU",  // Sulu
    "SHEN",  // Shenzhou
    "YORK",  // Yorktown
    "BURA",  // Buran
    "THUN",  // Thunderchild
    "FRAN",  // Franklin
    "VENG",  // Vengeance
    "SHRA",  // Shran
    "LEXI",  // Lexington
    "POTE",  // Potemkin
    "ENDE",  // Endeavour
    "COLU",  // Columbia
    "KELV",  // Kelvin
    "ANTA",  // Antares
    "DAED",  // Daedalus
    "ARES",  // Ares
    "NEBU",  // Nebula
    "AKIR",  // Akira
    "PROM",  // Prometheus
    "LUNA",  // Luna
    "NORW",  // Norway
    "STEA",  // Steamrunner
    "EXET",  // Exeter
    "HOOD",  // Hood
    "MIRA",  // Miranda
    "SOVE",  // Sovereign
    "AVEN",  // Avenger
    "EAGL",  // Eagle
    "FARR",  // Farragut
    "BOZE",  // Bozeman
    "CAIR",  // Cairo
    "EXCA",  // Excalibur
    "GALX",  // Galaxy
    "HATH",  // Hathaway
    "MAGL",  // Magellan
    "YAMA",  // Yamaguchi
    "ADVE",  // Adventure
    "BLAC",  // Blackwell
    "CHAL",  // Challenger
    "CHAR",  // Charleston
    "CHER",  // Cherokee
    "CONC",  // Concord
    "COPE",  // Copernicus
    "DERB",  // Derbyshire
    "DRAK",  // Drake
    "EDIS",  // Edison
    "EDMU",  // Edmund
    "GAGA",  // Gagarin
    "GRIF",  // Griffin
    "HANS",  // Hansen
    "MARY",  // Maryland
    "OBER",  // Oberth
    "OLIV",  // Oliver
    "ORIO",  // Orion
    "PEGA",  // Pegasus
    "SAGA",  // Sagan
    "ZHEN",  // Zheng He
    "NIAG",  // Niagara
    "NIMI",  // Nimitz
    "GETT",  // Gettysburg
    "CHAN",  // Chang
    "KONG",  // Kongo
    "TURK",  // Turkey
    "KURA",  // Kurak
    "SARE",  // Sarek
    "TPAU",  // T'Pau
    "SOVA",  // Soval
    "SURA",  // Surak
    "ARCH",  // Archer
    "DALL",  // Dallas
    "LOND",  // London
    "PARI",  // Paris
    "BERL",  // Berlin
    "MINS",  // Minsk
    "ROCI",   // Rocinante
    "DONN",   // Donnager
    "NAVA",   // Navoo
    "TYNA",   // Tynan
    "AGAT",   // Agatha King
    "BEHE",   // Behemoth
    "CONT",   // Contorta (Pinus Contorta)
    "PELL",   // Pella
    "SCOP",   // Scopuli
    "BARB",   // Barbapiccola
    "RAWE",   // Raweside
    "HYGE",   // Hygiea
    "ARBO"    // Arboghast
};

// Function to obfuscate value string
//std::string obfuscate_input(const std::string& value)
void ASTERIXPostprocessJob::obfuscateACID (std::string& value)
{

    // Static map to store input-output pairings
    //static std::map<std::string, std::string> obfuscate_acid_map_;

    // Check if the input has already been mapped
    auto it = obfuscate_acid_map_.find(value);
    if (it != obfuscate_acid_map_.end()) {
        // Return the previously stored output
        value = it->second;
        return;
    }

    // Ensure input length is between 0 and 8 characters
    std::string trimmed_input = value.substr(0, 8);

    // Maximum output length
    const size_t max_output_length = 8;

    // Compute a consistent hash value from the input string
    // Using a simple custom hash function for consistency across runs
    size_t hash_value = 0;
    for (char c : trimmed_input) {
        hash_value = hash_value * 31 + static_cast<size_t>(c);
    }

    // Select a ship name based on the hash value
    size_t ship_index = hash_value % ship_names.size();
    std::string ship_prefix = ship_names[ship_index];

    // Determine the maximum length for the count number
    size_t max_count_length = max_output_length - ship_prefix.length();
    if (max_count_length == 0) {
        // If there's no space for count number, truncate the ship prefix
        ship_prefix = ship_prefix.substr(0, max_output_length - 1);
        max_count_length = 1;
    }

    // Compute a count number to ensure uniqueness
    // Use a static counter for each ship prefix
    static std::map<std::string, size_t> ship_counters;
    size_t count_number = ship_counters[ship_prefix]++;
    count_number = count_number % static_cast<size_t>(std::pow(10, max_count_length));

    // Format the count number with leading zeros if necessary
    std::string count_str = std::to_string(count_number);
    if (count_str.length() < max_count_length) {
        count_str = std::string(max_count_length - count_str.length(), '0') + count_str;
    }

    // Combine ship prefix and count number
    std::string obfuscated_val = ship_prefix + count_str;

    // Ensure the output does not exceed the maximum length
    if (obfuscated_val.length() > max_output_length) {
        obfuscated_val = obfuscated_val.substr(0, max_output_length);
    }

    // Store the new input-output mapping
    obfuscate_acid_map_[value] = obfuscated_val;

    value = obfuscated_val;
}

