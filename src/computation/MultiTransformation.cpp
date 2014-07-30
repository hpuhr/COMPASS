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
 * MultiTransformation.cpp
 *
 *  Created on: Feb 3, 2013
 *      Author: sk
 */

#include "MultiTransformation.h"
#include "Buffer.h"

MultiTransformation::MultiTransformation( bool delete_trafos,
                                          std::string id,
                                          Buffer *input,
                                          bool sustainable )
:   Transformation( id, input, true, sustainable ),
    delete_( delete_trafos )
{
}

MultiTransformation::MultiTransformation( const MultiTransformation &copy )
:   Transformation( copy )
{
    delete_ = copy.delete_;
    trafos_ = copy.trafos_;
}

MultiTransformation::~MultiTransformation()
{
    if( delete_ )
    {
        unsigned int i, n = trafos_.size();
        for( i=0; i<n; ++i )
            delete trafos_[ i ];
    }
}

void MultiTransformation::addTransformation( Transformation* trafo )
{
    trafo->setSustainable( sustainable_ );
    trafo->setInputBuffer( input_ );
    trafos_.push_back( trafo );
}

bool MultiTransformation::execute ()
{
    unsigned int i, n = trafos_.size();
    unsigned int stop = n-1;
    for( i=0; i<n; ++i )
    {
        if( i == 0 )
            trafos_[ i ]->setInputBuffer( input_ );
        else
            trafos_[ i ]->setInputBuffer( trafos_[ i-1 ]->getOutput() );
        trafos_[ i ]->doExecute();
        if( !trafos_[ i ]->isOutputReady() )
        {
            stop = i;
            break;
        }
    }

    //delete all newly constructed buffers but the last one
    bool found_last = false;
    for( i=stop; i>=0; --i )
    {
        if( !trafos_[ i ]->isAppending() )
            continue;

        if( !found_last )
            found_last = true;
        else
            delete trafos_[ i ]->getOutput();
    }

    output_ = trafos_[ stop ]->getOutput();

    return true;
}

void MultiTransformation::createVariables()
{

}

Transformation* MultiTransformation::clone()
{
    return new MultiTransformation( *this );
}
