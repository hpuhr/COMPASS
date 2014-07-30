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
 * CalculatePosition2D.h
 *
 *  Created on: Apr 24, 2012
 *      Author: sk
 */

#ifndef CALCULATEPOSITION2D_H_
#define CALCULATEPOSITION2D_H_

#include "Transformation.h"

class DBOVariable;

#include <string>

class CalculatePosition2D : public Transformation
{
public:
    CalculatePosition2D( Buffer* input=NULL );
    CalculatePosition2D( std::string class_id,
                         std::string instance_id,
                         Configurable *parent );
    CalculatePosition2D( const CalculatePosition2D& copy );
    virtual ~CalculatePosition2D();

    CalculatePosition2D& operator=( const CalculatePosition2D& rhs );
    virtual Transformation* clone();
    virtual Transformation* clone( const std::string& class_id,
                                   const std::string& instance_id,
                                   Configurable* parent,
                                   bool assign );

    double getMinX() const { return min_[ 0 ]; }
    double getMaxX() const { return min_[ 1 ]; }
    double getMinY() const { return max_[ 0 ]; }
    double getMaxY() const { return max_[ 1 ]; }

    void setVariables( DBOVariable* x_var,
                       DBOVariable* y_var,
                       double min_x,
                       double max_x,
                       double min_y,
                       double max_y );
    void setColorId( unsigned int id );

protected:
    bool execute();
    void createVariables();

private:
    DBOVariable *x_var_;
    DBOVariable *y_var_;
    double min_[ 2 ];
    double max_[ 2 ];
    unsigned int col_;
};

#endif /* CALCULATEPOSITION_H_ */
