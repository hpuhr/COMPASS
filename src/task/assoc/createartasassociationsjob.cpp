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

#include "createartasassociationsjob.h"
#include "compass.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "buffer.h"
#include "createartasassociationstask.h"
#include "dbinterface.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/metavariable.h"
#include "stringconv.h"
#include "util/timeconv.h"

#include <QThread>
#include <QCoreApplication>

#include <cmath>
#include <algorithm>

using namespace Utils;
using namespace std;
using namespace nlohmann;

CreateARTASAssociationsJob::CreateARTASAssociationsJob(
    CreateARTASAssociationsTask& task, DBInterface& db_interface,
    map<string, shared_ptr<Buffer>> buffers)
    : Job("CreateARTASAssociationsJob"), task_(task), db_interface_(db_interface), buffers_(buffers)
{
    misses_acceptable_time_ = Time::partialSeconds(task_.missesAcceptableTime());

    association_time_past_ = Time::partialSeconds(task_.associationTimePast());
    association_time_future_ = Time::partialSeconds(task_.associationTimeFuture());  // will be negative time diff

    associations_dubious_distant_time_ = Time::partialSeconds(task_.associationsDubiousDistantTime());
    association_dubious_close_time_past_ = Time::partialSeconds(task_.associationDubiousCloseTimePast());
    association_dubious_close_time_future_ =
        Time::partialSeconds(task_.associationDubiousCloseTimeFuture());  // will be negative time diff
}

CreateARTASAssociationsJob::~CreateARTASAssociationsJob() {}

void CreateARTASAssociationsJob::run()
{
    logdbg << "CreateARTASAssociationsJob: run: start";

    started_ = true;

    boost::posix_time::ptime start_time;
    boost::posix_time::ptime stop_time;

    start_time = boost::posix_time::microsec_clock::local_time();

    // clear previous associations

    loginf << "CreateARTASAssociationsJob: run: clearing associations";

    emit statusSignal("Clearing Previous ARTAS Associations");
    removePreviousAssociations();

    // create utns
    emit statusSignal("Creating UTNs");
    createUTNS();

    // create associations for artas tracks
    emit statusSignal("Creating ARTAS Associations");
    createARTASAssociations();

    // create associations for sensors
    createSensorAssociations();

    if (missing_hashes_cnt_ || dubious_associations_cnt_)
    {
        stringstream ss;
        ss << "There are " << missing_hashes_cnt_ << " missing hashes and "
           << dubious_associations_cnt_
           << " dubious associations.\nDo you want to still save the associations?";
        emit saveAssociationsQuestionSignal(ss.str().c_str());

        while (!save_question_answered_)
            QThread::msleep(1);

        if (!save_question_answer_)  // nope
        {
            stop_time = boost::posix_time::microsec_clock::local_time();

            double load_time;
            boost::posix_time::time_duration diff = stop_time - start_time;
            load_time = diff.total_milliseconds() / 1000.0;

            loginf << "CreateARTASAssociationsJob: run: done ("
                   << String::doubleToStringPrecision(load_time, 2) << " s).";
            done_ = true;

            return;
        }
        // else simply continue
    }

    // save associations
    emit statusSignal("Saving Associations");

    saveAssociations();

//    object_man.setAssociationsDataSource(tracker_dbcontent_name_, task_.currentDataSourceName());

    stop_time = boost::posix_time::microsec_clock::local_time();

    double load_time;
    boost::posix_time::time_duration diff = stop_time - start_time;
    load_time = diff.total_milliseconds() / 1000.0;

    loginf << "CreateARTASAssociationsJob: run: done ("
           << String::doubleToStringPrecision(load_time, 2) << " s).";

    done_ = true;
}

size_t CreateARTASAssociationsJob::missingHashes() const { return missing_hashes_cnt_; }

size_t CreateARTASAssociationsJob::foundHashes() const { return found_hashes_cnt_; }

size_t CreateARTASAssociationsJob::foundHashDuplicates() const
{
    return found_hash_duplicates_cnt_;
}

size_t CreateARTASAssociationsJob::dubiousAssociations() const { return dubious_associations_cnt_; }

