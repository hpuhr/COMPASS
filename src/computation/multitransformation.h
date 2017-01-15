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
 * MultiTransformation.h
 *
 *  Created on: Feb 3, 2013
 *      Author: sk
 */

#ifndef MULTITRANSFORMATION_H_
#define MULTITRANSFORMATION_H_

#include "Transformation.h"

/**
 * @brief Base class for executing multiple transformations
 *
 * This class allows adding a number of transformations, which are all executed when execute is called.
 */
class MultiTransformation : public Transformation
{
public:
  /// @brief Constructor
  MultiTransformation( bool delete_trafos, std::string id, Buffer *input=NULL, bool sustainable=false );
  /// @brief Copy constructor
  MultiTransformation( const MultiTransformation& copy );
  /// @brief Desctructor
  virtual ~MultiTransformation();

  /// @brief Adds a transformation
  void addTransformation( Transformation* trafo );


  /// @brief Cloning
  virtual Transformation* clone();

protected:
  /// @brief Executes all added transformatins
  virtual bool execute ();
  virtual void createVariables();

  /// Container with all added Transformations
  std::vector<Transformation*> trafos_;
  /// Delete Transformations flag
  bool delete_;
};

#endif /* MULTITRANSFORMATION_H_ */
