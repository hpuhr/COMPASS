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

#include "reconstructor_defs.h"

#include <vector>

#include <boost/optional.hpp>

#include <QColor>

namespace reconstruction
{

/**
 * Interpolates given measurements using cubic splines.
*/
class SplineInterpolator
{
public:
    struct Config
    {
        double sample_dt = 1.0; //sample interval in seconds

        bool   check_fishy_segments  = true;
        bool   interpolate_cart      = false;
        
        double min_dt                      = 1e-06; //minimum time difference between measurements
        double max_dt                      = 30.0;  //maximum time difference between measurements
        double min_len                     = 1e-07; //minimum length between measurements
        double max_segment_distance_factor = 2.0;
    };

    SplineInterpolator() = default;
    virtual ~SplineInterpolator() = default;

    Config& config() { return config_; }
    const Config& config() const { return config_; }

    std::vector<MeasurementInterp> interpolate(const std::vector<Measurement>& measurements);

    static MeasurementInterp interpMeasurement(const Eigen::Vector2d& pos, 
                                               const boost::posix_time::ptime& t, 
                                               const Measurement& mm0, 
                                               const Measurement& mm1, 
                                               double interp_factor,
                                               CoordSystem coord_sys);
    static Eigen::VectorXd interpStateVector(const Eigen::VectorXd& x0, 
                                             const Eigen::VectorXd& x1, 
                                             double interp_factor);
    static Eigen::MatrixXd interpCovarianceMat(const Eigen::MatrixXd& C0, 
                                               const Eigen::MatrixXd& C1, 
                                               double interp_factor);
    static void interpCovarianceMat(Measurement& mm_interp,
                                    const Measurement& mm0,
                                    const Measurement& mm1,
                                    double interp_factor);
    
    static size_t estimatedSamples(const Measurement& mm0, 
                                   const Measurement& mm1,
                                   double dt);
protected:
    std::vector<MeasurementInterp> interpolatePart(const std::vector<Measurement>& measurements) const;
    bool isFishySegment(const Measurement& mm0, 
                        const Measurement& mm1, 
                        const std::vector<MeasurementInterp>& segment,
                        size_t n) const;

    MeasurementInterp generateMeasurement(const Eigen::Vector2d& pos, 
                                          const boost::posix_time::ptime& t, 
                                          const Measurement& mm0, 
                                          const Measurement& mm1, 
                                          double interp_factor,
                                          bool corrected) const;
    MeasurementInterp generateMeasurement(const Measurement& mm) const;

    void finalizeInterp(std::vector<MeasurementInterp>& samples, const Measurement& mm_last) const;

    std::vector<MeasurementInterp> interpolateLinear(const std::vector<Measurement>& measurements) const;

    CoordSystem coordSystem() const;
    Eigen::Vector2d position2D(const Measurement& mm) const;
    void position2D(Measurement& mm, const Eigen::Vector2d& pos) const;

private:
    Config config_;
};

} // namespace reconstruction
