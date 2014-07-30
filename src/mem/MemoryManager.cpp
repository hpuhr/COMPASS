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
 * MemoryManager.cpp
 *
 *  Created on: Dec 6, 2011
 *      Author: sk
 */

#ifdef Q_WS_X11
  #include <unistd.h>
  #include <ios>
#endif

#include <iostream>
#include <fstream>
#include <string>

#include "MemoryManager.h"
#include "Config.h"
#include "Logger.h"

/**
 * Registers parameters, creates managers and sets constructed_.
 */
MemoryManager::MemoryManager()
 : Configurable ("MemoryManager", "MemoryManager0")
{
  registerParameter ("buffer_size", &size_, 10000);
  registerParameter ("max_mem_mb", &max_mem_mb_, 6000.0);

  createManagers ();
  constructed_=true;
}

/**
 * Deletes managers and sets constructed to false.
 */
MemoryManager::~MemoryManager()
{
	deleteManagers ();
	constructed_=false;
}

ArrayTemplateManagerBase *MemoryManager::getArrayTemplateManager (PROPERTY_DATA_TYPE type)
{
  assert (managers_.size() != 0);
	assert (type >= 0);
	assert (type < managers_.size());

	return managers_.at(type)->getManager();
}

/**
 * Creates all managers for all values of PROPERTY_DATA_TYPE
 */
void MemoryManager::createManagers ()
{
	for (unsigned int cnt=0; cnt < P_TYPE_SENTINEL; cnt++)
	{
		ArrayManagerBase *tmp = new ArrayManagerBase ((PROPERTY_DATA_TYPE) cnt, size_);
		managers_.push_back (tmp);
		base_sizes_.push_back (tmp->getBaseSizeInBytes());
	}
}
void MemoryManager::deleteManagers ()
{
	assert (managers_.size() == P_TYPE_SENTINEL);
	for (unsigned int cnt=0; cnt < P_TYPE_SENTINEL; cnt++)
	{
		delete managers_.at(cnt);
	}
	managers_.clear();
}

unsigned int MemoryManager::getBaseSizesInBytes (unsigned int i)
{
  //loginf  << "MemoryManager: getBaseSizesInBytes: i " << i << " size " << base_sizes_.size();
  assert (i < base_sizes_.size());
  return base_sizes_.at(i);
};

/**
 * Takes two doubles by reference, attempts to read the system-dependent data for a process' virtual memory
 *  size and resident set size, and returns the results in MB.
 *
 *  On failure, returns 0.0, 0.0. Does only work under linux.
 */

void MemoryManager::processMemUsage(double& vm_usage, double& resident_set)
{
//#ifdef Q_WS_X11
   using std::ios_base;
   using std::ifstream;
   using std::string;

   vm_usage     = 0.0;
   resident_set = 0.0;

   // 'file' stat seems to give the most reliable results
   //
   ifstream stat_stream("/proc/self/stat",ios_base::in);

   // dummy vars for leading entries in stat that we don't care about
   //
   string pid, comm, state, ppid, pgrp, session, tty_nr;
   string tpgid, flags, minflt, cminflt, majflt, cmajflt;
   string utime, stime, cutime, cstime, priority, nice;
   string O, itrealvalue, starttime;

   // the two fields we want
   //
   unsigned long vsize;
   long rss;

   stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
               >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
               >> utime >> stime >> cutime >> cstime >> priority >> nice
               >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

   stat_stream.close();

   long page_size_kb = sysconf(_SC_PAGE_SIZE) / (1024); // in case x86-64 is configured to use 2MB pages
   vm_usage     = vsize / (1024.0*1024.0);
   resident_set = ((double)rss * page_size_kb)/1024.0;

//#endif

}

bool MemoryManager::getMaxMemUsed ()
{
	double vm_usage, resident_set;
	processMemUsage(vm_usage, resident_set);

	return (resident_set > max_mem_mb_);
}

void MemoryManager::printMemoryInfo ()
{
	double vm, res;
	MemoryManager::getInstance().processMemUsage (vm, res);

	unsigned int bytes_used=0;
	unsigned int bytes_free=0;

	for (unsigned int cnt=0; cnt < P_TYPE_SENTINEL; cnt++)
	{
		ArrayManagerBase *tmp = managers_.at(cnt);
		unsigned int used = tmp->getManager()->getUsedPages ();
		unsigned int free = tmp->getManager()->getFreePages ();
		//loginf  << "MemoryManager: printMemoryInfo: Pages type " << cnt << " bytes " << tmp->getManager()->getBaseSizeInBytes() << ": used " << used << " free " << free ;
		bytes_used += tmp->getManager()->getBaseSizeInBytes()*used*size_;
		bytes_free += tmp->getManager()->getBaseSizeInBytes()*free*size_;
	}

	loginf << "MemoryManager: printMemoryInfo: memory used vm " << vm << " resident " << res
			<< " used in pages " << (float) bytes_used/(1024.0*1024.0) << " free " << (float)bytes_free/(1024.0*1024.0);
}

