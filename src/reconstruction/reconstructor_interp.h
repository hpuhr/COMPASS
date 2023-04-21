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

#pragma once

#include "reconstructor.h"

namespace reconstruction
{

/**
*/
class ReconstructorInterp : public Reconstructor
{
public:
    struct Config
    {
        double sample_dt = 1.0;   //sample interval in seconds
        double min_dt    = 1e-06; //minimum time difference between measurements
        double max_dt    = 30.0;  //maximum time difference between measurements
        double min_len   = 1e-07; //minimum length between measurements
    };

    ReconstructorInterp() = default;
    virtual ~ReconstructorInterp() = default;

    Config& config() { return config_; }
    const Config& config() const { return config_; }

protected:
    virtual boost::optional<std::vector<Reference>> reconstruct_impl(const std::vector<Measurement>& measurements, 
                                                                     const std::string& data_info ) override;

    boost::optional<std::vector<Reference>> reconstructPart(const std::vector<Measurement>& measurements, 
                                                            const std::string& data_info) const;

    Reference generateReference(double lat, 
                                double lon, 
                                const boost::posix_time::ptime& t, 
                                const Measurement& mm0, 
                                const Measurement& mm1, 
                                double interp_factor) const;
    Reference generateReference(const Measurement& mm) const;

    std::vector<Reference> interpolateLinear(const std::vector<Measurement>& measurements) const;
private:
    Config config_;
};

} // namespace reconstruction
