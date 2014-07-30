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


#include "CalculateHistogram2D.h"
#include "Buffer.h"
#include "DBOVariable.h"
#include "Logger.h"

#include <stdexcept>


/**
  */
CalculateHistogram2D::CalculateHistogram2D( Buffer *input )
:    Transformation( input ),
     x_var_( NULL ),
     y_var_( NULL )
{
    id_ = "TransformationHistogram2D";

    min_[ 0 ]  = 0.0;
    min_[ 1 ]  = 0.0;
    max_[ 0 ]  = 0.0;
    max_[ 1 ]  = 0.0;
    dbin_[ 0 ] = 0.0;
    dbin_[ 1 ] = 0.0;
    bins_[ 0 ] = 0;
    bins_[ 1 ] = 0;

    createVariables();
}

/**
  */
CalculateHistogram2D::CalculateHistogram2D( std::string class_id,
                                            std::string instance_id,
                                            Configurable *parent )
:    Transformation( class_id, instance_id, parent ),
     x_var_( NULL ),
     y_var_( NULL )
{
    id_ = "TransformationHistogram2D";

    min_[ 0 ]  = 0.0;
    min_[ 1 ]  = 0.0;
    max_[ 0 ]  = 0.0;
    max_[ 1 ]  = 0.0;
    dbin_[ 0 ] = 0.0;
    dbin_[ 1 ] = 0.0;
    bins_[ 0 ] = 0;
    bins_[ 1 ] = 0;

    createVariables();
}

/**
  */
CalculateHistogram2D::CalculateHistogram2D( const CalculateHistogram2D& copy )
{
    *this = copy;
}

/**
  */
CalculateHistogram2D::~CalculateHistogram2D()
{
}

/**
  */
CalculateHistogram2D& CalculateHistogram2D::operator=( const CalculateHistogram2D& rhs )
{
    Transformation::operator=( rhs );

    x_var_     = rhs.x_var_;
    y_var_     = rhs.y_var_;
    min_[ 0 ]  = rhs.min_[ 0 ];
    min_[ 1 ]  = rhs.min_[ 1 ];
    max_[ 0 ]  = rhs.max_[ 0 ];
    max_[ 1 ]  = rhs.max_[ 1 ];
    dbin_[ 0 ] = rhs.dbin_[ 0 ];
    dbin_[ 1 ] = rhs.dbin_[ 1 ];
    bins_[ 0 ] = rhs.bins_[ 0 ];
    bins_[ 1 ] = rhs.bins_[ 1 ];

    return *this;
}

/**
  */
Transformation* CalculateHistogram2D::clone()
{
    return new CalculateHistogram2D( *this );
}


/**
  */
Transformation* CalculateHistogram2D::clone( const std::string& class_id,
                                             const std::string& instance_id,
                                             Configurable* parent,
                                             bool assign )
{
    CalculateHistogram2D* trafo = new CalculateHistogram2D( class_id, instance_id, parent );
    if( assign )
        *trafo = *this;
    return trafo;
}

/**
  */
void CalculateHistogram2D::createVariables()
{
    if( numberInputVariables() == 0 )
    {
        addInputVariable( "x_var" );
        addInputVariable( "y_var" );
    }

    if( numberOutputVariables() == 0 )
    {
        addOutputVariable( "bin_x_id", P_TYPE_INT , "bin_x_id" );
        addOutputVariable( "bin_y_id", P_TYPE_INT , "bin_y_id" );
        addOutputVariable( "bin_cnt" , P_TYPE_UINT, "bin_cnt"  );
    }
}

/**
  */
void CalculateHistogram2D::setXVariable( DBOVariable* var, double min, double max, unsigned int numbins )
{
    if( var )
        getInputVariable( "x_var" )->setProperties( var );
    x_var_ = var;
    min_[ 0 ] = min;
    max_[ 0 ] = max;
    bins_[ 0 ] = numbins;
    dbin_[ 0 ] = ( max_[ 0 ] - min_[ 0 ] ) / (double)bins_[ 0 ];
}

/**
  */
void CalculateHistogram2D::setYVariable( DBOVariable* var, double min, double max, unsigned int numbins )
{
    if( var )
        getInputVariable( "y_var" )->setProperties( var );
    y_var_ = var;
    min_[ 1 ] = min;
    max_[ 1 ] = max;
    bins_[ 1 ] = numbins;
    dbin_[ 1 ] = ( max_[ 1 ] - min_[ 1 ] ) / (double)bins_[ 1 ];
}

/**
  */