size_t CreateARTASAssociationsJob::missingHashesAtBeginning() const
{
    return acceptable_missing_hashes_cnt_;
}

void CreateARTASAssociationsJob::createUTNS()
{
    loginf << "CreateARTASAssociationsJob: createUTNS";

    if (!buffers_.count(tracker_dbcontent_name_))
    {
        logwrn << "CreateARTASAssociationsJob: createUTNS: no tracker data found";
        return;
    }

    shared_ptr<Buffer> buffer = buffers_.at(tracker_dbcontent_name_);
    size_t buffer_size = buffer->size();

    assert(buffer->has<unsigned int>(task_.trackerTrackNumVar()->name()));
    assert(buffer->has<bool>(task_.trackerTrackBeginVar()->name()));
    assert(buffer->has<bool>(task_.trackerTrackEndVar()->name()));
    assert(buffer->has<bool>(task_.trackerCoastingVar()->name()));
    assert(buffer->has<string>(task_.trackerTRIsVar()->name()));

    assert(buffer->has<unsigned int>(task_.keyVar()->getNameFor(tracker_dbcontent_name_)));
    assert(buffer->has<boost::posix_time::ptime>(task_.timestampVar()->getNameFor(tracker_dbcontent_name_)));

    NullableVector<unsigned int> track_nums = buffer->get<unsigned int>(task_.trackerTrackNumVar()->name());
    NullableVector<bool> track_begins = buffer->get<bool>(task_.trackerTrackBeginVar()->name());
    NullableVector<bool> track_ends = buffer->get<bool>(task_.trackerTrackEndVar()->name());
    NullableVector<bool> track_coastings = buffer->get<bool>(task_.trackerCoastingVar()->name());
    NullableVector<string> tri_hashes = buffer->get<string>(task_.trackerTRIsVar()->name());

    NullableVector<unsigned int> rec_nums = buffer->get<unsigned int>(
                task_.keyVar()->getNameFor(tracker_dbcontent_name_));
    NullableVector<boost::posix_time::ptime> ts_vec = buffer->get<boost::posix_time::ptime>(
                task_.timestampVar()->getNameFor(tracker_dbcontent_name_));

    map<int, UniqueARTASTrack> current_tracks;  // utn -> unique track
    map<int, int> current_track_mappings;       // track_num -> utn

    int utn_cnt{0};

    bool tri_set;
    string tri;
    int track_num;
    bool track_begin_set;
    bool track_begin;
    bool track_end_set;
    bool track_end;
    bool track_coasting_set;
    bool track_coasting;

    bool ignore_track_end_associations = task_.ignoreTrackEndAssociations();
    bool ignore_track_coasting_associations = task_.ignoreTrackCoastingAssociations();

    int rec_num;
    boost::posix_time::ptime timestamp;

    int utn;
    // bool new_track_created;
    bool finish_previous_track;
    bool ignore_update;

    boost::posix_time::time_duration track_end_time = Time::partialSeconds(task_.endTrackTime());
    // time-delta after which begin a new track

    for (size_t cnt = 0; cnt < buffer_size; ++cnt)
    {
        // new_track_created = false;
        finish_previous_track = false;

        tri_set = !tri_hashes.isNull(cnt);
        if (tri_set)
            tri = tri_hashes.get(cnt);
        else
            tri = "";

        assert(!track_nums.isNull(cnt));
        track_num = track_nums.get(cnt);

        track_begin_set = !track_begins.isNull(cnt);
        if (track_begin_set)
            track_begin = track_begins.get(cnt);
        else
            track_begin = false;

        track_end_set = !track_ends.isNull(cnt);
        if (track_end_set)
            track_end = track_ends.get(cnt);
        else
            track_end = false;

        track_coasting_set = !track_coastings.isNull(cnt);
        if (track_coasting_set)
            track_coasting = track_coastings.get(cnt);
        else
            track_coasting = false;

        assert(!rec_nums.isNull(cnt));
        rec_num = rec_nums.get(cnt);

        assert(!ts_vec.isNull(cnt));
        timestamp = ts_vec.get(cnt);

        // was loaded as sorted in time
        if (cnt == 0)  // store first time
            first_track_ts_ = timestamp;

        last_track_ts_ = timestamp;  // store last time

        if (track_begin_set && track_begin && current_track_mappings.count(track_num))
        {
            logdbg << "CreateARTASAssociationsJob: createUTNS: finalizing track utn " << utn
                   << " track end is set";

            finish_previous_track = true;
        }

        // check if existed but last update was more than 10m ago
        if (!finish_previous_track && current_track_mappings.count(track_num) &&
            timestamp - current_tracks.at(current_track_mappings.at(track_num)).last_ts_ >
                track_end_time)
        {
            logdbg << "CreateARTASAssociationsJob: createUTNS: finalizing track utn "
                   << current_track_mappings.at(track_num) << " since tod difference "
                   << timestamp - current_tracks.at(current_track_mappings.at(track_num)).last_ts_;

            finish_previous_track = true;
        }

        if (finish_previous_track)
        {
            // finalize old track
            utn = current_track_mappings.at(track_num);
            assert(!finished_tracks_.count(utn));
            finished_tracks_[utn] = current_tracks.at(utn);
            current_tracks.erase(utn);
            current_track_mappings.erase(track_num);
        }

        if (!current_track_mappings.count(track_num))  // new track where none existed
        {
            logdbg << "CreateARTASAssociationsJob: createUTNS: new track utn " << utn_cnt
                   << " track num " << track_num << " tod " << Time::toString(timestamp)
                   << " begin " << (track_begin_set ? to_string(track_begin) : " not set");

            utn = utn_cnt;
            ++utn_cnt;

            current_track_mappings[track_num] = utn;

            current_tracks[utn].utn = utn;
            current_tracks[utn].track_num = track_num;
            current_tracks[utn].first_ts_ = timestamp;

            // new_track_created = true;
        }
        else
            utn = current_track_mappings.at(track_num);

        assert(current_tracks.count(utn));
        UniqueARTASTrack& unique_track = current_tracks.at(utn);
        unique_track.last_ts_ = timestamp;

        // add tris if not to be ignored
        ignore_update =
            ((track_end_set && track_end && ignore_track_end_associations) ||
             (track_coasting_set && track_coasting && ignore_track_coasting_associations));

        if (ignore_update)
        {
            logdbg << "CreateARTASAssociationsJob: createUTNS: ignoring rec num " << rec_num;
            // add empty tri so that at least track update is associated
            unique_track.rec_nums_tris_[rec_num] = make_pair("", timestamp);
            ++ignored_track_updates_cnt_;
        }
        else
            unique_track.rec_nums_tris_[rec_num] = make_pair(tri, timestamp);

        if (track_end_set && track_end)
        {
            logdbg << "CreateARTASAssociationsJob: createUTNS: finalizing track utn " << utn
                   << " since track end is set";

            // finalize old track
            assert(!finished_tracks_.count(utn));
            finished_tracks_[utn] = current_tracks.at(utn);
            current_tracks.erase(utn);
            current_track_mappings.erase(track_num);
        }
    }

    // finish all and clean up
    for (auto& ut_it : current_tracks)
    {
        assert(!finished_tracks_.count(ut_it.first));
        finished_tracks_[ut_it.first] = ut_it.second;
    }

    current_tracks.clear();
    current_track_mappings.clear();

    loginf << "CreateARTASAssociationsJob: createUTNS: found " << finished_tracks_.size()
           << " finished tracks ";
}

