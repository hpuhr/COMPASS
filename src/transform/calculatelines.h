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


#ifndef CALCULATELINES_H
#define CALCULATELINES_H

#include "Transformation.h"


/**
  */
class CalculateLines : public Transformation
{
public:
    typedef struct trackinfo
    {
        double pos_[ 3 ];
        bool track_end_;
    } TrackInfo;

    typedef std::map<int,TrackInfo> TrackInfoMap;
    typedef std::map<int,int> TrackNumberMap;

    CalculateLines( Buffer* input=NULL );
    CalculateLines( std::string class_id,
                    std::string instance_id,
                    Configurable *parent );
    CalculateLines( const CalculateLines& copy );
    virtual ~CalculateLines();

    CalculateLines& operator=( const CalculateLines& rhs );
    virtual Transformation* clone();
    virtual Transformation* clone( const std::string& class_id,
                                   const std::string& instance_id,
                                   Configurable* parent,
                                   bool assign );

    virtual void clearIntermediateData();

protected:
    virtual bool execute();
    virtual void createVariables();

    TrackNumberMap track_number_map_;
    TrackInfoMap track_info_;
    int tracknum_counter_;
};

#endif //CALCULATELINES_H
