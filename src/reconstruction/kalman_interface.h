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

#include "kalman_defs.h"

#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace kalman
{
    class KalmanFilter;
}

namespace reconstruction
{

struct Measurement;
struct Uncertainty;
struct KalmanUpdate;

class KalmanProjectionHandler;

/**
 * Provides needed data structures for kalman and means to retrieve data from kalman state. 
 * Derive for a certain kalman variant.
 */
class KalmanInterface
{
public:
    KalmanInterface(kalman::KalmanFilter* kalman_filter);
    virtual ~KalmanInterface();

    virtual KalmanInterface* clone() const = 0;

    std::string asString(int flags, 
                         const std::string& prefix = "") const;

    const boost::posix_time::ptime& currrentTime() const { return ts_; }

    Measurement currentStateAsMeasurement() const;
    kalman::KalmanState currentState() const;

    //initialize kalman state
    void kalmanInit(kalman::KalmanState& init_state,
                    const Measurement& mm,
                    const reconstruction::Uncertainty& default_uncert,
                    double Q_var);
    void kalmanInit(const kalman::KalmanState& init_state,
                    const boost::posix_time::ptime& ts);
    void kalmanInit(const kalman::Vector& x,
                    const kalman::Matrix& P,
                    const boost::posix_time::ptime& ts);
    
    //integrate measurement and obtain next kalman state
    kalman::KalmanError kalmanStep(kalman::KalmanState& new_state,
                                   const Measurement& mm, 
                                   const reconstruction::Uncertainty& default_uncert, 
                                   double Q_var);
    
    //predict kalman state
    kalman::KalmanError kalmanPrediction(kalman::Vector& x,
                                         kalman::Matrix& P,
                                         double dt,
                                         double Q_var,
                                         bool fix_estimate,
                                         bool* fixed = nullptr) const;
    kalman::KalmanError kalmanPrediction(kalman::Vector& x,
                                         kalman::Matrix& P,
                                         const boost::posix_time::ptime& ts,
                                         double Q_var,
                                         bool fix_estimate,
                                         bool* fixed = nullptr) const;

    //smooth kalman updates  
    bool smoothUpdates(std::vector<kalman::KalmanUpdate>& updates,
                       size_t idx0,
                       size_t idx1,
                       KalmanProjectionHandler& proj_handler,
                       double smooth_scale = 1.0,
                       kalman::SmoothFailStrategy fail_strategy = kalman::SmoothFailStrategy::Stop) const; 

    //interpolation of kalman states
    kalman::KalmanError interpStep(kalman::KalmanState& state_interp,
                                   const kalman::KalmanState& state0,
                                   const kalman::KalmanState& state1,
                                   double dt,
                                   double Q_var,
                                   bool fix_estimate,
                                   bool* fixed = nullptr) const;
    //kalman state integrity
    bool checkKalmanState(kalman::KalmanState& state) const;
    
    //needed for feeding kalman
    void stateVecX(const kalman::Vector& x);

    virtual void stateVecXFromMM(kalman::Vector& x, const Measurement& mm) const = 0;
    virtual void measurementVecZ(kalman::Vector& z, const Measurement& mm) const = 0;
    virtual void covarianceMatP(kalman::Matrix& P,
                                const Measurement& mm, 
                                const reconstruction::Uncertainty& default_uncert) const = 0;
    virtual void measurementUncertMatR(kalman::Matrix& R, 
                                       const Measurement& mm, 
                                       const reconstruction::Uncertainty& default_uncert) const = 0;
    
    //needed for retrieval from kalman
    void storeState(Measurement& mm, 
                    const kalman::KalmanState& state) const;
    virtual void storeState(Measurement& mm, 
                            const kalman::Vector& x, 
                            const kalman::Matrix& P) const = 0;
    //helpers
    double timestep(const Measurement& mm) const;
    static double timestep(const Measurement& mm0, const Measurement& mm1);
    static double timestep(const boost::posix_time::ptime& ts0, const boost::posix_time::ptime& ts1);
    double distanceSqr(const Measurement& mm) const;

    void xPos(double& x, double& y) const;
    virtual void xPos(double& x, double& y, const kalman::Vector& x_vec) const = 0;
    virtual void xPos(kalman::Vector& x_vec, double x, double y) const = 0;
    virtual double xVar(const kalman::Matrix& P) const = 0;
    virtual double yVar(const kalman::Matrix& P) const = 0;
    virtual double xyCov(const kalman::Matrix& P) const = 0;

    void setVerbosity(int v) { verbosity_ = v; }

protected:
    int verbosity() const { return verbosity_; }

private:
    boost::posix_time::ptime ts_;

    int verbosity_ = 0;

    std::unique_ptr<kalman::KalmanFilter> kalman_filter_;
};

} // reconstruction
