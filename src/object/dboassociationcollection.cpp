#include "dboassociationcollection.h"

#include <sstream>

void DBOAssociationCollection::add(unsigned int rec_num, DBOAssociationEntry&& entry)
{
    entries_.emplace(rec_num, entry);
}

bool DBOAssociationCollection::contains(unsigned int rec_num) const
{
    return entries_.count(rec_num);
}

std::vector<unsigned int> DBOAssociationCollection::getUTNsFor(unsigned int rec_num) const
{
    std::vector<unsigned int> ret;

    typedef std::multimap<unsigned int, DBOAssociationEntry>::const_iterator MMAPIterator;

    std::pair<MMAPIterator, MMAPIterator> result = entries_.equal_range(rec_num);

    for (MMAPIterator it = result.first; it != result.second; it++)
        ret.push_back(it->second.utn_);

    return ret;
}

std::string DBOAssociationCollection::getUTNsStringFor(unsigned int rec_num) const
{
    std::stringstream ss;

    typedef std::multimap<unsigned int, DBOAssociationEntry>::const_iterator MMAPIterator;

    std::pair<MMAPIterator, MMAPIterator> result = entries_.equal_range(rec_num);

    bool first = true;

    for (MMAPIterator it = result.first; it != result.second; it++)
    {
        if (first)
            ss << std::to_string(it->second.utn_);
        else
            ss << "," << std::to_string(it->second.utn_);

        first = false;
    }

    return ss.str();
}

std::vector<unsigned int> DBOAssociationCollection::getRecNumsForUTN(unsigned int utn) const
{
    std::vector<unsigned int> result;

    for (auto& entry : entries_)
    {
        if (entry.second.utn_ == utn)
            result.push_back(entry.first);
    }

    return result;
}
