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


#include "CalculateLines.h"
#include "Buffer.h"


/**
 */
CalculateLines::CalculateLines( Buffer* input )
:   Transformation( input ),
    tracknum_counter_( 0 )
{
    id_ = "TransformationLines";

    setSustainable( true );
    createVariables();
}

/**
 */
CalculateLines::CalculateLines( std::string class_id,
                                std::string instance_id,
                                Configurable *parent )
:   Transformation( class_id, instance_id, parent ),
    tracknum_counter_( 0 )
{
    id_ = "TransformationLines";

    setSustainable( true );
    createVariables();
}

/**
 */
CalculateLines::CalculateLines( const CalculateLines& copy )
{
    *this = copy;
}

/**
 */
CalculateLines::~CalculateLines()
{
}

/**
  */
CalculateLines& CalculateLines::operator=( const CalculateLines& rhs )
{
    Transformation::operator=( rhs );

    track_number_map_ = rhs.track_number_map_;
    track_info_       = rhs.track_info_;
    tracknum_counter_ = rhs.tracknum_counter_;

    return *this;
}

/**
  */
Transformation* CalculateLines::clone()
{
    return new CalculateLines( *this );
}


/**
  */
Transformation* CalculateLines::clone( const std::string& class_id,
                                       const std::string& instance_id,
                                       Configurable* parent,
                                       bool assign )
{
    CalculateLines* trafo = new CalculateLines( class_id, instance_id, parent );
    if( assign )
        *trafo = *this;
    return trafo;
}

/**
 */
void CalculateLines::createVariables()
{
    if( numberInputVariables() == 0 )
    {
        addInputVariable( "pos_x", P_TYPE_FLOAT );
        addInputVariable( "pos_y", P_TYPE_FLOAT );
        addInputVariable( "pos_z", P_TYPE_FLOAT );
        addInputVariable( "track_num_value", P_TYPE_INT );
        addInputVariable( "time_last_update", P_TYPE_DOUBLE );
        addInputVariable( "track_created", P_TYPE_STRING );
        addInputVariable( "track_terminated", P_TYPE_STRING );
    }

    if( numberOutputVariables() == 0 )
    {
        addOutputVariable( "line_x", P_TYPE_FLOAT, "line_x" );
        addOutputVariable( "line_y", P_TYPE_FLOAT, "line_y" );
        addOutputVariable( "line_z", P_TYPE_FLOAT, "line_z" );
    }
}

/**
 */
