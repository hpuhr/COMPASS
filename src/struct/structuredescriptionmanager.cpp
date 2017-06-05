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
 * StructureDescriptionManager.cpp
 *
 *  Created on: Jul 26, 2012
 *      Author: sk
 */

#include "structuredescriptionmanager.h"
//#include "StructureDescriptionPlot.h"
//#include "StructureDescriptionSystemTrack.h"
//#include "StructureDescriptionReferenceTrajectory.h"
//#include "StructureDescriptionADSB.h"
//#include "StructureDescriptionMLAT.h"
//#include "StructureDescriptionSensor.h"
#include "configurationmanager.h"
#include "dbovariable.h"

StructureDescriptionManager::StructureDescriptionManager()
{
  logdbg  << "StructureDescriptionManager: constructor";

//  structure_descriptions_[DBO_PLOTS] = new StructureDescriptionPlot ();
//  structure_descriptions_[DBO_SYSTEM_TRACKS] = new StructureDescriptionSystemTrack ();
//  structure_descriptions_[DBO_REFERENCE_TRAJECTORIES] = new StructureDescriptionReferenceTrajectory ();
//  structure_descriptions_[DBO_ADS_B] = new StructureDescriptionADSB();
//  structure_descriptions_[DBO_MLAT] = new StructureDescriptionMLAT ();
  //structure_descriptions_[DBO_SENSOR_INFORMATION] = new StructureDescriptionSensor ();

  logdbg  << "StructureDescriptionManager: constructor done";
}

StructureDescriptionManager::~StructureDescriptionManager()
{
  //TODO cleanup causes segmentation
  /*
  for (unsigned int cnt=0; cnt < structure_descriptions_.size(); cnt++)
  {
    if (structure_descriptions_.at(cnt) != 0)
    {
      loginf  << "uga at " << cnt << " ptr " << (unsigned long) structure_descriptions_.at(cnt);
      delete structure_descriptions_.at(cnt);
      structure_descriptions_.at(cnt)=0;
    }
  }
  structure_descriptions_.clear();
   */
}

StructureDescription *StructureDescriptionManager::getStructureDescription (const std::string &dbo_type)
{
  assert (structure_descriptions_.find(dbo_type) != structure_descriptions_.end());

  logdbg  << "StructureDescriptionManager: getStructureDescription: type " << dbo_type;

  return structure_descriptions_.at(dbo_type);
}

//std::map <DB_OBJECT_TYPE, ConfigurableDefinition> StructureDescriptionManager::createAllDBObjects ()
//{
//  std::map <DB_OBJECT_TYPE, ConfigurableDefinition> definitions;
//
//  std::map <DB_OBJECT_TYPE, StructureDescription *>::iterator it;
//
//  for (it = structure_descriptions_.begin(); it != structure_descriptions_.end(); it++)
//  {
//    definitions [it->first] = it->second->createDBObject();
//  }
//
//  return definitions;
//}

