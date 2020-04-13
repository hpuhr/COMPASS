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

#ifndef TRANSFORMATIONJOB_H_
#define TRANSFORMATIONJOB_H_

#include "Job.h"

class Transformation;

/**
 * @brief Job specialization for generic transformations
 *
 * Uses a Transformation as encapsulation for a working package.
 */
class TransformationJob : public Job
{
  public:
    // @brief Constructor
    TransformationJob(JobOrderer* orderer, boost::function<void(Job*)> done_function,
                      boost::function<void(Job*)> obsolete_function,
                      Transformation* transformation);
    // @brief Desctructor
    virtual ~TransformationJob();

    // @brief Main operation function
    virtual void execute();

    // @brief Returns held Transformation
    Transformation* getTransformation();

  protected:
    /// Encapsulation of the working package
    Transformation* transformation_;
};

#endif /* TRANSFORMATIONJOB_H_ */