void CreateARTASAssociationsJob::createARTASAssociations()
{
    loginf << "CreateARTASAssociationsJob: createARTASAssociations";

    // set utns in tracker rec_nums

    for (auto& ut_it : finished_tracks_)                    // utn -> UAT
        for (auto& assoc_it : ut_it.second.rec_nums_tris_)  // rec_num -> tri
            associations_[tracker_dbcontent_name_][assoc_it.first] =
                    std::make_tuple(ut_it.first, std::vector<std::pair<std::string, unsigned int>>());
            //tracker_object.addAssociation(assoc_it.first, ut_it.first, true, assoc_it.first);
}

void CreateARTASAssociationsJob::saveAssociations()
{
    loginf << "CreateARTASAssociationsJob: saveAssociations";

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    // write association info to buffers

    unsigned int rec_num;

    for (auto& cont_assoc_it : associations_) // dbcontent -> rec_nums
    {
        unsigned int num_associated {0};
        unsigned int num_not_associated {0};

        string dbcontent_name = cont_assoc_it.first;
        std::map<unsigned int,
                std::tuple<unsigned int, std::vector<std::pair<std::string, unsigned int>>>>& associations
                = cont_assoc_it.second;

        loginf << "CreateARTASAssociationsJob: saveAssociations: db content " << dbcontent_name;

        assert (buffers_.count(dbcontent_name));

        assert (dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).existsIn(dbcontent_name));
        assert (dbcontent_man.metaVariable(DBContent::meta_var_utn_.name()).existsIn(dbcontent_name));

        string rec_num_var_name =
                dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).getFor(dbcontent_name).name();
        string utn_var_name =
                dbcontent_man.metaVariable(DBContent::meta_var_utn_.name()).getFor(dbcontent_name).name();

        assert (buffers_.at(dbcontent_name)->has<unsigned int>(rec_num_var_name));
        assert (buffers_.at(dbcontent_name)->has<unsigned int>(utn_var_name));

        NullableVector<unsigned int>& rec_num_vec = buffers_.at(dbcontent_name)->get<unsigned int>(rec_num_var_name);
        NullableVector<unsigned int>& utn_vec = buffers_.at(dbcontent_name)->get<unsigned int>(utn_var_name);

        for (unsigned int cnt=0; cnt < buffers_.at(dbcontent_name)->size(); ++cnt)
        {
            assert (!rec_num_vec.isNull(cnt));

            rec_num = rec_num_vec.get(cnt);

            if (associations.count(rec_num))
            {
                //if (utn_vec.isNull(cnt))
                utn_vec.set(cnt, get<0>(associations.at(rec_num)));
                //else
                    //utn_vec.getRef(cnt).push_back(get<0>(associations.at(rec_num)));

                ++num_associated;
            }
            else
                ++num_not_associated;
        }

        association_counts_[dbcontent_name] = {buffers_.at(dbcontent_name)->size(), num_associated};

        loginf << "CreateARTASAssociationsJob: saveAssociations: dcontent " << dbcontent_name
               <<  " assoc " << num_associated << " not assoc " << num_not_associated;
    }

    // delete all data from buffer except rec_nums and associations, rename to db column names
    for (auto& buf_it : buffers_)
    {
        string dbcontent_name = buf_it.first;

        string rec_num_var_name =
                dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).getFor(dbcontent_name).name();
        string rec_num_col_name =
                dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).getFor(dbcontent_name).dbColumnName();

        string utn_var_name =
                dbcontent_man.metaVariable(DBContent::meta_var_utn_.name()).getFor(dbcontent_name).name();
        string utn_col_name =
                dbcontent_man.metaVariable(DBContent::meta_var_utn_.name()).getFor(dbcontent_name).dbColumnName();


        PropertyList properties = buf_it.second->properties();

        for (auto& prop_it : properties.properties())
        {
            if (prop_it.name() == rec_num_var_name)
                buf_it.second->rename<unsigned int>(rec_num_var_name, rec_num_col_name);
            else if (prop_it.name() == utn_var_name)
                buf_it.second->rename<unsigned int>(utn_var_name, utn_col_name);
            else
                buf_it.second->deleteProperty(prop_it);
        }
    }

    // actually save data, ok since DB job
    for (auto& buf_it : buffers_)
    {
        string dbcontent_name = buf_it.first;

        loginf << "CreateARTASAssociationsJob: saveAssociations: saving for " << dbcontent_name;

        DBContent& dbcontent = dbcontent_man.dbContent(buf_it.first);
        dbContent::Variable& key_var =
                dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).getFor(dbcontent_name);

        unsigned int chunk_size = 50000;

        unsigned int steps = buf_it.second->size() / chunk_size;

        unsigned int index_from = 0;
        unsigned int index_to = 0;

        for (unsigned int cnt = 0; cnt <= steps; cnt++)
        {
            index_from = cnt * chunk_size;
            index_to = index_from + chunk_size;

            if (index_to > buf_it.second->size() - 1)
                index_to = buf_it.second->size() - 1;

            loginf << "CreateARTASAssociationsJob: saveAssociations: step " << cnt << " steps " << steps << " from "
                   << index_from << " to " << index_to;

            db_interface_.updateBuffer(dbcontent.dbTableName(), key_var.dbColumnName(),
                                       buf_it.second, index_from, index_to);

        }

    }

    buffers_.clear();

    loginf << "CreateARTASAssociationsJob: saveAssociations: done";
}

