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

public:
    CreateARTASAssociationsJob(CreateARTASAssociationsTask& task, DBInterface& db_interface,
                               std::map<std::string, std::shared_ptr<Buffer>> buffers);

    virtual ~CreateARTASAssociationsJob();

    virtual void run ();

protected:
    CreateARTASAssociationsTask& task_;
    DBInterface& db_interface_;
    std::map<std::string, std::shared_ptr<Buffer>> buffers_;

    const std::string tracker_dbo_name_{"Tracker"};
    std::map<int, UniqueARTASTrack> finished_tracks_; // utn -> unique track

    // dbo -> hash -> rec_num, tod
    std::map <std::string, std::multimap<std::string, std::pair<int, float>>> sensor_hashes_;

    void createUTNS ();
    void createARTASAssociations();
    void createSensorAssociations();
    void createSensorHashes (DBObject& object);

    std::map<unsigned int, unsigned int> track_rec_num_utns_; // track rec num -> utn
};

#endif // CREATEARTASASSOCIATIONSJOB_H
