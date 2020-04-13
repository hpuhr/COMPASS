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

#ifndef STRUCTUREREADER_H_
#define STRUCTUREREADER_H_

#include "joborderer.h"

class Buffer;
class BufferSet;
class DBInterface;
class StructureDescription;
class PropertyList;

/**
 * @brief Generates data buffers from C structs
 *
 * Uses the defined StructureDescriptions from StructureDescriptionManager, and generates Buffers
 * for the data contents. When add is called, the struct is read using the definition and the data
 * is added to the according buffer.
 */
class StructureReader : public JobOrderer
{
  public:
    /// @brief Constructor
    StructureReader(DBInterface* db_interface);
    /// @brief Destructor
    virtual ~StructureReader();

    /// @brief Adds a C struct located at the supplied address
    void add(DB_OBJECT_TYPE type, void* data);

    bool hasUnwrittenData();

    /// @brief Moves buffers from data_ to data_set_
    void finalize();

    void writeBufferDone(Job* job);
    void jobAborted(Job* job);

  protected:
    DBInterface* db_interface_;
    /// Container with temporary buffers
    std::map<DB_OBJECT_TYPE, Buffer*> data_;
    /// Container with data descriptions for the buffers
    std::map<DB_OBJECT_TYPE, PropertyList*> property_lists_;
    /// Container with the structure descriptions
    std::map<DB_OBJECT_TYPE, StructureDescription*> descriptions_;
    /// Container with mutexes for multi-thread access
    std::map<DB_OBJECT_TYPE, boost::mutex*> mutexes_;

    /// @brief Creates a new buffer for DBO type
    Buffer* getNewBuffer(DB_OBJECT_TYPE type);

    void writeBuffer(Buffer* buffer);
};

#endif /* STRUCTUREREADER_H_ */
