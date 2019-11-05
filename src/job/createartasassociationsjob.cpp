#include "createartasassociationsjob.h"

#include "boost/date_time/posix_time/posix_time.hpp"

#include "dbinterface.h"
#include "dbobjectmanager.h"
#include "dbobject.h"
#include "atsdb.h"
#include "buffer.h"
#include "createartasassociationstask.h"
#include "metadbovariable.h"
#include "dbovariable.h"

#include "stringconv.h"

#include <algorithm>
#include <math.h>

using namespace Utils;

CreateARTASAssociationsJob::CreateARTASAssociationsJob(CreateARTASAssociationsTask& task, DBInterface& db_interface,
                                                       std::map<std::string, std::shared_ptr<Buffer>> buffers)
: Job("CreateARTASAssociationsJob"), task_(task), db_interface_(db_interface), buffers_(buffers)
{

}

CreateARTASAssociationsJob::~CreateARTASAssociationsJob()
{

}

void CreateARTASAssociationsJob::run ()
{
    logdbg  << "CreateARTASAssociationsJob: run: start";

    started_ = true;

    boost::posix_time::ptime start_time;
    boost::posix_time::ptime stop_time;

    start_time = boost::posix_time::microsec_clock::local_time();

    // clear previous associations

    loginf  << "CreateARTASAssociationsJob: run: clearing associations";

    DBObjectManager& object_man = ATSDB::instance().objectManager();

    for (auto& dbo_it : object_man)
        dbo_it.second->clearAssociations();

    // create utns
    createUTNS();

    // create associations for artas tracks
    createARTASAssociations();

    // create associations for sensors
    createSensorAssociations();

    // save associations
    for (auto& dbo_it : object_man)
    {
        loginf  << "CreateARTASAssociationsJob: run: processing object " << dbo_it.first << " associated "
                << dbo_it.second->associations().size() << " of " << dbo_it.second->count();
        dbo_it.second->saveAssociations();
    }

    stop_time = boost::posix_time::microsec_clock::local_time();

    double load_time;
    boost::posix_time::time_duration diff = stop_time - start_time;
    load_time= diff.total_milliseconds()/1000.0;

    loginf  << "CreateARTASAssociationsJob: run: done (" << String::doubleToStringPrecision(load_time, 2) << " s).";
    done_=true;
}

size_t CreateARTASAssociationsJob::missingHashes() const
{
    return missing_hashes_;
}

size_t CreateARTASAssociationsJob::foundHashes() const
{
    return found_hashes_;
}

size_t CreateARTASAssociationsJob::foundDuplicates() const
{
    return found_duplicates_;
}

size_t CreateARTASAssociationsJob::dubiousAssociations() const
{
    return dubious_associations_;
}

size_t CreateARTASAssociationsJob::missingHashesAtBeginning() const
{
    return missing_hashes_at_beginning_;
}

