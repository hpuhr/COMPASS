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
 * CalculateHistogram.h
 *
 *  Created on: May 23, 2012
 *      Author: sk
 */

#ifndef CALCULATEHISTOGRAM_H_
#define CALCULATEHISTOGRAM_H_

#include "Transformation.h"

#include <string>
#include <vector>

class Buffer;
class DBOVariable;

class CalculateHistogram : public Transformation
{
public:
  CalculateHistogram( Buffer *input=NULL );
  CalculateHistogram( std::string class_id,
                      std::string instance_id,
                      Configurable *parent );
  CalculateHistogram( const CalculateHistogram& copy );
  virtual ~CalculateHistogram();

  CalculateHistogram& operator=( const CalculateHistogram& rhs );
  virtual Transformation* clone();
  virtual Transformation* clone( const std::string& class_id,
                                 const std::string& instance_id,
                                 Configurable* parent,
                                 bool assign );

  void setupBins( DBOVariable* var, double min, double max, unsigned int numbins );

protected:
  bool execute();
  void createVariables();

private:
  double min_;
  double max_;
  unsigned int bins_;
  DBOVariable *var_;
  double dbin_;
};

#endif /* CALCULATEHISTOGRAM_H_ */
