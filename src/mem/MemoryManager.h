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
 * MemoryManager.h
 *
 *  Created on: Dec 6, 2011
 *      Author: sk
 */

#ifndef MEMORYMANAGER_H_
#define MEMORYMANAGER_H_

#include "Singleton.h"
#include "Global.h"
#include "ArrayTemplateManager.h"
#include "Configurable.h"

/**
 * @brief Main access point for memory allocation and management
 *
 * Holds ArrayTemplateManagers of different data types. Does not wrap their functionality but forwards the pointers to the
 * ArrayTemplateManagers themselves, which manage pages (ArrayTemplate) of the different data types.
 *
 */
class MemoryManager : public Singleton, public Configurable
{
private:
    /// Constructed flag
    bool constructed_;
    /// Number of elements in page
    unsigned int size_;
    /// Container with all ArrayManagerBase managers
    std::vector <ArrayManagerBase *> managers_;
    /// Value of maximal allocated memory
    double max_mem_mb_;
    /// Container with the data type base sizes
    std::vector <unsigned int> base_sizes_;

    /// @brief Creates the managers
    void createManagers ();
    /// @brief Deletes the managers
    void deleteManagers ();

    /// @brief Constructor
    MemoryManager ();

public:
    /// @brief Destructor
    virtual ~MemoryManager ();

    /// @brief Returns manager of type
    ArrayTemplateManagerBase *getArrayTemplateManager (PROPERTY_DATA_TYPE type);

    /// @brief Prints memory information for benchmarking purposes
    void printMemoryInfo ();

    /// @brief Returns singleton instance
    static MemoryManager& getInstance()
    {
        static MemoryManager instance;
        return instance;
    }

    /// @brief Returns number of elements per page
    unsigned int getSize ()
    {
        return size_;
    };

    /// @brief Returns container with data type base sizes
    std::vector <unsigned int> getBaseSizesInBytes ()
	        {
        return base_sizes_;
	        };

    /// @brief Returns base size for type data i
    unsigned int getBaseSizesInBytes (unsigned int i);

    /// @brief Writes virtual memory size and resident memory size into argument references (in MB)
    void processMemUsage(double& vm_usage, double& resident_set);

    /// @brief Returns if maximal memory limit is reached
    bool getMaxMemUsed ();

    /// @brief Returns constructed flag
    bool getConstructed () { return constructed_; };
};

#endif /* MEMORYMANAGER_H_ */
