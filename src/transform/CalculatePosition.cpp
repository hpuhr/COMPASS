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
 * CalculatePosition.cpp
 *
 *  Created on: Apr 24, 2012
 *      Author: sk
 */

#include "CalculatePosition.h"
#include "DBObjectManager.h"

#include "PropertyList.h"
#include "Buffer.h"
#include "ATSDB.h"
#include "ProjectionManager.h"

//HACK
#include "AirspaceSectorManager.h"
#include "AirspaceSector.h"
//\HACK

#define _USE_MATH_DEFINES
#include <math.h>


/*************************************************************************
CalculatePosition
 **************************************************************************/

/**
Constructor.
@param input Input buffer.
 */
CalculatePosition::CalculatePosition( Buffer* input )
:   Transformation( input )
{
    //set id in derived class
    id_ = "TransformationPosition";

    proj_ = &ProjectionManager::getInstance();

    //important, call in constructor
    createVariables();
}

/**
Configurable constructor.
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param parent Configurable parent.
 */
CalculatePosition::CalculatePosition( std::string class_id,
                                      std::string instance_id,
                                      Configurable *parent )
:   Transformation( class_id, instance_id, parent )
{
    //set id in derived class
    id_ = "TransformationPosition";

    proj_ = &ProjectionManager::getInstance();

    //important, call in constructor
    createVariables();
}

/**
Copy constructor.
@param copy Instance to copy.
 */
CalculatePosition::CalculatePosition( const CalculatePosition& copy )
{
    *this = copy;
    proj_ = &ProjectionManager::getInstance();
}

/**
Destructor.
 */
CalculatePosition::~CalculatePosition()
{
}

/**
Assignment operator.
@param rhs Instance to assign.
@return Reference to me.
  */
CalculatePosition& CalculatePosition::operator=( const CalculatePosition& rhs )
{
    Transformation::operator=( rhs );

    p_mode_ = rhs.p_mode_;

    return *this;
}

/**
Clone method.
@return Cloned me.
  */
Transformation* CalculatePosition::clone()
{
    return new CalculatePosition( *this );
}


/**
Configurable clone method.
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param parent Configurable parent.
@param assign Return a fresh version of the transformation or copy data inside.
@return Cloned transformation.
  */
Transformation* CalculatePosition::clone( const std::string& class_id,
                                          const std::string& instance_id,
                                          Configurable* parent,
                                          bool assign )
{
    CalculatePosition* trafo = new CalculatePosition( class_id, instance_id, parent );
    if( assign )
        *trafo = *this;
    return trafo;
}

/**
Sets the transformations presentation mode.
@param pmode New presentation mode.
 */
void CalculatePosition::setPresentationMode( PRESENTATION_MODE pmode )
{
    p_mode_ = pmode;
}

/**
Creates the transformation variables.
  */
void CalculatePosition::createVariables()
{
    //if not already here, add them
    if( numberInputVariables() == 0 )
    {
        addInputVariable( "x", P_TYPE_DOUBLE );
        addInputVariable( "y", P_TYPE_DOUBLE );
        addInputVariable( "h", P_TYPE_INT );

        //two optional input variables
        addInputVariable( "data_source", P_TYPE_INT )->setOptional( true );
        addInputVariable( "detection_type", P_TYPE_CHAR )->setOptional( true );
    }

    if( numberOutputVariables() == 0 )
    {
        addOutputVariable( "ogre_pos_x", P_TYPE_FLOAT, "ogre_pos_x" );
        addOutputVariable( "ogre_pos_y", P_TYPE_FLOAT, "ogre_pos_y" );
        addOutputVariable( "ogre_pos_z", P_TYPE_FLOAT, "ogre_pos_z" );
        addOutputVariable( "color", P_TYPE_UINT, "ogre_color" );
        addOutputVariable( "symbol", P_TYPE_UINT, "ogre_symbol" );
    }
}

/**
Sets some common keys. Those can be used to quickaccess properties.
 */
void CalculatePosition::setupCommon()
{
    pos_u_key_ = iKey( "x" );
    pos_v_key_ = iKey( "y" );
    pos_h_key_ = iKey( "h" );
    ds_key_    = iKey( "data_source" );
    dtype_key_ = iKey( "detection_type" );

    ogre_pos_x_key_  = oKey( "ogre_pos_x" );
    ogre_pos_y_key_  = oKey( "ogre_pos_y" );
    ogre_pos_z_key_  = oKey( "ogre_pos_z" );
    ogre_color_key_  = oKey( "color" );
    ogre_symbol_key_ = oKey( "symbol" );
}

/**
Main execution method.
@return True if the execution went ok, false otherwise.
 */
