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

    // create utns
    createUTNS();

    // create associations for artas tracks

    // for each rec_num + tri, find sensor hash + rec_num

    // create associations for sensors

    // save associations


//    loginf  << "CreateARTASAssociationsJob: run: writing object " << dbobject_.name() << " size " << buffer_->size();
//    assert (buffer_->size());

    for (auto& dbo_it : ATSDB::instance().objectManager())
    {
        logdbg  << "CreateARTASAssociationsJob: run: processing object " << dbo_it.first;

        DBObject* object = dbo_it.second;

        size_t size = object->count();

        for (unsigned int cnt=0; cnt < size; ++cnt)
            object->addAssociation(cnt, cnt, cnt);

        object->saveAssociations();
    }


//    db_interface_.insertBuffer(dbobject_.currentMetaTable(), buffer_);
    stop_time = boost::posix_time::microsec_clock::local_time();

    double load_time;
    boost::posix_time::time_duration diff = stop_time - start_time;
    load_time= diff.total_milliseconds()/1000.0;

    loginf  << "CreateARTASAssociationsJob: run: done (" << String::doubleToStringPrecision(load_time, 2) << " s).";
    done_=true;
}

struct UniqueTrack
{
    int utn;
    int track_num;
    std::vector<int> rec_nums_;
    //std::vector<int> track_nums_;
    float first_tod_;
    float last_tod_;
};

void CreateARTASAssociationsJob::createUTNS ()
{
    loginf << "CreateARTASAssociationsJob: createUTNS";

    if (!buffers_.count("Tracker"))
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

    std::vector <std::pair<int, UniqueTrack>> finished_tracks; // utn -> unique track
    std::map<int, UniqueTrack> current_tracks; // utn -> unique track
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

        if (track_begin_set && track_begin && current_track_mappings.count(track_num))
        {
            loginf << "CreateARTASAssociationsJob: createUTNS: finalizing track utn " << utn << " track end is set";

            finish_previous_track = true;
        }

        // check if existed but last update was more than 10m ago
        if (!finish_previous_track && current_track_mappings.count(track_num) &&
                tod - current_tracks.at(current_track_mappings.at(track_num)).last_tod_ > 600.0)
        {
            loginf << "CreateARTASAssociationsJob: createUTNS: finalizing track utn "
                   << current_track_mappings.at(track_num) << " since tod difference "
                   << tod - current_tracks.at(current_track_mappings.at(track_num)).last_tod_;

            finish_previous_track = true;
        }

        if (finish_previous_track)
        {
            // finalize old track
            utn = current_track_mappings.at(track_num);
            finished_tracks.push_back(std::make_pair(utn, current_tracks.at(utn)));
            current_tracks.erase(utn);
            current_track_mappings.erase(track_num);
        }

        if (!current_track_mappings.count(track_num)) // new track where none existed
        {
            loginf << "CreateARTASAssociationsJob: createUTNS: new track utn " << utn_cnt << " track num " << track_num
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
        UniqueTrack& unique_track = current_tracks.at(utn);
        unique_track.last_tod_ = tod;
        unique_track.rec_nums_.push_back(rec_num);

        if (track_end_set && track_end)
        {
            loginf << "CreateARTASAssociationsJob: createUTNS: finalizing track utn " << utn
                   << " since track end is set";

            // finalize old track
            finished_tracks.push_back(std::make_pair(utn, current_tracks.at(utn)));
            current_tracks.erase(utn);
            current_track_mappings.erase(track_num);
        }
    }

    // finish all and clean up
    for (auto& ut_it : current_tracks)
        finished_tracks.push_back(std::make_pair(ut_it.first, ut_it.second));

    current_tracks.clear();
    current_track_mappings.clear();

    loginf << "CreateARTASAssociationsJob: createUTNS: found " << finished_tracks.size() << " finished tracks ";
}
