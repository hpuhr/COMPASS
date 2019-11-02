#ifndef DBOASSOCIATIONENTRY_H
#define DBOASSOCIATIONENTRY_H

#include <map>

class DBOAssociationEntry
{
public:
    //unsigned int assoc_id, unsigned int rec_num,
    //assoc_id_(assoc_id), rec_num_(rec_num),
    DBOAssociationEntry (unsigned int utn, unsigned int src_rec_num)
        : utn_(utn), src_rec_num_(src_rec_num)
    {
    }

    //unsigned int assoc_id_;
    //unsigned int rec_num_;
    unsigned int utn_;
    unsigned int src_rec_num_;
};

using DBOAssociationCollection = typename std::multimap<unsigned int, DBOAssociationEntry>; // rec_num -> assoc entry

#endif // DBOASSOCIATIONENTRY_H