bool CalculatePosition::execute()
{
    setupCommon();

    bool ok;
    switch( input_->getDBOType() )
    {
        case DBO_PLOTS:
            ok = executePlots();
            break;
        case DBO_SYSTEM_TRACKS:
            ok = executeSysTracks();
            break;
        case DBO_ADS_B:
            ok = executeADSB();
            break;
        case DBO_MLAT:
            ok = executeMLAT();
            break;
//        case DBO_REFERENCE_TRAJECTORIES:
//            ok = executeRefTraj();
//            break;
        default:
            throw std::runtime_error( "CalculatePosition: execute: Invalid dbo type." );
    }

    return ok;
}

/**
Execution method for plots.
@return True if the execution went ok, false otherwise.
 */
bool CalculatePosition::executePlots()
{
    //init buffer indices
    if( append_ )
    {
        input_->setIndex( 0 );
    }
    else
    {
        input_->setIndex( 0 );
        output_->setIndex( 0 );
    }

    //check if optional variables exist, the others are prechecked.
    bool detection_type_present = iExists( dtype_key_ );
    bool data_source_present    = iExists( ds_key_ );

    //iterate over buffer entries
    bool det_valid;
    char det;
    double x, y;
    unsigned int col, sym, radar_num, cnt, n = input_->getSize();
    for( cnt=0; cnt<n; ++cnt )
    {
        //increment buffer indices
        if( cnt != 0 )
        {
            if (append_)
            {
                input_->incrementIndex();
            }
            else
            {
                input_->incrementIndex( );
                output_->incrementIndex( );
            }
        }

        //set the current addresses
        iSetAddresses(  input_->getAdresses() );
        oSetAddresses( output_->getAdresses() );

        //x, z
        //check NaN
        if( iIsNan( pos_u_key_ ) || iIsNan( pos_v_key_ ) )
        {
            oSetNan( ogre_pos_x_key_ );
            oSetNan( ogre_pos_z_key_ );
        }
        else
        {
            //geographical projection
            proj_->geo2Cart( *(double*)iPtr( pos_v_key_ ), *(double*)iPtr( pos_u_key_ ), x, y );
            *(float*)oPtr( ogre_pos_x_key_ ) = x;
            *(float*)oPtr( ogre_pos_z_key_ ) = y;
        }

        //y
        //check NaN
        if( iIsNan( pos_h_key_ ) )
            *(float*)oPtr( ogre_pos_y_key_ ) = 0.0f;
        else
            *(float*)oPtr( ogre_pos_y_key_ ) = proj_->transformHeight( *(int*)iPtr( pos_h_key_ ) );

        //style
        if( data_source_present && detection_type_present )
        {
            det_valid = !iIsNan( dtype_key_ );
            det       = *(char*)iPtr( dtype_key_ );
            radar_num = *(int*)iPtr( ds_key_ );

            if( det_valid )
            {
                //symbols
                if( p_mode_ == PRESENTATION_MODE_POINT_DETECTION ||
                    p_mode_ == PRESENTATION_MODE_POINT_RADAR )
                {
                    sym = 0;
                }
                else
                {
                    if( det == 1 )
                        sym = 2;
                    else if( det == 2 )
                        sym = 1;
                    else if( det == 3 )
                        sym = 3;
                    else if ( det > 3 && det < 6 )
                        sym = 5;
                    else if( det > 5 && det < 8)
                        sym = 6;
                    else
                        sym = 4;
                }

                //colors
                if( p_mode_ == PRESENTATION_MODE_POINT_DETECTION ||
                    p_mode_ == PRESENTATION_MODE_SYMBOL_DETECTION )
                {
                    if( det > 0 && det < 4 )
                        col = det;
                    else if( det > 3 )
                        col = 5;
                    else
                        col = 4;
                }
                else
                {
                    col = 5 + radar_num;
                }
            }
            else
            {
                sym=0;
                col=0;
            }
        }
        else
        {
            sym=0;
            col=0;
        }

        *(unsigned int*)oPtr( ogre_color_key_  ) = col;
        *(unsigned int*)oPtr( ogre_symbol_key_ ) = sym;
    }

    outputReady();

    return true;
}

/**
 */
