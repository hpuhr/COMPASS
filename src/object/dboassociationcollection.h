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

#ifndef DBOASSOCIATIONCOLLECTION_H
#define DBOASSOCIATIONCOLLECTION_H

#include <string>
#include <map>
#include <vector>
#include <set>

class DBOAssociationEntry
{
  public:
    // unsigned int assoc_id, unsigned int rec_num,
    // assoc_id_(assoc_id), rec_num_(rec_num),
    DBOAssociationEntry(unsigned int utn, bool has_src, unsigned int src_rec_num)
        : utn_(utn), has_src_(has_src), src_rec_num_(src_rec_num)
    {
    }

    // unsigned int assoc_id_;
    // unsigned int rec_num_;
    unsigned int utn_;
    bool has_src_;
    unsigned int src_rec_num_;
};

// using DBOAssociationCollection = typename std::multimap<unsigned int, DBOAssociationEntry>; //
// rec_num -> assoc entry

class DBOAssociationCollection
{
  public:
    DBOAssociationCollection() = default;

    void add(unsigned int rec_num, DBOAssociationEntry&& entry);
    void clear() { entries_.clear(); }
    size_t size() const { return entries_.size(); }

    typedef typename std::multimap<unsigned int, DBOAssociationEntry> collection_type;
    typedef typename collection_type::iterator iterator;
    typedef typename collection_type::const_iterator const_iterator;

    inline iterator begin() { return entries_.begin(); }
    inline iterator end() { return entries_.end(); }

    inline const_iterator cbegin() const noexcept { return entries_.cbegin(); }
    inline const_iterator cend() const noexcept { return entries_.cend(); }

    //    ConstDBOAssociationEntryIterator const_begin() const { return entries_.begin(); }
    //    ConstDBOAssociationEntryIterator const_end() const { return entries_.end(); }

    bool contains(unsigned int rec_num) const;
    std::vector<unsigned int> getUTNsFor(unsigned int rec_num) const;
    std::string getUTNsStringFor(unsigned int rec_num) const;

    bool hasRecNumsForUTN(unsigned int utn) const;
    std::vector<unsigned int> getRecNumsForUTN(unsigned int utn) const;
    std::set<unsigned int> getAllUTNS () const;

  protected:
    // rec_num -> assoc entry
    std::multimap<unsigned int, DBOAssociationEntry> entries_; // rec_num -> entry
};

#endif  // DBOASSOCIATIONCOLLECTION_H
