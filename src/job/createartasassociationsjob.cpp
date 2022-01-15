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
#include "dbovariable.h"
#include "metadbovariable.h"
#include "stringconv.h"

#include <math.h>

#include <QThread>
#include <algorithm>

using namespace Utils;

CreateARTASAssociationsJob::CreateARTASAssociationsJob(
    CreateARTASAssociationsTask& task, DBInterface& db_interface,
    std::map<std::string, std::shared_ptr<Buffer>> buffers)
    : Job("CreateARTASAssociationsJob"), task_(task), db_interface_(db_interface), buffers_(buffers)
{
    misses_acceptable_time_ = task_.missesAcceptableTime();

    association_time_past_ = task_.associationTimePast();
    association_time_future_ = task_.associationTimeFuture();  // will be negative time diff

    associations_dubious_distant_time_ = task_.associationsDubiousDistantTime();
    association_dubious_close_time_past_ = task_.associationDubiousCloseTimePast();
    association_dubious_close_time_future_ =
        task_.associationDubiousCloseTimeFuture();  // will be negative time diff
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

    DBContentManager& object_man = COMPASS::instance().objectManager();

    object_man.removeAssociations();

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
        std::stringstream ss;
        ss << "There are " << missing_hashes_cnt_ << " missing hashes and "
           << dubious_associations_cnt_
           << " dubious associations.\nDo you want to still save the associations?";
        emit saveAssociationsQuestionSignal(ss.str().c_str());

        while (!save_question_answered_)
            QThread::msleep(1);

        if (!save_question_answer_)  // nope
        {
            emit statusSignal("Clearing created Associations");

            object_man.removeAssociations();

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
    for (auto& dbo_it : object_man)
    {
        loginf << "CreateARTASAssociationsJob: run: processing object " << dbo_it.first
               << " associated " << dbo_it.second->associations().size() << " of "
               << dbo_it.second->count();
        dbo_it.second->saveAssociations();
    }

    object_man.setAssociationsDataSource(tracker_dbo_name_, task_.currentDataSourceName());

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

    if (!buffers_.count(tracker_dbo_name_))
    {
        logwrn << "CreateARTASAssociationsJob: createUTNS: no tracker data found";
        return;
    }

    std::shared_ptr<Buffer> buffer = buffers_.at(tracker_dbo_name_);
    size_t buffer_size = buffer->size();

    assert(buffer->has<int>(task_.trackerTrackNumVarStr()));
    assert(buffer->has<std::string>(task_.trackerTrackBeginVarStr()));
    assert(buffer->has<std::string>(task_.trackerTrackEndVarStr()));
    assert(buffer->has<std::string>(task_.trackerTrackCoastingVarStr()));

    assert(buffer->has<int>(task_.keyVar()->getNameFor(tracker_dbo_name_)));
    assert(buffer->has<std::string>(task_.hashVar()->getNameFor(tracker_dbo_name_)));
    assert(buffer->has<float>(task_.todVar()->getNameFor(tracker_dbo_name_)));

    NullableVector<int> track_nums = buffer->get<int>(task_.trackerTrackNumVarStr());
    NullableVector<std::string> track_begins =
        buffer->get<std::string>(task_.trackerTrackBeginVarStr());
    NullableVector<std::string> track_ends =
        buffer->get<std::string>(task_.trackerTrackEndVarStr());
    NullableVector<std::string> track_coastings =
        buffer->get<std::string>(task_.trackerTrackCoastingVarStr());

    NullableVector<int> rec_nums = buffer->get<int>(task_.keyVar()->getNameFor(tracker_dbo_name_));
    NullableVector<std::string> hashes =
        buffer->get<std::string>(task_.hashVar()->getNameFor(tracker_dbo_name_));
    NullableVector<float> tods = buffer->get<float>(task_.todVar()->getNameFor(tracker_dbo_name_));

    std::map<int, UniqueARTASTrack> current_tracks;  // utn -> unique track
    std::map<int, int> current_track_mappings;       // track_num -> utn

    int utn_cnt{0};

    bool tri_set;
    std::string tri;
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
    float tod;

    int utn;
    // bool new_track_created;
    bool finish_previous_track;
    bool ignore_update;

    float track_end_time = task_.endTrackTime();  // time-delta after which begin a new track

    for (size_t cnt = 0; cnt < buffer_size; ++cnt)
    {
        // new_track_created = false;
        finish_previous_track = false;

        tri_set = !hashes.isNull(cnt);
        if (tri_set)
            tri = hashes.get(cnt);
        else
            tri = "";

        assert(!track_nums.isNull(cnt));
        track_num = track_nums.get(cnt);

        track_begin_set = !track_begins.isNull(cnt);
        if (track_begin_set)
            track_begin = track_begins.get(cnt) == "Y";
        else
            track_begin = false;

        track_end_set = !track_ends.isNull(cnt);
        if (track_end_set)
            track_end = track_ends.get(cnt) == "Y";
        else
            track_end = false;

        track_coasting_set = !track_coastings.isNull(cnt);
        if (track_coasting_set)
            track_coasting = track_coastings.get(cnt) == "Y";
        else
            track_coasting = false;

        assert(!rec_nums.isNull(cnt));
        rec_num = rec_nums.get(cnt);

        assert(!tods.isNull(cnt));
        tod = tods.get(cnt);

        // was loaded as sorted in time
        if (cnt == 0)  // store first time
            first_track_tod_ = tod;

        last_track_tod_ = tod;  // store last time

        if (track_begin_set && track_begin && current_track_mappings.count(track_num))
        {
            logdbg << "CreateARTASAssociationsJob: createUTNS: finalizing track utn " << utn
                   << " track end is set";

            finish_previous_track = true;
        }

        // check if existed but last update was more than 10m ago
        if (!finish_previous_track && current_track_mappings.count(track_num) &&
            tod - current_tracks.at(current_track_mappings.at(track_num)).last_tod_ >
                track_end_time)
        {
            logdbg << "CreateARTASAssociationsJob: createUTNS: finalizing track utn "
                   << current_track_mappings.at(track_num) << " since tod difference "
                   << tod - current_tracks.at(current_track_mappings.at(track_num)).last_tod_;

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
                   << " track num " << track_num << " tod " << String::timeStringFromDouble(tod)
                   << " begin " << (track_begin_set ? std::to_string(track_begin) : " not set");

            utn = utn_cnt;
            ++utn_cnt;

            current_track_mappings[track_num] = utn;

            current_tracks[utn].utn = utn;
            current_tracks[utn].track_num = track_num;
            current_tracks[utn].first_tod_ = tod;

            // new_track_created = true;
        }
        else
            utn = current_track_mappings.at(track_num);

        assert(current_tracks.count(utn));
        UniqueARTASTrack& unique_track = current_tracks.at(utn);
        unique_track.last_tod_ = tod;

        // add tris if not to be ignored
        ignore_update =
            ((track_end_set && track_end && ignore_track_end_associations) ||
             (track_coasting_set && track_coasting && ignore_track_coasting_associations));

        if (ignore_update)
        {
            logdbg << "CreateARTASAssociationsJob: createUTNS: ignoring rec num " << rec_num;
            // add empty tri so that at least track update is associated
            unique_track.rec_nums_tris_[rec_num] = std::make_pair("", tod);
            ++ignored_track_updates_cnt_;
        }
        else
            unique_track.rec_nums_tris_[rec_num] = std::make_pair(tri, tod);

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

    DBContentManager& object_man = COMPASS::instance().objectManager();
    DBContent& tracker_object = object_man.object(tracker_dbo_name_);

    for (auto& ut_it : finished_tracks_)                    // utn -> UAT
        for (auto& assoc_it : ut_it.second.rec_nums_tris_)  // rec_num -> tri
            tracker_object.addAssociation(assoc_it.first, ut_it.first, true, assoc_it.first);
}

void CreateARTASAssociationsJob::createSensorAssociations()
{
    loginf << "CreateARTASAssociationsJob: createSensorAssociations";
    // for each rec_num + tri, find sensor hash + rec_num

    DBContentManager& object_man = COMPASS::instance().objectManager();

    for (auto& dbo_it : object_man)
    {
        if (dbo_it.first != tracker_dbo_name_ && dbo_it.second->hasData())
        {
            std::string status = "Creating " + dbo_it.first + " Hash List";
            emit statusSignal(status.c_str());
            createSensorHashes(*dbo_it.second);
        }
    }

    std::vector<std::string> tri_splits;
    bool match_found;
    bool best_match_dubious;
    std::string best_match_dubious_comment;

    std::string best_match_dbo_name;
    int best_match_rec_num{-1};
    float best_match_tod{0};
    float tri_tod;

    typedef std::multimap<std::string, std::pair<int, float>>::iterator HashIterator;
    std::pair<HashIterator, HashIterator> possible_hash_matches;

    assert(first_track_tod_ > 0);  // has to be set

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
            tri_tod = assoc_it.second.second;

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
                            std::pair<int, float>& match = it->second;  // rec_num, tod

                            if (!isPossibleAssociation(tri_tod, match.second))
                                continue;

                            if (match_found)
                            {
                                logdbg << "CreateARTASAssociationsJob: createSensorAssociations: "
                                          "found duplicate hash '"
                                       << tri << "' in dbo " << dbo_it.first << " rec num "
                                       << match.first;

                                if (isAssociationHashCollisionInDubiousTime(tri_tod,
                                                                            best_match_tod) &&
                                    isAssociationHashCollisionInDubiousTime(tri_tod, match.second))
                                {
                                    best_match_dubious = true;
                                    best_match_dubious_comment =
                                        tri + " has multiple matches in close time at " +
                                        String::timeStringFromDouble(tri_tod);
                                }
                                else  // not dubious
                                {
                                    best_match_dubious = false;
                                    best_match_dubious_comment = "";
                                }

                                // store if closer in time
                                if (fabs(tri_tod - match.second) < fabs(tri_tod - best_match_tod))
                                {
                                    if (isAssociationInDubiousDistantTime(tri_tod, match.second))
                                    {
                                        best_match_dubious = true;
                                        best_match_dubious_comment =
                                            tri + " in too distant time (" +
                                            std::to_string(tri_tod - match.second) + "s) at " +
                                            String::timeStringFromDouble(tri_tod);
                                    }
                                    else  // not dubious
                                    {
                                        best_match_dubious = false;
                                        best_match_dubious_comment = "";
                                    }

                                    best_match_dbo_name = dbo_it.first;
                                    best_match_rec_num = match.first;  // rec_num
                                    best_match_tod = match.second;     // tod
                                    match_found = true;
                                }

                                found_hash_duplicates_cnt_++;
                            }
                            else  // store as best match
                            {
                                if (isAssociationInDubiousDistantTime(tri_tod, match.second))
                                {
                                    best_match_dubious = true;
                                    best_match_dubious_comment =
                                        tri + " in too distant time (" +
                                        std::to_string(tri_tod - match.second) + "s) at " +
                                        String::timeStringFromDouble(tri_tod);
                                }

                                best_match_dbo_name = dbo_it.first;
                                best_match_rec_num = match.first;  // rec_num
                                best_match_tod = match.second;     // tod
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

                    object_man.object(best_match_dbo_name)
                        .addAssociation(best_match_rec_num, ut_it.first, true, assoc_it.first);
                    ++found_hashes_cnt_;
                }
                else
                {
                    logdbg << "CreateARTASAssociationsJob: createSensorAssociations: utn "
                           << ut_it.first << " has missing hash '" << tri << "' at "
                           << String::timeStringFromDouble(tri_tod);

                    // assert (tri_tod-first_track_tod_ >= 0);

                    if (isTimeAtBeginningOrEnd(tri_tod))
                        ++acceptable_missing_hashes_cnt_;
                    else
                    {
                        loginf << "CreateARTASAssociationsJob: createSensorAssociations: utn "
                               << ut_it.first << " has missing hash '" << tri << "' at "
                               << String::timeStringFromDouble(tri_tod);

                        missing_hashes_.emplace(tri, std::make_pair(ut_it.first, assoc_it.first));
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

bool CreateARTASAssociationsJob::isPossibleAssociation(float tod_track, float tod_target)
{
    if (tod_target > tod_track)  // target update in the future
        return tod_target - tod_track <= association_time_future_;
    else  // target update in the past (or same time)
        return tod_track - tod_target <= association_time_past_;
}

bool CreateARTASAssociationsJob::isAssociationInDubiousDistantTime(float tod_track,
                                                                   float tod_target)
{
    if (tod_target > tod_track)  // target update in the future
        return false;            // only measured in the past
    else                         // target update in the past (or same time)
        return tod_track - tod_target >= associations_dubious_distant_time_;
}

bool CreateARTASAssociationsJob::isAssociationHashCollisionInDubiousTime(float tod_track,
                                                                         float tod_target)
{
    if (tod_target > tod_track)  // target update in the future
        return tod_target - tod_track <= association_dubious_close_time_future_;
    else  // target update in the past (or same time)
        return tod_track - tod_target <= association_dubious_close_time_past_;
}

bool CreateARTASAssociationsJob::isTimeAtBeginningOrEnd(float tod_track)
{
    return (fabs(tod_track - first_track_tod_) <= misses_acceptable_time_) ||
           (fabs(last_track_tod_ - tod_track) <= misses_acceptable_time_);
}

void CreateARTASAssociationsJob::createSensorHashes(DBContent& object)
{
    loginf << "CreateARTASAssociationsJob: createSensorHashes: object " << object.name();

    if (!buffers_.count(object.name()))
    {
        logwrn << "CreateARTASAssociationsJob: createSensorHashes: no data found";
        return;
    }

    std::string dbo_name = object.name();
    std::shared_ptr<Buffer> buffer = buffers_.at(dbo_name);
    size_t buffer_size = buffer->size();

    MetaDBOVariable* key_meta_var = task_.keyVar();
    MetaDBOVariable* hash_meta_var = task_.hashVar();
    MetaDBOVariable* tod_meta_var = task_.todVar();

    DBContentVariable& key_var = key_meta_var->getFor(dbo_name);
    DBContentVariable& hash_var = hash_meta_var->getFor(dbo_name);
    DBContentVariable& tod_var = tod_meta_var->getFor(dbo_name);

    assert(buffer->has<int>(key_var.name()));
    assert(buffer->has<std::string>(hash_var.name()));
    assert(buffer->has<float>(tod_var.name()));

    NullableVector<int> rec_nums = buffer->get<int>(key_var.name());
    NullableVector<std::string> hashes = buffer->get<std::string>(hash_var.name());
    NullableVector<float> tods = buffer->get<float>(tod_var.name());

    for (size_t cnt = 0; cnt < buffer_size; ++cnt)
    {
        assert(!rec_nums.isNull(cnt));
        assert(!hashes.isNull(cnt));

        if (tods.isNull(cnt))
        {
            logwrn << "CreateARTASAssociationsJob: createSensorHashes: rec_num "
                   << rec_nums.get(cnt) << " of dbo " << object.name() << " has no time, skipping";
            continue;
        }

        assert(!tods.isNull(cnt));

        // dbo -> hash -> rec_num, tod
        // std::map <std::string, std::multimap<std::string, std::pair<int, float>>> sensor_hashes_;

        sensor_hashes_[dbo_name].emplace(hashes.get(cnt),
                                         std::make_pair(rec_nums.get(cnt), tods.get(cnt)));
    }
}

void CreateARTASAssociationsJob::setSaveQuestionAnswer(bool value)
{
    loginf << "CreateARTASAssociationsJob: setSaveQuestionAnswer: value " << value;

    save_question_answer_ = value;
    save_question_answered_ = true;
}
