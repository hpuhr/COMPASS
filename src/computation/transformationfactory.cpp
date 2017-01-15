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

#include <cassert>

#include "TransformationFactory.h"
#include "Transformation.h"
//#include "CalculateLines.h"
//#include "CalculateHistogram.h"
//#include "CalculatePosition.h"
//#include "CalculatePosition2D.h"
//#include "CalculateHistogram2D.h"


/**
Private constructor.
  */
TransformationFactory::TransformationFactory()
{
    registerAll();
}

/**
Destructor.
  */
TransformationFactory::~TransformationFactory()
{
    unregisterAll();
}

/**
Unregisters (and deletes) all registered transformations.
  */
void TransformationFactory::unregisterAll()
{
    Transformations::iterator it, itend = trafos_.end();
    for( it=trafos_.begin(); it!=itend; ++it )
        delete it->second;
    trafos_.clear();
}

/**
Registers all needed transformations. Add transformations here if
you want to use them.
  */
void TransformationFactory::registerAll()
{
    //register trafos here

    // TODO
    assert (false);

//    registerTransformation( new CalculatePosition    );
//    registerTransformation( new CalculatePosition2D  );
//    registerTransformation( new CalculateHistogram   );
//    registerTransformation( new CalculateHistogram2D );
//    registerTransformation( new CalculateLines       );
}

/**
Registers the given transformation.
@param trafo Transformation to register.
  */
void TransformationFactory::registerTransformation( Transformation* trafo )
{
    assert( trafo );

    const std::string& id = trafo->getId();
    if( trafos_.find( id ) != trafos_.end() )
        throw std::runtime_error( "TransformationFactory: registerTransformation: ID already registered." );

    trafos_[ id ] = trafo;
}

/**
Clones the registered transformation of the given id.
@param id ID string of a transformation to clone.
  */
Transformation* TransformationFactory::createTransformation( const std::string& id )
{
    if( trafos_.find( id ) == trafos_.end() )
        throw std::runtime_error( "TransformationFactory: createTransformation: ID not registered." );
    return trafos_[ id ]->clone();
}

/**
Clones the registered transformation of the given id as a subconfigurable of the given parent.
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param parent Configurable parent.
@param id ID string of a transformation to clone.
  */
Transformation* TransformationFactory::createTransformation( const std::string& class_id,
                                                             const std::string& instance_id,
                                                             Configurable* parent,
                                                             const std::string& id )
{
    if( trafos_.find( id ) == trafos_.end() )
        throw std::runtime_error( "TransformationFactory: createTransformation: ID not registered." );
    return trafos_[ id ]->clone( class_id, instance_id, parent, false );
}
