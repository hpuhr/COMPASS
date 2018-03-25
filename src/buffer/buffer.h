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

#ifndef BUFFER_H_
#define BUFFER_H_

#include <unordered_map>
#include <vector>
#include "propertylist.h"
#include "arraylist.h"

class DBOVariableSet;

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
    Buffer(PropertyList properties, const std::string &dbo_name="");
    /// @brief Desctructor.
    virtual ~Buffer();

    /// @brief Set addresses to point to values of element at index.

    /// @brief Adds all containers of org_buffer and removes them from org_buffer.
    void seizeBuffer (Buffer &org_buffer);

    /// @brief Adds an additional property.
    void addProperty (std::string id, PropertyDataType type);
    void addProperty (const Property &property);

    /// @brief Returns boolean indicating if any data was ever written.
    bool firstWrite ();

    /// @brief Returns boolean indicating if buffer is the last of one DB operation.
    bool lastOne() { return last_one_;}
    /// @brief Sets if buffer is the last one of one DB operation.
    void lastOne (bool last_one) { last_one_=last_one; }

    /// @brief Returns flag indicating if buffer is filled to a multiple of BUFFER_ARRAY_SIZE.
    bool full ();

    /// @brief Returns the buffers id
    unsigned int id() const { return id_; }

    bool hasBool (const std::string &id);
    bool hasChar (const std::string id);
    bool hasUChar (const std::string &id);
    bool hasInt (const std::string &id);
    bool hasUInt (const std::string &id);
    bool hasLongInt (const std::string &id);
    bool hasULongInt (const std::string &id);
    bool hasFloat (const std::string &id);
    bool hasDouble (const std::string &id);
    bool hasString (const std::string &id);

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

    void renameBool (const std::string &id, const std::string &id_new);
    void renameChar (const std::string &id, const std::string &id_new);
    void renameUChar (const std::string &id, const std::string &id_new);
    void renameInt (const std::string &id, const std::string &id_new);
    void renameUInt (const std::string &id, const std::string &id_new);
    void renameLongInt (const std::string &id, const std::string &id_new);
    void renameULongInt (const std::string &id, const std::string &id_new);
    void renameFloat (const std::string &id, const std::string &id_new);
    void renameDouble (const std::string &id, const std::string &id_new);
    void renameString (const std::string &id, const std::string &id_new);

    /// @brief  Returns maximal used index size
    const size_t size ();

    /// @brief Returns PropertyList
    const PropertyList &properties ()
    {
        return properties_;
    }

    /// @brief Returns DBO type
    const std::string &dboName () { return dbo_name_; }

    /// @brief Sets DBO type
    void dboName (const std::string &dbo_name) { dbo_name_=dbo_name;}

    bool isNone (const Property& property, unsigned int row_cnt);

    void transformVariables (DBOVariableSet& list, bool tc2dbovar); // tc2dbovar true for db->dbo, false dbo->db

protected:
    /// Unique buffer id, copied when getting shallow copies
    unsigned int id_;
    /// List of all properties
    PropertyList properties_;
    /// DBO type
    std::string dbo_name_;

    /// Vector with all ArrayList
    //std::vector <std::shared_ptr<ArrayListBase>> arrays_;
    /// Map with all ArrayListTemplates, one for each Property type, identified by
    std::map <std::string, std::shared_ptr<ArrayListTemplate<bool>>> arrays_bool_;
    std::map <std::string, std::shared_ptr<ArrayListTemplate<char>>> arrays_char_;
    std::map <std::string, std::shared_ptr<ArrayListTemplate<unsigned char>>> arrays_uchar_;
    std::map <std::string, std::shared_ptr<ArrayListTemplate<int>>> arrays_int_;
    std::map <std::string, std::shared_ptr<ArrayListTemplate<unsigned int>>> arrays_uint_;
    std::map <std::string, std::shared_ptr<ArrayListTemplate<long int>>> arrays_long_int_;
    std::map <std::string, std::shared_ptr<ArrayListTemplate<unsigned long int>>> arrays_ulong_int_;
    std::map <std::string, std::shared_ptr<ArrayListTemplate<float>>> arrays_float_;
    std::map <std::string, std::shared_ptr<ArrayListTemplate<double>>> arrays_double_;
    std::map <std::string, std::shared_ptr<ArrayListTemplate<std::string>>> arrays_string_;

    /// Flag indicating if buffer is the last of a DB operation
    bool last_one_;

    static unsigned int ids_;
};

#endif /* BUFFER_H_ */
