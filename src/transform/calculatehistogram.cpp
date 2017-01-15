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
 * CalculateHistogram.cpp
 *
 *  Created on: May 23, 2012
 *      Author: sk
 */

#include "CalculateHistogram.h"
#include "Buffer.h"
#include "DBOVariable.h"
#include "Logger.h"

#include <stdexcept>


/**
  */
CalculateHistogram::CalculateHistogram( Buffer *input )
:    Transformation( input ),
     min_( 0.0 ),
     max_( 1.0 ),
     bins_( 0 ),
     var_( NULL ),
     dbin_( 0.0 )
{
    id_ = "TransformationHistogram";

    createVariables();
}

/**
  */
CalculateHistogram::CalculateHistogram( std::string class_id,
                                        std::string instance_id,
                                        Configurable *parent )
:    Transformation( class_id, instance_id, parent ),
     min_( 0.0 ),
     max_( 1.0 ),
     bins_( 0 ),
     var_( NULL ),
     dbin_( 0.0 )
{
    id_ = "TransformationHistogram";

    createVariables();
}

/**
  */
CalculateHistogram::CalculateHistogram( const CalculateHistogram& copy )
{
    *this = copy;
}

/**
  */
CalculateHistogram::~CalculateHistogram()
{
}

/**
  */
CalculateHistogram& CalculateHistogram::operator=( const CalculateHistogram& rhs )
{
    Transformation::operator=( rhs );

    min_  = rhs.min_;
    max_  = rhs.max_;
    var_  = rhs.var_;
    bins_ = rhs.bins_;
    dbin_ = rhs.dbin_;

    return *this;
}

/**
  */
Transformation* CalculateHistogram::clone()
{
    return new CalculateHistogram( *this );
}


/**
  */
Transformation* CalculateHistogram::clone( const std::string& class_id,
                                           const std::string& instance_id,
                                           Configurable* parent,
                                           bool assign )
{
    CalculateHistogram* trafo = new CalculateHistogram( class_id, instance_id, parent );
    if( assign )
        *trafo = *this;
    return trafo;
}

/**
  */
void CalculateHistogram::createVariables()
{
    if( numberInputVariables() == 0 )
        addInputVariable( "var" );
    if( numberOutputVariables() == 0 )
        addOutputVariable( "bin_id", P_TYPE_INT, "bin_id" );
}

/**
  */
void CalculateHistogram::setupBins( DBOVariable* var, double min, double max, unsigned int numbins )
{
    if( var )
        getInputVariable( "var" )->setProperties( var );
    var_ = var;
    min_ = min;
    max_ = max;
    bins_ = numbins;
    dbin_ = ( max_ - min_ ) / (double)bins_;
}

/**
  */
bool CalculateHistogram::execute ()
{
    if( !var_ )
        throw std::runtime_error( "CalculateHistogram: execute: input variable not set correctly" );

    if( min_ > max_ )
        throw std::runtime_error( "CalculateHistogram: execute: invalid minmax information" );

    Property* ind_prop = getInputVariable( "var" )->property( input_ );
    int ind = getInputVariable( "var" )->propertyIndex( input_ );
    PROPERTY_DATA_TYPE data_type = (PROPERTY_DATA_TYPE)ind_prop->data_type_int_;

    unsigned int bin_ind = getOutputVariable( "bin_id" )->propertyIndex( output_ );

    input_->setIndex( 0 );

    int bin;
    double value;
    unsigned int i, n = input_->getSize();
    std::vector<void*>* adresses;
    for( i=0; i<n; ++i )
    {
        if( i != 0 )
            input_->incrementIndex();

        adresses = input_->getAdresses();

        if( isNan( ind_prop->data_type_int_, adresses->at( ind ) ) )
        {
            *(int*)adresses->at( bin_ind ) = -1;
            continue;
        }

        if (data_type == P_TYPE_BOOL)
            value = *(bool*)adresses->at( ind );
        else if (data_type == P_TYPE_CHAR)
            value = *(char*)adresses->at( ind );
        else if (data_type == P_TYPE_INT)
            value = *(int*)adresses->at( ind );
        else if (data_type == P_TYPE_UCHAR)
            value = *(unsigned char*)adresses->at( ind );
        else if (data_type == P_TYPE_UINT)
            value = *(unsigned int*)adresses->at( ind );
        else if (data_type == P_TYPE_STRING)
            throw std::runtime_error( "CalculateHistogram: execute: cannot handle strings" );
        else if (data_type == P_TYPE_FLOAT)
            value = *(float*)adresses->at( ind );
        else if (data_type == P_TYPE_DOUBLE)
            value = *(double*)adresses->at( ind );
        else
            throw std::runtime_error( "CalculateHistogram: execute: unknown data type" );

        if( value < min_ || value > max_ )
        {
            //loginf  << "CalculateHistogram::execute(): Out of bounds.";
            bin = -1;
        }
        else
        {
            bin = (int)( ( value - min_ ) / dbin_ );
            if( bin == (signed)bins_ )
                --bin;
        }

        *(int*)adresses->at( bin_ind ) = bin;
    }

    outputReady();

    return true;
}
