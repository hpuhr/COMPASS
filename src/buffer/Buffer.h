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
#include "ArrayList.h"

//class PropertyContainer;

/**
 * @brief Fast, dynamic data container
 *
 * Encapsulates general data storage with access functions with maximum efficiency.
 * Performs basic checks, allocates new PropertyContainers when space is needed, but NOT thread-safe.
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
    /// @brief Default constructor.
    Buffer ();
    /// @brief Constructor.
    Buffer(PropertyList properties, std::string db_type="");
    /// @brief Desctructor.
    virtual ~Buffer();

    /// @brief Set addresses to point to values of element at index.

    /// @brief Adds all containers of org_buffer and removes them from org_buffer.
    //void seizeBuffer (Buffer &org_buffer);

    /// @brief Adds an additional property.
    void addProperty (std::string id, PropertyDataType type);

    /// @brief Print function for debugging.
//    void print (unsigned int num_elements);
//    void printLong (unsigned int num_elements);

    /// @brief Returns boolean indicating if any data was ever written.
    bool firstWrite ();
    /// @brief Sets data was written flag.
    //void setWritten () { first_write_=false; };

    /// @brief Returns boolean indicating if buffer is the last of one DB operation.
    //bool getLastOne () { return last_one_;};
    /// @brief Sets if buffer is the last one of one DB operation.
    //void setLastOne (bool last_one) { last_one_=last_one; };

    /// @brief Returns buffer containing the data of this buffer and re-initializes.
    //Buffer *transferData ();

    /// @brief Returns a shallow copy of the this buffer.
    //Buffer *getShallowCopy ();

    /// @brief Activates key search on given property index.
    //void activateKeySearch (unsigned int key_pos);

    /// @brief Sets index that where key matches, returns true if found, else false.
    //bool setIndexForKey (unsigned int key);

    /// @brief Sets all values of a property to NaN.
    //void setPropertyValuesNan (unsigned int property_index);

    /// @brief Returns flag indicating if buffer is filled to a multiple of BUFFER_ARRAY_SIZE.
    //bool isFilled ();

    /// @brief Returns the buffers id
    unsigned int getId() const { return id_; }

    /// Unique buffer id, copied when getting shallow copies
    unsigned int id_;

    /// Copies current index from source buffer into this one. PropertyLists must have the same indices
    //void deepCopyRecordFrom (Buffer *src);

private:
    /// List of all properties
    PropertyList properties_;
    /// DBO type
    std::string dbo_type_;

    /// Maximal index for currently allocated size
    //unsigned int num_properties_index_;

    /// Vector with all ArrayList
    std::vector <ArrayListBase *> arrays_;
    /// Map with all ArrayListTemplates, one for each Property type, identified by
    std::map <std::string, ArrayListTemplate<bool> > arrays_bool_;
    std::map <std::string, ArrayListTemplate<char> > arrays_char_;
    std::map <std::string, ArrayListTemplate<unsigned char> > arrays_uchar_;
    std::map <std::string, ArrayListTemplate<int> > arrays_int_;
    std::map <std::string, ArrayListTemplate<unsigned int> > arrays_uint_;
    std::map <std::string, ArrayListTemplate<long int> > arrays_long_int_;
    std::map <std::string, ArrayListTemplate<unsigned long int> > arrays_ulong_int_;
    std::map <std::string, ArrayListTemplate<float> > arrays_float_;
    std::map <std::string, ArrayListTemplate<double> > arrays_double_;
    std::map <std::string, ArrayListTemplate<std::string> > arrays_string_;
    /// Flag indicating if data any was written
    //bool first_write_;

    /// Flag indicating if buffer is the last of a DB operation
    //bool last_one_;

    /// Flag indicating if key search is possible
//    bool search_active_;
    /// Key property index, key must be positive and monotonous
//    int search_key_pos_;
    /// Key minimum
//    int search_key_min_;
    /// Key maximum
//    int search_key_max_;

    static unsigned int ids_;

    /// @brief Initialization
    //void init();
    /// @brief Allocates memory up to index
    //void allocateUpTo (unsigned int index);

public:
    ArrayListTemplate<bool> &getBool (const std::string &id);
    ArrayListTemplate<char> &getChar (const std::string id);
    ArrayListTemplate<unsigned char> &getUChar (const std::string &id);
    ArrayListTemplate<int> &getInt (const std::string &id);
    ArrayListTemplate<unsigned int> &getUInt (const std::string &id);
    ArrayListTemplate<long int> &getLongInt (const std::string &id);
    ArrayListTemplate<unsigned long int> &getULongInt (const std::string &id);
    ArrayListTemplate<float> &getFloat (const std::string &id);
    ArrayListTemplate<double> &getDouble (const std::string &id);
    ArrayListTemplate<std::string> &getString (const std::string &id);

    /// @brief  Returns maximal used index size
    const size_t size ();


    /// @brief Returns current maximal size for current allocation
//    const size_t maxSize ()
//    {
//        if (containers_.size() == 0)
//            return 0;

//        return containers_.front()->maxSize();
//    };

    /// @brief Returns PropertyList
    const PropertyList &properties ()
    {
        return properties_;
    }

    /// @brief Returns DBO type
    const std::string dboType () { return dbo_type_; };

    /// @brief Sets DBO type
    void setDBOType (const std::string &dbo_type) { dbo_type_=dbo_type;};
};

#endif /* BUFFER_H_ */