void CreateARTASAssociationsJob::createUTNS ()
{
    loginf << "CreateARTASAssociationsJob: createUTNS";

    if (!buffers_.count(tracker_dbo_name_))
    {
        logwrn << "CreateARTASAssociationsJob: createUTNS: no tracker data found";
        return;
    }

    std::shared_ptr<Buffer> buffer = buffers_.at(tracker_dbo_name_);
    size_t buffer_size = buffer->size();

    assert (buffer->has<std::string>(task_.trackerTRIVarStr()));
    assert (buffer->has<int>(task_.trackerTrackNumVarStr()));
    assert (buffer->has<std::string>(task_.trackerTrackBeginVarStr()));
    assert (buffer->has<std::string>(task_.trackerTrackEndVarStr()));
    assert (buffer->has<int>(task_.keyVar()->getNameFor(tracker_dbo_name_)));
    assert (buffer->has<std::string>(task_.hashVar()->getNameFor(tracker_dbo_name_)));
    assert (buffer->has<float>(task_.todVar()->getNameFor(tracker_dbo_name_)));

    NullableVector<std::string> tris = buffer->get<std::string>(task_.trackerTRIVarStr());
    NullableVector<int> track_nums = buffer->get<int>(task_.trackerTrackNumVarStr());
    NullableVector<std::string> track_begins = buffer->get<std::string>(task_.trackerTrackBeginVarStr());
    NullableVector<std::string> track_ends = buffer->get<std::string>(task_.trackerTrackEndVarStr());
    NullableVector<int> rec_nums = buffer->get<int>(task_.keyVar()->getNameFor(tracker_dbo_name_));
    NullableVector<std::string> hashes = buffer->get<std::string>(task_.hashVar()->getNameFor(tracker_dbo_name_));
    NullableVector<float> tods = buffer->get<float>(task_.todVar()->getNameFor(tracker_dbo_name_));

    std::map<int, UniqueARTASTrack> current_tracks; // utn -> unique track
    std::map<int, int> current_track_mappings; // track_num -> utn

    int utn_cnt {0};

    bool tri_set;
    std::string tri;
    int track_num;
    bool track_begin_set;
    bool track_begin;
    bool track_end_set;
    bool track_end;
    int rec_num;
    std::string hash;
    float tod;

    int utn;
    bool new_track_created;
    bool finish_previous_track;

    for (size_t cnt=0; cnt < buffer_size; ++cnt)
    {
        new_track_created = false;
        finish_previous_track = false;

        tri_set = !tris.isNull(cnt);
        if (tri_set)
            tri = tris.get(cnt);
        else
            tri = "";

        assert (!track_nums.isNull(cnt));
        track_num = track_nums.get(cnt);

        track_begin_set = !track_begins.isNull(cnt);
        if (track_begin_set)
            track_begin = track_begins.get(cnt) == "1";
        else
            track_begin = false;

        track_end_set = !track_ends.isNull(cnt);
        if (track_end_set)
            track_end = track_ends.get(cnt) == "1";
        else
            track_end = false;

        assert (!rec_nums.isNull(cnt));
        rec_num = rec_nums.get(cnt);

        assert (!hashes.isNull(cnt));
        hash = hashes.get(cnt);

        assert (!tods.isNull(cnt));
        tod = tods.get(cnt);

        if (cnt == 0) // store first time, was loaded as sorted in time
            first_tod_ = tod;

        if (track_begin_set && track_begin && current_track_mappings.count(track_num))
        {
            logdbg << "CreateARTASAssociationsJob: createUTNS: finalizing track utn " << utn << " track end is set";

            finish_previous_track = true;
        }

        // check if existed but last update was more than 10m ago
        if (!finish_previous_track && current_track_mappings.count(track_num) &&
                tod - current_tracks.at(current_track_mappings.at(track_num)).last_tod_ > 600.0)
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
            assert (!finished_tracks_.count(utn));
            finished_tracks_[utn] = current_tracks.at(utn);
            current_tracks.erase(utn);
            current_track_mappings.erase(track_num);
        }

        if (!current_track_mappings.count(track_num)) // new track where none existed
        {
            logdbg << "CreateARTASAssociationsJob: createUTNS: new track utn " << utn_cnt << " track num " << track_num
                   << " tod " << String::timeStringFromDouble(tod)
                   << " begin " << (track_begin_set ? std::to_string(track_begin) : " not set");

            utn = utn_cnt;
            ++utn_cnt;

            current_track_mappings[track_num] = utn;

            current_tracks[utn].utn = utn;
            current_tracks[utn].track_num = track_num;
            current_tracks[utn].first_tod_ = tod;

            new_track_created = true;
        }
        else
            utn = current_track_mappings.at(track_num);


        assert (current_tracks.count (utn));
        UniqueARTASTrack& unique_track = current_tracks.at(utn);
        unique_track.last_tod_ = tod;
        unique_track.rec_nums_tris_[rec_num] = std::make_pair(tri, tod);

        if (track_end_set && track_end)
        {
            logdbg << "CreateARTASAssociationsJob: createUTNS: finalizing track utn " << utn
                   << " since track end is set";

            // finalize old track
            assert (!finished_tracks_.count(utn));
            finished_tracks_[utn] = current_tracks.at(utn);
            current_tracks.erase(utn);
            current_track_mappings.erase(track_num);
        }
    }

    // finish all and clean up
    for (auto& ut_it : current_tracks)
    {
        assert (!finished_tracks_.count(ut_it.first));
        finished_tracks_[ut_it.first] = ut_it.second;
    }

    current_tracks.clear();
    current_track_mappings.clear();

    loginf << "CreateARTASAssociationsJob: createUTNS: found " << finished_tracks_.size() << " finished tracks ";
}

void CreateARTASAssociationsJob::createARTASAssociations()
{
    loginf << "CreateARTASAssociationsJob: createARTASAssociations";

    DBObjectManager& object_man = ATSDB::instance().objectManager();
    DBObject& tracker_object = object_man.object(tracker_dbo_name_);

    for (auto& ut_it : finished_tracks_) // utn -> UAT
        for (auto& assoc_it : ut_it.second.rec_nums_tris_) // rec_num -> tri
            tracker_object.addAssociation(assoc_it.first, ut_it.first, assoc_it.first);
}