bool CalculateHistogram2D::execute ()
{
    if( !x_var_ || !y_var_ )
        throw std::runtime_error( "CalculateHistogram2D: execute: Input variables not set correctly." );

    if( min_[ 0 ] > max_[ 0 ] || min_[ 1 ] > max_[ 1 ] )
        throw std::runtime_error( "CalculateHistogram2D: execute: Invalid minmax information." );

    unsigned int x_ind = getInputVariable( "x_var" )->propertyIndex( input_ );
    unsigned int y_ind = getInputVariable( "y_var" )->propertyIndex( input_ );
    Property* x_prop = getInputVariable( "x_var" )->property( input_ );
    Property* y_prop = getInputVariable( "y_var" )->property( input_ );
    PROPERTY_DATA_TYPE x_data_type = (PROPERTY_DATA_TYPE)x_prop->data_type_int_;
    PROPERTY_DATA_TYPE y_data_type = (PROPERTY_DATA_TYPE)y_prop->data_type_int_;

    input_->setIndex( 0 );

    Bin2D binvec( bins_[ 0 ], Bin1D( bins_[ 1 ], 0 ) );

    double x, y;
    unsigned int i, j, n = input_->getSize();
    int xbin, ybin;
    std::vector<void*>* adresses;
    for( i=0; i<n; ++i )
    {
        if( i != 0 )
            input_->incrementIndex();

        adresses = input_->getAdresses();

        if( isNan( x_prop->data_type_int_, adresses->at( x_ind ) ) ||
            isNan( y_prop->data_type_int_, adresses->at( y_ind ) ) )
            continue;

        if (x_data_type == P_TYPE_BOOL)
            x = *(bool*)adresses->at( x_ind );
        else if (x_data_type == P_TYPE_CHAR)
            x = *(char*)adresses->at( x_ind );
        else if (x_data_type == P_TYPE_INT)
            x = *(int*)adresses->at( x_ind );
        else if (x_data_type == P_TYPE_UCHAR)
            x = *(unsigned char*)adresses->at( x_ind );
        else if (x_data_type == P_TYPE_UINT)
            x = *(unsigned int*)adresses->at( x_ind );
        else if (x_data_type == P_TYPE_STRING)
            throw std::runtime_error( "CalculateHistogram2D::execute(): Cannot handle strings" );
        else if (x_data_type == P_TYPE_FLOAT)
            x = *(float*)adresses->at( x_ind );
        else if (x_data_type == P_TYPE_DOUBLE)
            x = *(double*)adresses->at( x_ind );
        else
            throw std::runtime_error( "CalculateHistogram2D::execute(): Unknown data type" );

        if (y_data_type == P_TYPE_BOOL)
            y = *(bool*)adresses->at( y_ind );
        else if (y_data_type == P_TYPE_CHAR)
            y = *(char*)adresses->at( y_ind );
        else if (y_data_type == P_TYPE_INT)
            y = *(int*)adresses->at( y_ind );
        else if (y_data_type == P_TYPE_UCHAR)
            y = *(unsigned char*)adresses->at( y_ind );
        else if (y_data_type == P_TYPE_UINT)
            y = *(unsigned int*)adresses->at( y_ind );
        else if (y_data_type == P_TYPE_STRING)
            throw std::runtime_error( "CalculateHistogram2D::execute(): Cannot handle strings" );
        else if (y_data_type == P_TYPE_FLOAT)
            y = *(float*)adresses->at( y_ind );
        else if (y_data_type == P_TYPE_DOUBLE)
            y = *(double*)adresses->at( y_ind );
        else
            throw std::runtime_error( "CalculateHistogram2D::execute(): Unknown data type" );

        if( x < min_[ 0 ] || x > max_[ 0 ] )
        {
            loginf  << "CalculateHistogram2D::execute(): X var out of bounds.";
            continue;
        }

        if( y < min_[ 1 ] || y > max_[ 1 ] )
        {
            loginf  << "CalculateHistogram2D::execute(): Y var out of bounds.";
            continue;
        }

        xbin = (int)( ( x - min_[ 0 ] ) / dbin_[ 0 ] );
        ybin = (int)( ( y - min_[ 1 ] ) / dbin_[ 1 ] );
        if( xbin == (signed)bins_[ 0 ] )
            --xbin;
        if( ybin == (signed)bins_[ 1 ] )
            --ybin;

        binvec[ xbin ][ ybin ] += 1;
    }

    unsigned int nlast = bins_[ 0 ] * bins_[ 1 ];
    output_->setIndex( nlast - 1 );
    n = output_->getMaxSize();
    output_->setIndex( 0 );
    for( i=0; i<bins_[ 0 ]; ++i )
    {
        for( j=0; j<bins_[ 1 ]; ++j )
        {
            if( i != 0 || j != 0 )
                output_->incrementIndex();
            adresses = output_->getAdresses();

            *(int*)adresses->at( 0 ) = i;
            *(int*)adresses->at( 1 ) = j;
            *(unsigned int*)adresses->at( 2 ) = binvec[ i ][ j ];
        }
    }

    if( nlast < n )
    {
        output_->setIndex( nlast );
        for( i=nlast; i<n; ++i )
        {
            if( i != nlast )
                output_->incrementIndex();
            adresses = output_->getAdresses();
            *(int*)adresses->at( 0 ) = -1;
        }
    }

    outputReady();

    return true;
}