void CreateARTASAssociationsJob::createSensorAssociations()
{
    loginf << "CreateARTASAssociationsJob: createSensorAssociations";
    // for each rec_num + tri, find sensor hash + rec_num

    DBContentManager& object_man = COMPASS::instance().dbContentManager();

    for (auto& dbo_it : object_man)
    {
        if (dbo_it.first != tracker_dbcontent_name_ && dbo_it.second->hasData())
        {
            string status = "Creating " + dbo_it.first + " Hash List";
            emit statusSignal(status.c_str());
            createSensorHashes(*dbo_it.second);
        }
    }

    vector<string> tri_splits;
    bool match_found;
    bool best_match_dubious;
    string best_match_dubious_comment;

    string best_match_dbcontent_name;
    int best_match_rec_num{-1};
    boost::posix_time::ptime best_match_ts;
    boost::posix_time::ptime tri_ts;

    typedef multimap<string, pair<int, boost::posix_time::ptime>>::iterator HashIterator;
    pair<HashIterator, HashIterator> possible_hash_matches;

    assert(!first_track_ts_.is_not_a_date_time());  // has to be set

    emit statusSignal("Creating Associations");
    for (auto& ut_it : finished_tracks_)  // utn -> UAT, for each unqique target
    {
        logdbg << "CreateARTASAssociationsJob: createSensorAssociations: utn " << ut_it.first;

        for (auto& assoc_it :
             ut_it.second.rec_nums_tris_)  // rec_num -> (tri, tod), for each TRIs compound string
        {
            if (!assoc_it.second.first.size())  // empty tri, ignored update
                continue;

            tri_splits = String::split(assoc_it.second.first, ';');
            tri_ts = assoc_it.second.second;

            for (auto& tri : tri_splits)  // for each referenced hash
            {
                match_found = false;  // indicates if there was already a (previous) match found
                best_match_dubious = false;  // indicates if the association is dubious
                best_match_dubious_comment = "";

                for (auto& dbo_it : object_man)  // iterate over all dbos
                {
                    if (!dbo_it.second->hasData())
                        continue;

                    if (sensor_hashes_.count(dbo_it.first))  // if dbo has hash values
                    {
                        possible_hash_matches = sensor_hashes_.at(dbo_it.first).equal_range(tri);

                        for (HashIterator it = possible_hash_matches.first;
                             it != possible_hash_matches.second; ++it)
                        {
                            pair<int, boost::posix_time::ptime>& match = it->second;  // rec_num, tod

                            if (!isPossibleAssociation(tri_ts, match.second))
                                continue;

                            if (match_found)
                            {
                                logdbg << "CreateARTASAssociationsJob: createSensorAssociations: "
                                          "found duplicate hash '"
                                       << tri << "' in dbo " << dbo_it.first << " rec num "
                                       << match.first;

                                if (isAssociationHashCollisionInDubiousTime(tri_ts,
                                                                            best_match_ts) &&
                                    isAssociationHashCollisionInDubiousTime(tri_ts, match.second))
                                {
                                    best_match_dubious = true;
                                    best_match_dubious_comment =
                                        tri + " has multiple matches in close time at " +
                                        Time::toString(tri_ts);
                                }
                                else  // not dubious
                                {
                                    best_match_dubious = false;
                                    best_match_dubious_comment = "";
                                }

                                // store if closer in time
                                if ((tri_ts - match.second).abs() < (tri_ts - best_match_ts).abs())
                                {
                                    if (isAssociationInDubiousDistantTime(tri_ts, match.second))
                                    {
                                        best_match_dubious = true;
                                        best_match_dubious_comment =
                                            tri + " in too distant time (" +
                                            Time::toString(tri_ts - match.second) + ") at " +
                                            Time::toString(tri_ts);
                                    }
                                    else  // not dubious
                                    {
                                        best_match_dubious = false;
                                        best_match_dubious_comment = "";
                                    }

                                    best_match_dbcontent_name = dbo_it.first;
                                    best_match_rec_num = match.first;  // rec_num
                                    best_match_ts = match.second;     // tod
                                    match_found = true;
                                }

                                found_hash_duplicates_cnt_++;
                            }
                            else  // store as best match
                            {
                                if (isAssociationInDubiousDistantTime(tri_ts, match.second))
                                {
                                    best_match_dubious = true;
                                    best_match_dubious_comment =
                                        tri + " in too distant time (" +
                                        Time::toString(tri_ts - match.second) + "s) at " +
                                        Time::toString(tri_ts);
                                }

                                best_match_dbcontent_name = dbo_it.first;
                                best_match_rec_num = match.first;  // rec_num
                                best_match_ts = match.second;     // tod
                                match_found = true;
                            }
                        }
                    }
                }

                if (match_found)
                {
                    if (best_match_dubious)
                    {
                        loginf << "CreateARTASAssociationsJob: createSensorAssociations: utn "
                               << ut_it.first << " match rec_num " << best_match_rec_num
                               << " is dubious because " << best_match_dubious_comment;
                        ++dubious_associations_cnt_;
                    }

//                    object_man.object(best_match_dbcontent_name)
//                        .addAssociation(best_match_rec_num, ut_it.first, true, assoc_it.first);

                    // add utn to non-tracker rec_num
                    associations_[best_match_dbcontent_name][best_match_rec_num] =
                            make_tuple(ut_it.first, std::vector<std::pair<std::string, unsigned int>>());

                    // add non-tracker rec_num to tracker src rec_nums

                    assert (associations_.count(tracker_dbcontent_name_));
                    assert (associations_.at(tracker_dbcontent_name_).count(assoc_it.first));

                    get<1>(associations_.at(tracker_dbcontent_name_).at(assoc_it.first)).push_back(
                                {best_match_dbcontent_name, best_match_rec_num});

                    ++found_hashes_cnt_;
                }
                else
                {
                    logdbg << "CreateARTASAssociationsJob: createSensorAssociations: utn "
                           << ut_it.first << " has missing hash '" << tri << "' at "
                           << Time::toString(tri_ts);

                    // assert (tri_tod-first_track_tod_ >= 0);

                    if (isTimeAtBeginningOrEnd(tri_ts))
                        ++acceptable_missing_hashes_cnt_;
                    else
                    {
                        loginf << "CreateARTASAssociationsJob: createSensorAssociations: utn "
                               << ut_it.first << " has missing hash '" << tri << "' at "
                               << Time::toString(tri_ts);

                        missing_hashes_.emplace(tri, make_pair(ut_it.first, assoc_it.first));
                        ++missing_hashes_cnt_;
                    }
                }
            }
        }
    }

    loginf << "CreateARTASAssociationsJob: createSensorAssociations: done with "
           << found_hashes_cnt_ << " found, " << acceptable_missing_hashes_cnt_
           << " missing at beginning, " << missing_hashes_cnt_ << " missing, "
           << found_hash_duplicates_cnt_ << " duplicates";
}

