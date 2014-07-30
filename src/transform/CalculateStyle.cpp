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


#include "CalculateStyle.h"
#include "DBObjectManager.h"

#include "PropertyList.h"
#include "Buffer.h"
#include "ATSDB.h"
#include "ProjectionManager.h"

#define _USE_MATH_DEFINES
#include <math.h>


/**
 */
CalculateStyle::CalculateStyle( Buffer* input )
:   Transformation( input )
{
    id_ = "TransformationStyle";
}

/**
 */
CalculateStyle::CalculateStyle( const std::string& class_id,
                                const std::string& instance_id,
                                Configurable *parent )
:   Transformation( class_id, instance_id, parent )
{
    id_ = "TransformationStyle";
}

/**
 */
CalculateStyle::CalculateStyle( const CalculateStyle& copy )
{
    *this = copy;
}

/**
 */
CalculateStyle::~CalculateStyle()
{
}


/**
  */
CalculateStyle& CalculateStyle::operator=( const CalculateStyle& rhs )
{
    Transformation::operator=( rhs );

    p_mode_ = rhs.p_mode_;

    return *this;
}

/**
  */
Transformation* CalculateStyle::clone()
{
    return new CalculateStyle( *this );
}


/**
  */
Transformation* CalculateStyle::clone( const std::string& class_id,
                                       const std::string& instance_id,
                                       Configurable* parent,
                                       bool assign )
{
    CalculateStyle* trafo = new CalculateStyle( class_id, instance_id, parent );
    if( assign )
        *trafo = *this;
    return trafo;
}

/**
 */
void CalculateStyle::setPresentationMode( PRESENTATION_MODE pmode )
{
    p_mode_ = pmode;
}

/**
  */
void CalculateStyle::createVariables()
{
}

/**
 */
bool CalculateStyle::execute()
{
    return true;
}
