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


#ifndef CALCULATESTYLE_H
#define CALCULATESTYLE_H

#include "Transformation.h"
#include "Global.h"

class ProjectionManager;

#include <string>


/**
@todo DO!
  */
class CalculateStyle : public Transformation
{
public:
    CalculateStyle( Buffer* input=NULL );
    CalculateStyle( const std::string& class_id,
                    const std::string& instance_id,
                    Configurable *parent );
    CalculateStyle( const CalculateStyle& copy );
    virtual ~CalculateStyle();

    void setPresentationMode( PRESENTATION_MODE pmode );

    CalculateStyle& operator=( const CalculateStyle& rhs );
    virtual Transformation* clone();
    virtual Transformation* clone( const std::string& class_id,
                                   const std::string& instance_id,
                                   Configurable* parent,
                                   bool assign );
protected:
    virtual void createVariables();
    virtual bool execute();

    PRESENTATION_MODE p_mode_;
};

#endif //CALCULATESTYLE_H
