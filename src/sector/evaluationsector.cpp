/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "evaluationsector.h"

#include "logger.h"

/**
*/
EvaluationSector::EvaluationSector(unsigned int id, 
                                   const std::string& name, 
                                   const std::string& layer_name,
                                   bool exclusion_sector, 
                                   QColor color, 
                                   std::vector<std::pair<double,double>> points)
:   Sector(id, name, layer_name, true, exclusion_sector, color, points)
{
}

/**
*/
EvaluationSector::EvaluationSector(unsigned int id, 
                                   const std::string& name, 
                                   const std::string& layer_name)
:   Sector(id, name, layer_name, true)
{
}
