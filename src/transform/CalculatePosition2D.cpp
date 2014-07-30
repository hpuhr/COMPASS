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
 * CalculatePosition2D2D.cpp
 *
 *  Created on: Apr 24, 2012
 *      Author: sk
 */

#include "CalculatePosition2D.h"
#include "PropertyList.h"
#include "Buffer.h"
#include "DBOVariable.h"

#define _USE_MATH_DEFINES
#include <math.h>


/**
  */
CalculatePosition2D::CalculatePosition2D( Buffer *input )
:   Transformation( input ),
    x_var_( NULL ),
    y_var_( NULL ),
    col_( 0 )
{
    id_ = "TransformationPosition2D";

    min_[ 0 ] = 0.0;
    min_[ 1 ] = 0.0;
    max_[ 0 ] = 0.0;
    max_[ 1 ] = 0.0;

    createVariables();
}

/**
  */
CalculatePosition2D::CalculatePosition2D( std::string class_id,
                                          std::string instance_id,
                                          Configurable *parent )
:   Transformation( class_id, instance_id, parent ),
    x_var_( NULL ),
    y_var_( NULL ),
    col_( 0 )
{
    id_ = "TransformPosition2D";

    min_[ 0 ] = 0.0;
    min_[ 1 ] = 0.0;
    max_[ 0 ] = 0.0;
    max_[ 1 ] = 0.0;

    createVariables();
}

/**
  */
CalculatePosition2D::CalculatePosition2D( const CalculatePosition2D& copy )
{
    *this = copy;
}

/**
  */
CalculatePosition2D::~CalculatePosition2D()
{
}


/**
  */
CalculatePosition2D& CalculatePosition2D::operator=( const CalculatePosition2D& rhs )
{
    Transformation::operator=( rhs );

    x_var_    = rhs.x_var_;
    y_var_    = rhs.y_var_;
    col_      = rhs.col_;
    min_[ 0 ] = rhs.min_[ 0 ];
    min_[ 1 ] = rhs.min_[ 1 ];
    max_[ 0 ] = rhs.max_[ 0 ];
    max_[ 1 ] = rhs.max_[ 1 ];

    return *this;
}

/**
  */
Transformation* CalculatePosition2D::clone()
{
    return new CalculatePosition2D( *this );
}

/**
  */
Transformation* CalculatePosition2D::clone( const std::string& class_id,
                                            const std::string& instance_id,
                                            Configurable* parent,
                                            bool assign )
{
    CalculatePosition2D* trafo = new CalculatePosition2D( class_id, instance_id, parent );
    if( assign )
        *trafo = *this;
    return trafo;
}

/**
  */
void CalculatePosition2D::createVariables()
{
    if( numberInputVariables() == 0 )
    {
        addInputVariable( "x_var" );
        addInputVariable( "y_var" );
    }

    if( numberOutputVariables() == 0 )
    {
        addOutputVariable( "pos_x", P_TYPE_FLOAT, "ogre_pos_x" );
        addOutputVariable( "pos_y", P_TYPE_FLOAT, "ogre_pos_y" );
        addOutputVariable( "pos_z", P_TYPE_FLOAT, "ogre_pos_z" );
        addOutputVariable( "color", P_TYPE_UINT, "ogre_color" );
    }
}

/**
  */
void CalculatePosition2D::setVariables( DBOVariable* x_var,
                                        DBOVariable* y_var,
                                        double min_x,
                                        double max_x,
                                        double min_y,
                                        double max_y )
{
    x_var_ = x_var;
    y_var_ = y_var;
    min_[ 0 ] = min_x;
    min_[ 1 ] = min_y;
    max_[ 0 ] = max_x;
    max_[ 1 ] = max_y;

    if( x_var )
        getInputVariable( "x_var" )->setProperties( x_var );
    if( y_var )
        getInputVariable( "y_var" )->setProperties( y_var );
}

/**
  */
void CalculatePosition2D::setColorId( unsigned int id )
{
    col_ = id;
}

/**
  */
