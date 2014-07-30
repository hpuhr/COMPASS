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
 * CalculatePosition.h
 *
 *  Created on: Apr 24, 2012
 *      Author: sk
 */

#ifndef CALCULATEPOSITION_H_
#define CALCULATEPOSITION_H_

#include "Transformation.h"
#include "Global.h"

class ProjectionManager;
class DisplayObject;
class Property;

#include <string>


/**
@brief Calculates a position in 3d ogre space by using a geographical coordinate transformation
and a presentation mode.

@todo Strip coordinate from style computation.
  */
class CalculatePosition : public Transformation
{
public:
    /// @brief Constructor
    CalculatePosition( Buffer* input=NULL );
    /// @brief Configurable constructor
    CalculatePosition( std::string class_id,
                       std::string instance_id,
                       Configurable *parent );
    /// @brief Copy constructor
    CalculatePosition( const CalculatePosition& copy );
    /// @brief Copy constructor
    virtual ~CalculatePosition();

    /// @brief Sets the transformations presentation mode
    void setPresentationMode( PRESENTATION_MODE pmode );

    /// @brief Assignment operator
    CalculatePosition& operator=( const CalculatePosition& rhs );
    /// @brief Clone method
    virtual Transformation* clone();
    /// @brief Configurable clone method
    virtual Transformation* clone( const std::string& class_id,
                                   const std::string& instance_id,
                                   Configurable* parent,
                                   bool assign );

protected:
    /// @brief Creates the transformations transformation variables
    virtual void createVariables();
    /// @brief Main execution method
    virtual bool execute();
    /// @brief Set common stuff
    void setupCommon();

    /// @brief Execute for plots
    bool executePlots();
    /// @brief Execute for system tracks
    bool executeSysTracks();
    /// @brief Execute for ADSB
    bool executeADSB();
    /// @brief Execute for WAM
    bool executeMLAT();
    /// @brief Execute for Reference trajectories
    bool executeRefTraj();

    /// Input var longitude key
    unsigned int pos_u_key_;
    /// Input var latitude key
    unsigned int pos_v_key_;
    /// Input var height key
    unsigned int pos_h_key_;
    /// Input var data source key
    unsigned int ds_key_;
    /// Input var detection type key
    unsigned int dtype_key_;

    /// Output var x coord key
    unsigned int ogre_pos_x_key_;
    /// Output var y coord key
    unsigned int ogre_pos_y_key_;
    /// Output var z coord key
    unsigned int ogre_pos_z_key_;
    /// Output var color key
    unsigned int ogre_color_key_;
    /// Output var symbol key
    unsigned int ogre_symbol_key_;

    /// Projection manager
    ProjectionManager* proj_;
    /// Presentation mode
    PRESENTATION_MODE p_mode_;
};

#endif /* CALCULATEPOSITION_H_ */