void CreateARTASAssociationsJob::removePreviousAssociations()
{
    loginf << "CreateARTASAssociationsJob: removePreviousAssociations";

    DBContentManager& object_man = COMPASS::instance().dbContentManager();

    for (auto& buf_it : buffers_)
    {
        assert (object_man.metaVariable(DBContent::meta_var_utn_.name()).existsIn(buf_it.first));

        string utn_var_name =
                object_man.metaVariable(DBContent::meta_var_utn_.name()).getFor(buf_it.first).name();

        assert (buf_it.second->has<unsigned int>(utn_var_name));
        NullableVector<unsigned int>& assoc_vec = buf_it.second->get<unsigned int>(utn_var_name);

        assoc_vec.setAllNull();
    }
}

bool CreateARTASAssociationsJob::isPossibleAssociation(boost::posix_time::ptime ts_track,
                                                       boost::posix_time::ptime ts_target)
{
    if (ts_target > ts_track)  // target update in the future
        return ts_target - ts_track <= association_time_future_;
    else  // target update in the past (or same time)
        return ts_track - ts_target <= association_time_past_;
}

bool CreateARTASAssociationsJob::isAssociationInDubiousDistantTime(boost::posix_time::ptime ts_track,
                                                                   boost::posix_time::ptime ts_target)
{
    if (ts_target > ts_track)  // target update in the future
        return false;            // only measured in the past
    else                         // target update in the past (or same time)
        return ts_track - ts_target >= associations_dubious_distant_time_;
}