bool CalculateLines::execute()
{
  unsigned int px_ind = getOutputVariable( "line_x" )->propertyIndex( output_ );
  unsigned int py_ind = getOutputVariable( "line_y" )->propertyIndex( output_ );
  unsigned int pz_ind = getOutputVariable( "line_z" )->propertyIndex( output_ );

  Property* px_prop = getOutputVariable( "line_x" )->property( output_ );
  Property* py_prop = getOutputVariable( "line_y" )->property( output_ );
  Property* pz_prop = getOutputVariable( "line_z" )->property( output_ );

  unsigned int x_ind = getInputVariable( "pos_x" )->propertyIndex( input_ );
  unsigned int y_ind = getInputVariable( "pos_y" )->propertyIndex( input_ );
  unsigned int z_ind = getInputVariable( "pos_z" )->propertyIndex( input_ );
  unsigned int tracknum_ind     = getInputVariable( "track_num_value" )->propertyIndex( input_ );
  unsigned int time_ind         = getInputVariable( "time_last_update" )->propertyIndex( input_ );
  unsigned int track_create_ind = getInputVariable( "track_created" )->propertyIndex( input_ );
  unsigned int track_end_ind    = getInputVariable( "track_terminated" )->propertyIndex( input_ );

  Property* x_prop = input_->getPropertyList()->getProperty( x_ind );
  Property* y_prop = input_->getPropertyList()->getProperty( y_ind );
  Property* z_prop = input_->getPropertyList()->getProperty( z_ind );
  Property* time_prop = input_->getPropertyList()->getProperty( time_ind );
  Property* tracknum_prop = input_->getPropertyList()->getProperty( tracknum_ind );
  Property* track_create_prop = input_->getPropertyList()->getProperty( track_create_ind );
  Property* track_end_prop = input_->getPropertyList()->getProperty( track_end_ind );

    /*if( input_->getPropertyList()->hasProperty( "track_number__value" ) )
    {
      tracknum_ind = input_->getPropertyList()->getPropertyIndex( "track_number__value" );
      time_ind     = input_->getPropertyList()->getPropertyIndex( "time_of_last_update__value" );
      track_create_ind     = input_->getPropertyList()->getPropertyIndex( "track_created" );
      track_end_ind     = input_->getPropertyList()->getPropertyIndex( "track_terminated" );
      track_status_string=false;
    }
    else if( input_->getPropertyList()->hasProperty( "TRACK_NUM" ) )
    {
      tracknum_ind = input_->getPropertyList()->getPropertyIndex( "TRACK_NUM" );
      time_ind     = input_->getPropertyList()->getPropertyIndex( "TOD" );
      track_create_ind     = input_->getPropertyList()->getPropertyIndex( "TRACK_CREATED" );
      track_end_ind     = input_->getPropertyList()->getPropertyIndex( "TRACK_END" );
      track_status_string=true;
    }
    else
      throw std::runtime_error ("CalculateLines: execute: track info incomplete");*/

  int data_tracknum;
  double pt[ 3 ];
  double x,y,z;
  std::vector<void*>* adresses;
  bool track_created, track_end;

  input_->setIndex( 0 );
  unsigned int i, n = input_->getSize();
  for( i=0; i<n; ++i )
  {
    if( i != 0 )
      input_->incrementIndex();

    adresses = input_->getAdresses();

    if( isNan( x_prop->data_type_int_, adresses->at( x_ind ) ) ||
        isNan( y_prop->data_type_int_, adresses->at( y_ind ) ) ||
        isNan( z_prop->data_type_int_, adresses->at( z_ind ) ) ||
        isNan( tracknum_prop->data_type_int_, adresses->at( tracknum_ind ) ) ||
        isNan( time_prop->data_type_int_, adresses->at( time_ind ) ) ||
        isNan( track_create_prop->data_type_int_, adresses->at( track_create_ind ) ) ||
        isNan( track_end_prop->data_type_int_, adresses->at( track_end_ind ) ))
    {
      logdbg  << "CalculateLines: execute: index " << i << " has nans";
      setNan( px_prop->data_type_int_, adresses->at( px_ind ) );
      setNan( py_prop->data_type_int_, adresses->at( py_ind ) );
      setNan( pz_prop->data_type_int_, adresses->at( pz_ind ) );
      continue;
    }

    if( track_create_prop->data_type_int_ == P_TYPE_STRING )
    {
      if ((*((std::string *)adresses->at( track_create_ind ))).compare ("Y") == 0)
        track_created=true;
      else if ((*((std::string *)adresses->at( track_create_ind ))).compare ("N") == 0)
        track_created=false;
      else
      {
        throw std::runtime_error( "CalculateLines: execute: unknown track create value" );
      }

      if ((*((std::string *)adresses->at( track_end_ind ))).compare ("Y") == 0)
        track_end=true;
      else if ((*((std::string *)adresses->at( track_end_ind ))).compare ("N") == 0)
        track_end=false;
      else
      {
        throw std::runtime_error( "CalculateLines: execute: unknown track end value" );
      }
    }
    else if( track_create_prop->data_type_int_ == P_TYPE_CHAR )
    {
      if (*((char *)adresses->at( track_create_ind )) == 1)
        track_created=true;
      else if (*((char *)adresses->at( track_create_ind )) == 0)
        track_created=false;
      else
      {
        throw std::runtime_error( "CalculateLines: execute: unknown track create value" );
      }

      if (*((char *)adresses->at( track_end_ind )) == 1)
        track_end=true;
      else if (*((char *)adresses->at( track_end_ind )) == 0)
        track_end=false;
      else
      {
          throw std::runtime_error( "CalculateLines: execute: unknown track end value" );
      }
    }
    else
    {
        throw std::runtime_error( "CalculateLines: execute: unknown track create datatype" );
    }

    data_tracknum = *((int*)adresses->at( tracknum_ind ));
    x = *((float *)adresses->at( x_ind ));
    y = *((float *)adresses->at( y_ind ));
    z = *((float *)adresses->at( z_ind ));

    if( track_number_map_.count( data_tracknum ) == 0 ) // new track
    {
      if( !track_created )
      {
        logdbg  << "CalculateLines: execute: new track without created flag";
      }
      logdbg  << "CalculateLines: execute: adding new track with tracknum " << data_tracknum;
      // not yet exists
      track_number_map_[ data_tracknum ] = tracknum_counter_;
      TrackInfo& info = track_info_[ tracknum_counter_ ];
      info.pos_[ 0 ] = x;
      info.pos_[ 1 ] = y;
      info.pos_[ 2 ] = z;
      //info.time_ = *((double*)adresses->at( time_ind ));
      info.track_end_ = track_end;
      ++tracknum_counter_;

      // do not add line
      setNan( px_prop->data_type_int_, adresses->at( px_ind ) );
      setNan( py_prop->data_type_int_, adresses->at( py_ind ) );
      setNan( pz_prop->data_type_int_, adresses->at( pz_ind ) );
    }
    else
    {
      //add to existing track
      int virtual_tracknum_old = track_number_map_[ data_tracknum ];
      assert( track_info_.count( virtual_tracknum_old ) == 1 );

      //time_new = *((double *)adresses->at( time_ind ));

      //loginf << "CalculateLines: execute: index << " << i << " tracknum " << data_tracknum << " time diff "
      //    << time_new - track_info_[ virtual_tracknum_old ].time_  << " new " << time_new << " old " <<
      //    track_info_[ virtual_tracknum_old ].time_ << endl;

      if (track_created)
      {
        logdbg << "CalculateLines: execute: adding new track with tracknum " << data_tracknum << ", previous existed";
        track_number_map_[ data_tracknum ] = tracknum_counter_;
        TrackInfo& info = track_info_[ tracknum_counter_ ];
        info.pos_[ 0 ] = x;
        info.pos_[ 1 ] = y;
        info.pos_[ 2 ] = z;
        //info.time_ = *((double*)adresses->at( time_ind ));
        info.track_end_=track_end;
        ++tracknum_counter_;

        // do not add line
        setNan( px_prop->data_type_int_, adresses->at( px_ind ) );
        setNan( py_prop->data_type_int_, adresses->at( py_ind ) );
        setNan( pz_prop->data_type_int_, adresses->at( pz_ind ) );
      }
      else // should be added to existing one
      {
        TrackInfo& info = track_info_[ virtual_tracknum_old ];

        if (info.track_end_)
        {
          logdbg  << "CalculateLines: execute: track info out of sync for tracknum " << data_tracknum;

          // do not add line
          setNan( px_prop->data_type_int_, adresses->at( px_ind ) );
          setNan( py_prop->data_type_int_, adresses->at( py_ind ) );
          setNan( pz_prop->data_type_int_, adresses->at( pz_ind ) );

        }
        else
        {
          //loginf  << "CalculateLines: execute: adding track info for tracknum " << data_tracknum;
          pt[ 0 ] = info.pos_[ 0 ];
          pt[ 1 ] = info.pos_[ 1 ];
          pt[ 2 ] = info.pos_[ 2 ];
          info.pos_[ 0 ] = x;
          info.pos_[ 1 ] = y;
          info.pos_[ 2 ] = z;
          //info.time_ = *((double*)adresses->at( time_ind ));
          info.track_end_=track_end;

          //add line
          *(float*)adresses->at( px_ind ) = pt[ 0 ];
          *(float*)adresses->at( py_ind ) = pt[ 1 ];
          *(float*)adresses->at( pz_ind ) = pt[ 2 ];
        }
      }
    }
  }

  outputReady();

  return true;
}

/**
 */
void CalculateLines::clearIntermediateData()
{
  track_number_map_.clear();
  track_info_.clear();
  tracknum_counter_ = 0;
}
