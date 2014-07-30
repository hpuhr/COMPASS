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


#ifndef CALCULATEHISTOGRAM2D_H
#define CALCULATEHISTOGRAM2D_H

#include <string>
#include <vector>
#include "Transformation.h"

class Buffer;
class DBOVariable;


class CalculateHistogram2D : public Transformation
{
public:
    typedef std::vector<unsigned int> Bin1D;
    typedef std::vector<Bin1D> Bin2D;

    CalculateHistogram2D( Buffer *input=NULL );
    CalculateHistogram2D( std::string class_id,
                          std::string instance_id,
                          Configurable *parent );
    CalculateHistogram2D( const CalculateHistogram2D& copy );
    virtual ~CalculateHistogram2D();

    CalculateHistogram2D& operator=( const CalculateHistogram2D& rhs );
    virtual Transformation* clone();
    virtual Transformation* clone( const std::string& class_id,
                                   const std::string& instance_id,
                                   Configurable* parent,
                                   bool assign );

    void setXVariable( DBOVariable* var, double min, double max, unsigned int numbins );
    void setYVariable( DBOVariable* var, double min, double max, unsigned int numbins );

private:
    bool execute();
    void createVariables();

    double min_[ 2 ];
    double max_[ 2 ];
    double dbin_[ 2 ];
    unsigned int bins_[ 2 ];

    DBOVariable* x_var_;
    DBOVariable* y_var_;
};


#endif /* CALCULATEHISTOGRAM2D_H */