bool CalculatePosition2D::execute ()
{
    if( !x_var_ || !y_var_ )
        throw std::runtime_error( "CalculatePosition2D: execute: Input variables not set correctly." );

    if( min_[ 0 ] > max_[ 0 ] || min_[ 1 ] > max_[ 1 ] )
        throw std::runtime_error( "CalculatePosition2D: execute: Invalid minmax information." );

    unsigned int x_ind = getInputVariable( "x_var" )->propertyIndex( input_ );
    unsigned int y_ind = getInputVariable( "y_var" )->propertyIndex( input_ );
    Property* x_prop = input_->getPropertyList()->getProperty( x_ind );
    Property* y_prop = input_->getPropertyList()->getProperty( y_ind );
    PROPERTY_DATA_TYPE x_data_type = (PROPERTY_DATA_TYPE)input_->getPropertyList()->getProperty( x_ind )->data_type_int_;
    PROPERTY_DATA_TYPE y_data_type = (PROPERTY_DATA_TYPE)input_->getPropertyList()->getProperty( y_ind )->data_type_int_;
    double w_x = max_[ 0 ] - min_[ 0 ];
    double w_y = max_[ 1 ] - min_[ 1 ];

    unsigned int ogre_pos_x_ind = getOutputVariable( "pos_x" )->propertyIndex( output_ );
    unsigned int ogre_pos_y_ind = getOutputVariable( "pos_y" )->propertyIndex( output_ );
    unsigned int ogre_pos_z_ind = getOutputVariable( "pos_z" )->propertyIndex( output_ );
    unsigned int ogre_col_ind   = getOutputVariable( "color" )->propertyIndex( output_ );
    Property* ogre_pos_x_prop = output_->getPropertyList()->getProperty( ogre_pos_x_ind );
    Property* ogre_pos_z_prop = output_->getPropertyList()->getProperty( ogre_pos_z_ind );

    if( append_ )
    {
        input_->setIndex( 0 );
    }
    else
    {
        input_->setIndex( 0 );
        output_->setIndex( 0 );
    }

    double x_value;
    double y_value;
    std::vector<void*>* input_adresses;
    std::vector<void*>* output_adresses;
    unsigned int i, n = input_->getSize();
    for( i=0; i<n; ++i )
    {
        if( i != 0 )
        {
            if (append_)
            {
                input_->incrementIndex();
            }
            else
            {
                input_->incrementIndex();
                output_->incrementIndex();
            }
        }

        input_adresses = input_->getAdresses();
        output_adresses = output_->getAdresses();

        //x
        if( isNan( x_prop->data_type_int_, input_adresses->at( x_ind ) ) )
        {
            setNan( ogre_pos_x_prop->data_type_int_, output_adresses->at( ogre_pos_x_ind ) );
        }
        else
        {
            if( x_data_type == P_TYPE_BOOL )
                x_value = *(bool*)input_adresses->at( x_ind );
            else if( x_data_type == P_TYPE_CHAR )
                x_value = *(char*)input_adresses->at( x_ind );
            else if( x_data_type == P_TYPE_INT )
                x_value = *(int*)input_adresses->at( x_ind );
            else if( x_data_type == P_TYPE_UCHAR )
                x_value = *(unsigned char*)input_adresses->at( x_ind );
            else if( x_data_type == P_TYPE_UINT )
                x_value = *(unsigned int*)input_adresses->at( x_ind );
            else if( x_data_type == P_TYPE_STRING )
                throw std::runtime_error( "CalculatePosition2D::execute(): Cannot handle strings" );
            else if( x_data_type == P_TYPE_FLOAT )
                x_value = *(float*)input_adresses->at( x_ind );
            else if( x_data_type == P_TYPE_DOUBLE )
                x_value = *(double*)input_adresses->at( x_ind );

            *(float*)output_adresses->at( ogre_pos_x_ind ) = ( x_value - min_[ 0 ] ) / w_x;
        }

        //y
        if( isNan( y_prop->data_type_int_, input_adresses->at( y_ind ) ) )
        {
            setNan( ogre_pos_z_prop->data_type_int_, output_adresses->at( ogre_pos_z_ind ) );
        }
        else
        {
            if( y_data_type == P_TYPE_BOOL )
                y_value = *(bool*)input_adresses->at( y_ind );
            else if( y_data_type == P_TYPE_CHAR )
                y_value = *(char*)input_adresses->at( y_ind );
            else if( y_data_type == P_TYPE_INT )
                y_value = *(int*)input_adresses->at( y_ind );
            else if( y_data_type == P_TYPE_UCHAR )
                y_value = *(unsigned char*)input_adresses->at( y_ind );
            else if( y_data_type == P_TYPE_UINT )
                y_value = *(unsigned int*)input_adresses->at( y_ind );
            else if( y_data_type == P_TYPE_STRING )
                throw std::runtime_error( "CalculatePosition2D::execute(): Cannot handle strings" );
            else if( y_data_type == P_TYPE_FLOAT )
                y_value = *(float*)input_adresses->at( y_ind );
            else if( y_data_type == P_TYPE_DOUBLE )
                y_value = *(double*)input_adresses->at( y_ind );

            *(float*)output_adresses->at( ogre_pos_z_ind ) = ( y_value - min_[ 1 ] ) / w_y;
        }

        *(float*)output_adresses->at( ogre_pos_y_ind ) = 0.0;
        //*(unsigned int*)output_adresses->at( ogre_col_ind ) = col_; HACK
        *(unsigned int*)output_adresses->at( ogre_col_ind ) = (unsigned int)input_->getDBOType();
    }

    outputReady();

    return true;
}
