#ifndef CREATEARTASASSOCIATIONSJOB_H
#define CREATEARTASASSOCIATIONSJOB_H

#include "job.h"

class CreateARTASAssociationsTask;
class DBInterface;
class Buffer;
class DBObject;

struct UniqueARTASTrack
{
    int utn;
    int track_num;
    std::map<int, std::pair<std::string, float>> rec_nums_tris_; // rec_num -> (tri, tod)
    //std::vector<int> track_nums_;
    float first_tod_;
    float last_tod_;
};


class CreateARTASAssociationsJob : public Job
{
    Q_OBJECT

signals:
    void statusSignal (QString status);
    void saveAssociationsQuestionSignal (QString question);

public:
    CreateARTASAssociationsJob(CreateARTASAssociationsTask& task, DBInterface& db_interface,
                               std::map<std::string, std::shared_ptr<Buffer>> buffers);

    virtual ~CreateARTASAssociationsJob();

    virtual void run ();

    void setSaveQuestionAnswer (bool value);

    size_t missingHashesAtBeginning() const;
    size_t missingHashes() const;
    size_t foundHashes() const;
    size_t foundHashDuplicates() const;
    size_t dubiousAssociations() const;

protected:
    CreateARTASAssociationsTask& task_;
    DBInterface& db_interface_;
    std::map<std::string, std::shared_ptr<Buffer>> buffers_;

    float misses_acceptable_time_ {0}; // time delta at beginning/end of recording where misses are acceptable

    float association_time_past_ {0}; // time_delta for which associations are considered into past time
    float association_time_future_ {0}; // time_delta for which associations are considered into future time

    float associations_dubious_distant_time_ {0};
    // time delta of tou where association is dubious bc too distant in time
    float association_dubious_close_time_past_ {0};
    // time delta of tou where association is dubious when multible hashes exist
    float association_dubious_close_time_future_ {0};
    // time delta of tou where association is dubious when multible hashes exist

    const std::string tracker_dbo_name_{"Tracker"};
    std::map<int, UniqueARTASTrack> finished_tracks_; // utn -> unique track

    // dbo -> hash -> rec_num, tod
    std::map <std::string, std::multimap<std::string, std::pair<int, float>>> sensor_hashes_;

    float first_track_tod_ {0};
    float last_track_tod_ {0};

    size_t ignored_track_updates_cnt_ {0};
    size_t acceptable_missing_hashes_cnt_ {0};
    size_t missing_hashes_cnt_ {0};
    std::multimap<std::string, std::pair<int, int>> missing_hashes_; // hash -> (utn, rec_num)
    size_t found_hashes_cnt_ {0}; // dbo name -> cnt

    size_t dubious_associations_cnt_ {0}; // counter for all dubious
    size_t found_hash_duplicates_cnt_ {0};

    volatile bool save_question_answered_ {false};
    volatile bool save_question_answer_ {false};

    void createUTNS ();
    void createARTASAssociations();
    void createSensorAssociations();
    void createSensorHashes (DBObject& object);

    std::map<unsigned int, unsigned int> track_rec_num_utns_; // track rec num -> utn

    bool isPossibleAssociation(float tod_track, float tod_target);
    bool isAssociationInDubiousDistantTime (float tod_track, float tod_target);
    bool isAssociationHashCollisionInDubiousTime(float tod_track, float tod_target);
    bool isTimeAtBeginningOrEnd(float tod_track);
};

#endif // CREATEARTASASSOCIATIONSJOB_H
