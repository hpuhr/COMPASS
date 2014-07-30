/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Buffer.h
 *
 *  Created on: Dec 8, 2011
 *      Author: sk
 */

#ifndef BUFFER_H_
#define BUFFER_H_

#include <vector>
#include "PropertyList.h"

class PropertyContainer;

/**
 * @brief Fast, dynamic data container
 *
 * Encapsulates general data storage with access functions with maximum efficiency.
 * Performs basic checks, allocates new PropertyContainers when space is needed, but NOT thread-safe.
 *
 * Is basically an aggregation of PropertyContainer instances, which provide the storage functionality. Provides access functions
 * based on a set index, calculates which PropertyContainer is used for the current index, and provides an interface for it by
 * forwarding the void pointer container (getAdresses).
 *
 * New PropertyContainers are allocated when the index requires it.
 *
 * To access the data, a vector with void pointers can be retrieved using getAdresses(). Each of the void* points to the
 * current element (at index, like a row), each position in the addresses vector corresponds to the Property in the member list.
 * One needs to cast the void* to the correct data type and read the data.
 *
 * Note the following example code:
 * @code
 * buffer->setIndex( 0 );
 *
 * std::vector<void*>* output_adresses;
 * unsigned int cnt;
 * unsigned n = buffer->getSize();
 *
 * for( cnt=0; cnt<n; ++cnt )
 * {
 *     if( cnt != 0 )
 *     {
 *         assert (buffer);
 *         buffer->incrementIndex();
 *     }
 *
 *     output_adresses = buffer->getAdresses();
 *
 *     if (isNan(data_type, output_adresses->at( col )))
 *         continue;
 *
 *     // do domething with output_adresses->at( col )
 * }
 *
 * @endcode
 *
 * Buffers can also exchange the underlying PropertyContainers to allow shallow copies or appending data.
 *
 * Another function is the key search. If within the buffer there exists a monotonous unsigned integer value, it can be used
 * as key. To activate this key search, use activateKeySearch() with the appropriate Property index. After that,
 * setIndexForKey() can be called.
 */
class Buffer
{
public:
    /// @brief Constructor.
    Buffer(PropertyList member_list, DB_OBJECT_TYPE type=DBO_UNDEFINED);
    /// @brief Desctructor.
    virtual ~Buffer();

    /// @brief Set addresses to point to values of element at index.
    void setIndex (unsigned int index);
    /// @brief Return void pointer to value at property number of current index.
    void *get (unsigned int property);
    /// @brief Return void pointer to value at property number of index.
    void *get (unsigned int index, unsigned int property);

    /// @brief Step to the next element.
    void incrementIndex ();
    /// @brief Step to the previous element.
    void decrementIndex ();

    /// @brief Adds all containers of org_buffer and removes them.
    void seizeBuffer (Buffer *org_buffer);

    /// @brief Adds an additional property.
    void addProperty (std::string id, PROPERTY_DATA_TYPE type);

    /// @brief Print function for debugging.
    void print (unsigned int num_elements);
    void printLong (unsigned int num_elements);

    /// @brief Returns boolean indicating if any data was ever written.
    bool getFirstWrite () { return first_write_;};
    /// @brief Sets data was written flag.
    void unsetFirstWrite () { first_write_=false; };

    /// @brief Returns boolean indicating if buffer is the last of one DB operation.
    bool getLastOne () { return last_one_;};
    /// @brief Sets if buffer is the last one of one DB operation.
    void setLastOne (bool last_one) { last_one_=last_one; };

    /// @brief Returns vector with void pointers to all values of the current element.
    std::vector<void *>* getAdresses();

    /// @brief Returns buffer containing the data of this buffer and re-initializes.
    Buffer *transferData ();

    /// @brief Returns a shallow copy of the this buffer.
    Buffer *getShallowCopy ();

    /// @brief Activates key search on given property index.
    void activateKeySearch (unsigned int key_pos);

    /// @brief Sets index that where key matches, returns true if found, else false.
    bool setIndexForKey (unsigned int key);

    /// @brief Sets all values of a property to NaN.
    void setPropertyValuesNan (unsigned int property_index);

    /// @brief Returns flag indicating if buffer is filled to capacity.
    bool isMaxIndex ();

    /// @brief Returns the buffers id
    unsigned int getId() const { return id_; }

    /// Unique buffer id, copied when getting shallow copies
    unsigned int id_;

    /// Copies current index from source buffer into this one. PropertyLists must have the same indices
    void deepCopyRecordFrom (Buffer *src);

private:
    /// List of all properties
    PropertyList member_list_;
    /// DBO type
    DB_OBJECT_TYPE dbo_type_;
    /// Current index
    unsigned int index_;
    /// Maximal index for currently allocated size
    unsigned int max_index_;
    /// Maximal index that was set
    unsigned int max_used_index_;
    /// Current container index
    unsigned int index_container_;
    /// Current index in container
    unsigned int index_in_container_;
    /// Size of a container
    unsigned int size_per_container_;
    /// Vector with all PropertyConainers, one for each Property
    std::vector <PropertyContainer *> containers_;
    /// Flag indicating if data any was written
    bool first_write_;
    /// Flag indicating if any PropertyContainers exist
    bool empty_;
    /// Flag indicating if buffer is the last of a DB operation
    bool last_one_;
    /// Vector with void pointers to all values of the current element
    std::vector<void *>* adresses_;

    /// Flag indicating if key search is possible
    bool search_active_;
    /// Key property index, key must be positive and monotonous
    int search_key_pos_;
    /// Key minimum
    int search_key_min_;
    /// Key maximum
    int search_key_max_;

    static unsigned int ids_;

    /// @brief Initialization
    void init();
    /// @brief Deletes and removes containers
    void clear ();
    /// @brief Allocates memory up to index
    void allocateUpTo (unsigned int index);
    /// @brief Additionally allocates one page
    void allocateOnePage ();

    /// @brief Updates key info of containers
    void updateContainerKeyInfo ();

public:
    /// @brief  Returns maximal used index size
    unsigned int getSize ()
    {
        return max_used_index_+1;
    };

    /// @brief Returns current maximal size for current allocation
    unsigned int getMaxSize ()
    {
        return max_index_+1;
    };

    /// @brief Returns flag if all PropertyContainers are full
    bool isFull ();

    /// @brief Returns PropertyList
    PropertyList *getPropertyList ()
    {
        return &member_list_;
    }

    /// @brief Returns DBO type
    DB_OBJECT_TYPE getDBOType () { return dbo_type_; };

    /// @brief Sets DBO type
    void setDBOType (DB_OBJECT_TYPE type) { dbo_type_=type;};
};

#endif /* BUFFER_H_ */