bool CalculatePosition::executeSysTracks()
{
    if (append_)
    {
      input_->setIndex( 0 );
    }
    else
    {
      input_->setIndex( 0 );
      output_->setIndex( 0 );
    }

    bool data_source_present = iExists( ds_key_ );

    double x, y;
    unsigned int col, sym, source, cnt, n = input_->getSize();
    for( cnt=0; cnt<n; ++cnt )
    {
        if( cnt != 0 )
        {
            if (append_)
            {
                input_->incrementIndex();
            }
            else
            {
                input_->incrementIndex( );
                output_->incrementIndex( );
            }
        }

        iSetAddresses(  input_->getAdresses() );
        oSetAddresses( output_->getAdresses() );

        //x, z
        if( iIsNan( pos_u_key_ ) || iIsNan( pos_v_key_ ) )
        {
            oSetNan( ogre_pos_x_key_ );
            oSetNan( ogre_pos_z_key_ );
        }
        else
        {
            proj_->geo2Cart( *(double*)iPtr( pos_v_key_ ), *(double*)iPtr( pos_u_key_ ), x, y );
            *(float *)oPtr( ogre_pos_x_key_ ) = x;
            *(float *)oPtr( ogre_pos_z_key_ ) = y;
        }

        //y
        if( iIsNan( pos_h_key_ ) )
            *(float*)oPtr( ogre_pos_y_key_ ) = 0.0;
        else
            *(float*)oPtr( ogre_pos_y_key_ ) = proj_->transformHeight( *(int*)iPtr( pos_h_key_ ) );

        //color & symbol
        if( p_mode_ == PRESENTATION_MODE_POINT_DETECTION ||
            p_mode_ == PRESENTATION_MODE_POINT_RADAR )
            sym = 0;
        else
            sym = 1;

        col = 0;
        if( p_mode_ == PRESENTATION_MODE_POINT_RADAR ||
            p_mode_ == PRESENTATION_MODE_SYMBOL_RADAR )
        {
            if( data_source_present && !iIsNan( ds_key_ ) )
            {
                source = *(int*)iPtr( ds_key_ );
                col = 5 + source;
            }
        }

        *(unsigned int *)oPtr( ogre_color_key_  ) = col;
        *(unsigned int *)oPtr( ogre_symbol_key_ ) = sym;
    }

    outputReady();

    return true;
}

/**
 */
bool CalculatePosition::executeADSB()
{
    assert( input_ );

    if (append_)
    {
      input_->setIndex( 0 );
    }
    else
    {
      input_->setIndex( 0 );
      output_->setIndex( 0 );
    }

    double x, y;
    //double rad2deg = 180.0 / M_PI;
    unsigned int col, sym, cnt, n = input_->getSize();
    for( cnt=0; cnt<n; ++cnt )
    {
        if( cnt != 0 )
        {
            if( append_ )
            {
                input_->incrementIndex();
            }
            else
            {
                input_->incrementIndex( );
                output_->incrementIndex( );
            }
        }

        iSetAddresses(  input_->getAdresses() );
        oSetAddresses( output_->getAdresses() );

        //x & z
        if( iIsNan( pos_u_key_ ) || iIsNan( pos_v_key_ ) )
        {
            oSetNan( ogre_pos_x_key_ );
            oSetNan( ogre_pos_z_key_ );
        }
        else
        {
            proj_->geo2Cart( *(double*)iPtr( pos_v_key_ ), *(double*)iPtr( pos_u_key_ ), x, y );
            //proj_->project( *(double*)iPtr( pos_v_key_ ) * rad2deg, *(double*)iPtr( pos_u_key_ ) * rad2deg, x, y );
            *(float*)oPtr( ogre_pos_x_key_ ) = x;
            *(float*)oPtr( ogre_pos_z_key_ ) = y;
        }

        //y
        if( iIsNan( pos_h_key_ ) )
            *(float*)oPtr( ogre_pos_y_key_ ) = 0.0;
        else
            *(float*)oPtr( ogre_pos_y_key_ ) = proj_->transformHeight( *(int*)iPtr( pos_h_key_ ) );

        //color & symbol
        if( p_mode_ == PRESENTATION_MODE_POINT_DETECTION ||
            p_mode_ == PRESENTATION_MODE_POINT_RADAR )
            sym = 0;
        else
            sym = 3;
        col = 15;
        *(unsigned int*)oPtr( ogre_color_key_  ) = col;
        *(unsigned int*)oPtr( ogre_symbol_key_ ) = sym;
    }

    outputReady();

    return true;
}

/**
 */