void CreateARTASAssociationsJob::createSensorAssociations()
{
    loginf << "CreateARTASAssociationsJob: createSensorAssociations";
    // for each rec_num + tri, find sensor hash + rec_num

    DBObjectManager& object_man = ATSDB::instance().objectManager();

    for (auto& dbo_it : object_man)
        if (dbo_it.first != tracker_dbo_name_)
            createSensorHashes(*dbo_it.second);

    std::vector<std::string> tri_splits;
    bool match_found;
    std::string best_match_dbo_name;
    int best_match_rec_num{-1};
    float best_match_tod{0};
    float tri_tod;
    //float tod_diff;

    typedef std::multimap<std::string, std::pair<int, float>>::iterator HashIterator;
    std::pair<HashIterator, HashIterator> possible_hash_matches;

    assert (first_tod_ > 0); // has to be set

    for (auto& ut_it : finished_tracks_) // utn -> UAT
    {
        logdbg << "CreateARTASAssociationsJob: createSensorAssociations: utn " << ut_it.first;

        for (auto& assoc_it : ut_it.second.rec_nums_tris_) // rec_num -> (tri, tod)
        {
            tri_splits = String::split(assoc_it.second.first, ';');
            tri_tod = assoc_it.second.second;

            for (auto& tri : tri_splits) // for each referenced hash
            {
                match_found = false;

                for (auto& dbo_it : object_man) // iterate over all dbos
                {
                    if (sensor_hashes_.count(dbo_it.first)) // if dbo has hash values
                    {
                        possible_hash_matches = sensor_hashes_.at(dbo_it.first).equal_range(tri);

                        for (HashIterator it = possible_hash_matches.first; it != possible_hash_matches.second; ++it)
                        {
                            std::pair<int, float>& match = it->second; // rec_num, tod

                            if (match_found)
                            {
                                logdbg << "CreateARTASAssociationsJob: createSensorAssociations: found duplicate hash '"
                                       << tri << "' in dbo " << dbo_it.first << " rec num " << match.first;

                                // store if closer in time
                                if (fabs (tri_tod-match.second) < fabs (tri_tod-best_match_tod))
                                {
                                    best_match_dbo_name = dbo_it.first;
                                    best_match_rec_num = match.first; // rec_num
                                    best_match_tod = match.second; // tod
                                    match_found = true;
                                }


                                found_duplicates_++;
                            }
                            else // store as best match
                            {
                                best_match_dbo_name = dbo_it.first;
                                best_match_rec_num = match.first; // rec_num
                                best_match_tod = match.second; // tod
                                match_found = true;
                            }

                        }
                    }
                }

                if (match_found)
                {
                    if (fabs (tri_tod-best_match_tod) > 30.0)
                    {
                        loginf << "CreateARTASAssociationsJob: createSensorAssociations: utn " << ut_it.first
                               << " has best matching hash " << fabs (tri_tod-best_match_tod) << "s apart from "
                               << String::timeStringFromDouble(tri_tod);
                        ++dubious_associations_;
                    }

                    object_man.object(best_match_dbo_name).addAssociation(best_match_rec_num, ut_it.first,
                                                                          assoc_it.first);
                    ++found_hashes_;
                }
                else
                {
                    logdbg << "CreateARTASAssociationsJob: createSensorAssociations: utn " << ut_it.first
                           << " has missing hash '" << tri << "' at " << String::timeStringFromDouble(tri_tod);

                    if (fabs (tri_tod-first_tod_) <= 30.0)
                        ++missing_hashes_at_beginning_;
                    else
                    {
                        loginf << "CreateARTASAssociationsJob: createSensorAssociations: utn " << ut_it.first
                               << " has missing hash '" << tri << "' at " << String::timeStringFromDouble(tri_tod);

                        ++missing_hashes_;
                    }
                }

            }
        }
    }

    loginf << "CreateARTASAssociationsJob: createSensorAssociations: done with " << found_hashes_ << " found, "
           << missing_hashes_at_beginning_ << " missing at beginning, " << missing_hashes_ << " missing, "
           << found_duplicates_ << " duplicates";

}

void CreateARTASAssociationsJob::createSensorHashes (DBObject& object)
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

    DBOVariable& key_var = key_meta_var->getFor(dbo_name);
    DBOVariable& hash_var = hash_meta_var->getFor(dbo_name);
    DBOVariable& tod_var = tod_meta_var->getFor(dbo_name);

    assert (buffer->has<int>(key_var.name()));
    assert (buffer->has<std::string>(hash_var.name()));
    assert (buffer->has<float>(tod_var.name()));

    NullableVector<int> rec_nums = buffer->get<int>(key_var.name());
    NullableVector<std::string> hashes = buffer->get<std::string>(hash_var.name());
    NullableVector<float> tods = buffer->get<float>(tod_var.name());

    for (size_t cnt=0; cnt < buffer_size; ++cnt)
    {
        assert (!rec_nums.isNull(cnt));
        assert (!hashes.isNull(cnt));
        assert (!tods.isNull(cnt));

        // dbo -> hash -> rec_num, tod
        //std::map <std::string, std::multimap<std::string, std::pair<int, float>>> sensor_hashes_;

        sensor_hashes_[dbo_name].emplace(hashes.get(cnt), std::make_pair(rec_nums.get(cnt), tods.get(cnt)));
    }
}
