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

#include "reconstructor.h"

#include "logger.h"

namespace reconstruction
{

/**
*/
boost::optional<std::vector<Reference>> Reconstructor::reconstruct(const std::vector<Measurement>& measurements, 
                                                                   const std::string& data_info)
{
    try
    {
        auto result = reconstruct_impl(measurements, data_info);
        return result;
    }
    catch(const std::exception& ex)
    {
        logerr << "Reconstructor::reconstruct(): " << ex.what();
    }
    catch(...)
    {
        logerr << "Reconstructor::reconstruct(): unknown error";
    }

    return {};
}

/**
*/
double Reconstructor::timestep(const Measurement& mm0, const Measurement& mm1)
{
    return (mm1.t - mm0.t);
}

/**
*/
double Reconstructor::distance(const Measurement& mm0, const Measurement& mm1)
{
    if (mm0.z.has_value() && mm1.z.has_value())
        return (Eigen::Vector3d(mm0.x, mm0.y, mm0.z.value()) - Eigen::Vector3d(mm1.x, mm1.y, mm1.z.value())).norm();
    
    return (Eigen::Vector2d(mm0.x, mm0.y) - Eigen::Vector2d(mm1.x, mm1.y)).norm();
}

}