bool CalculatePosition::executeMLAT()
{
    logdbg << "CalculatePosition: executeMLAT";
    if( append_ )
    {
      input_->setIndex( 0 );
    }
    else
    {
      input_->setIndex( 0 );
      output_->setIndex( 0 );
    }

    //HACK
    bool filter;
    //double height;
    AirspaceSector *sector = AirspaceSectorManager::getInstance().getSector("LowerHeightFilter");
    //\HACK

    double x, y;
    //double rad2deg = 180.0/M_PI;
    unsigned int col, sym, cnt, n = input_->getSize();
    for( cnt=0; cnt<n; ++cnt )
    {
        if( cnt != 0 )
        {
            if (append_)
            {
                input_->incrementIndex();
            }
            else
            {
                input_->incrementIndex( );
                output_->incrementIndex( );
            }
        }

        iSetAddresses(  input_->getAdresses() );
        oSetAddresses( output_->getAdresses() );

        //x & z
        if( iIsNan( pos_u_key_ ) || iIsNan( pos_v_key_ ) )
        {
            oSetNan( ogre_pos_x_key_ );
            oSetNan( ogre_pos_z_key_ );
        }
        else
        {
            //proj_->project( *(double*)iPtr( pos_v_key_ ) * rad2deg, *(double*)iPtr( pos_u_key_ ) * rad2deg, x, y );
            proj_->geo2Cart( *(double*)iPtr( pos_v_key_ ), *(double*)iPtr( pos_u_key_ ), x, y );
            *(float*)oPtr( ogre_pos_x_key_ ) = x;
            *(float*)oPtr( ogre_pos_z_key_ ) = y;
        }

        //y
        if( iIsNan( pos_h_key_ ) )
        {
            *(float*)oPtr( ogre_pos_y_key_ ) = 0.0;

            //HACK
            filter = true;
            //\HACK

        }
        else
        {
            *(float*)oPtr( ogre_pos_y_key_ ) = proj_->transformHeight( *(int*)iPtr( pos_h_key_ ) );
            //loginf << "UGA to height org " << *(int*)iPtr( pos_h_key_ ) << " transformed " << *(float*)oPtr( ogre_pos_y_key_ );

            //HACK
            //height =*(int*)iPtr( pos_h_key_ );
            //loginf << "UGA to height org " << *(int*)iPtr( pos_h_key_ ) << " double " << height;
            //bool isPointInside (double latitude, double longitude, double height_ft, bool debug);
            filter = !sector->isPointInside(*(double*)iPtr( pos_v_key_ ), *(double*)iPtr( pos_u_key_ ), *(int*)iPtr( pos_h_key_ ), false);
            //\HACK
        }

        //color & symbol
        if( p_mode_ == PRESENTATION_MODE_POINT_DETECTION ||
            p_mode_ == PRESENTATION_MODE_POINT_RADAR )
            sym = 0;
        else
            sym = 4;
        col = 20;

        //HACK
        if (!filter)
            *(unsigned int*)oPtr( ogre_color_key_  ) = col; // not HACK
        else
            *(unsigned int*)oPtr( ogre_color_key_  ) = 0;
        //\HACK
        *(unsigned int*)oPtr( ogre_symbol_key_ ) = sym;
    }

    logdbg << "CalculatePosition: executeMLAT: cnt " << cnt;

    outputReady();

    return true;
}

/**
 */
//bool CalculatePosition::executeRefTraj()
//{
//    assert( input_ );
//
//    if( append_ )
//    {
//      input_->setIndex( 0 );
//    }
//    else
//    {
//      input_->setIndex( 0 );
//      output_->setIndex( 0 );
//    }
//
//    unsigned int col, sym, cnt, n = input_->getSize();
//    for( cnt=0; cnt<n; ++cnt )
//    {
//        if( cnt != 0 )
//        {
//            if (append_)
//            {
//                input_->incrementIndex();
//            }
//            else
//            {
//                input_->incrementIndex( );
//                output_->incrementIndex( );
//            }
//        }
//
//        iSetAddresses(  input_->getAdresses() );
//        oSetAddresses( output_->getAdresses() );
//
//        //x
//        if( iIsNan( pos_u_key_ ) )
//            oSetNan( ogre_pos_x_key_ );
//        else
//            *(float*)oPtr( ogre_pos_x_key_ ) = proj_->transformPositionX( *(double*)iPtr( pos_u_key_ ) );
//
//        //y
//        if( iIsNan( pos_h_key_ ) )
//            *(float*)oPtr( ogre_pos_y_key_ ) = 0.0;
//        else
//            *(float*)oPtr( ogre_pos_y_key_ ) = proj_->transformHeight( *(int*)iPtr( pos_h_key_ ) );
//
//        //z
//        if( iIsNan( pos_v_key_ ) )
//            oSetNan( ogre_pos_z_key_ );
//        else
//            *(float*)oPtr( ogre_pos_z_key_ ) = proj_->transformPositionY( *(double*)iPtr( pos_v_key_ ) );
//
//        //color & symbol
//        if( p_mode_ == PRESENTATION_MODE_POINT_DETECTION ||
//            p_mode_ == PRESENTATION_MODE_POINT_RADAR )
//            sym = 0;
//        else
//            sym = 1;
//        col = 0;
//
//        *(unsigned int*)oPtr( ogre_color_key_  ) = col;
//        *(unsigned int*)oPtr( ogre_symbol_key_ ) = sym;
//    }
//
//    outputReady();
//
//    return true;
//}