bool CreateARTASAssociationsJob::isAssociationHashCollisionInDubiousTime(boost::posix_time::ptime ts_track,
                                                                         boost::posix_time::ptime ts_target)
{
    if (ts_target > ts_track)  // target update in the future
        return ts_target - ts_track <= association_dubious_close_time_future_;
    else  // target update in the past (or same time)
        return ts_track - ts_target <= association_dubious_close_time_past_;
}

bool CreateARTASAssociationsJob::isTimeAtBeginningOrEnd(boost::posix_time::ptime ts_track)
{
    return ((ts_track - first_track_ts_).abs() <= misses_acceptable_time_) ||
           ((last_track_ts_ - ts_track).abs() <= misses_acceptable_time_);
}

void CreateARTASAssociationsJob::createSensorHashes(DBContent& object)
{
    loginf << "CreateARTASAssociationsJob: createSensorHashes: object " << object.name();

    if (!buffers_.count(object.name()))
    {
        logwrn << "CreateARTASAssociationsJob: createSensorHashes: no data found";
        return;
    }

    string dbcontent_name = object.name();
    shared_ptr<Buffer> buffer = buffers_.at(dbcontent_name);
    size_t buffer_size = buffer->size();

    using namespace dbContent;

    MetaVariable* key_meta_var = task_.keyVar();
    MetaVariable* hash_meta_var = task_.hashVar();
    MetaVariable* ts_meta_var = task_.timestampVar();

    Variable& key_var = key_meta_var->getFor(dbcontent_name);
    Variable& hash_var = hash_meta_var->getFor(dbcontent_name);
    Variable& ts_var = ts_meta_var->getFor(dbcontent_name);

    assert(buffer->has<unsigned int>(key_var.name()));
    assert(buffer->has<string>(hash_var.name()));
    assert(buffer->has<boost::posix_time::ptime>(ts_var.name()));

    NullableVector<unsigned int> rec_nums = buffer->get<unsigned int>(key_var.name());
    NullableVector<string> hashes = buffer->get<string>(hash_var.name());
    NullableVector<boost::posix_time::ptime> ts_vec = buffer->get<boost::posix_time::ptime>(ts_var.name());

    for (size_t cnt = 0; cnt < buffer_size; ++cnt)
    {
        assert(!rec_nums.isNull(cnt));
        assert(!hashes.isNull(cnt));

        if (ts_vec.isNull(cnt))
        {
            logwrn << "CreateARTASAssociationsJob: createSensorHashes: rec_num "
                   << rec_nums.get(cnt) << " of dbo " << object.name() << " has no time, skipping";
            continue;
        }

        assert(!ts_vec.isNull(cnt));

        // dbo -> hash -> rec_num, tod
        // map <string, multimap<string, pair<int, float>>> sensor_hashes_;

        sensor_hashes_[dbcontent_name].emplace(hashes.get(cnt),
                                         make_pair(rec_nums.get(cnt), ts_vec.get(cnt)));
    }
}

std::map<std::string, std::pair<unsigned int, unsigned int> > CreateARTASAssociationsJob::associationCounts() const
{
    return association_counts_;
}

void CreateARTASAssociationsJob::setSaveQuestionAnswer(bool value)
{
    loginf << "CreateARTASAssociationsJob: setSaveQuestionAnswer: value " << value;

    save_question_answer_ = value;
    save_question_answered_ = true;
}
