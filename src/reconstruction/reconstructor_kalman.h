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
#include "reconstructor_kalman_projection.h"
#include "kalman_defs.h"

#include <Eigen/Core>

namespace reconstruction
{

/**
 * A connected chain of kalman reference updates and respective kalman states.
 */
struct KalmanChain
{
    bool empty() const
    {
        return references.empty();
    }

    void reserve(size_t n)
    {
        references.reserve(n);
        kalman_states.reserve(n);
        proj_centers.reserve(n);
    }

    void add(const KalmanChain& other)
    {
        references.insert(references.end(), other.references.begin(), other.references.end());
        kalman_states.insert(kalman_states.end(), other.kalman_states.begin(), other.kalman_states.end());
        proj_centers.insert(proj_centers.end(), other.proj_centers.begin(), other.proj_centers.end());
    }

    void add(const Reference& ref, 
             const kalman::KalmanState& state,
             const Eigen::Vector2d& proj_center)
    {
        references.push_back(ref);
        kalman_states.push_back(state);
        proj_centers.push_back(proj_center);
    }

    void pop_back()
    {
        references.pop_back();
        kalman_states.pop_back();
        proj_centers.pop_back();
    }

    size_t size() const
    {
        return references.size();
    }

    std::vector<Reference>           references;
    std::vector<kalman::KalmanState> kalman_states;
    std::vector<Eigen::Vector2d>     proj_centers;
};

/**
 * Base class for kalman-based reconstructors.
*/
class ReconstructorKalman : public Reconstructor
{
public:
    struct BaseConfig
    {
        double Q_std      = 10.0;   // process noise
        double R_std      = 30.0;   // observation noise (standard)
        double R_std_high = 1000.0; // observation noise (high)
        double P_std      = 30.0;   // system noise (standard)
        double P_std_high = 1000.0; // system noise (high)

        boost::optional<kalman::Matrix>           init_cov;    // initial covariance for first measurement        (optional)
        boost::optional<boost::posix_time::ptime> last_update; // last input update to be returned as a reference (optional)

        size_t min_chain_size = 2;       // minimum number of connected points used as result (and input for RTS smoother)
        double max_distance   = 50000.0; // maximum allowed distance of consecutive target reports in meters (0 = do not check)
        double min_dt         = 1e-06;   // minimum allowed time difference of consecutive target reports in seconds (0 = do not check)
        double max_dt         = 30.0;    // maximum allowed time difference of consecutive target reports in seconds (0 = do not check)

        bool smooth = true; // use RTS smoother

        bool            resample_result = false;                      // resample result references using kalman infos
        double          resample_dt     = 1.0;                        // resampling step size in seconds
        double          resample_Q_std  = 10.0;                       // resampling process noise
        StateInterpMode interp_mode     = StateInterpMode::BlendVar;  // kalman state interpolation mode used during resampling

        MapProjectionMode map_proj_mode          = MapProjectionMode::MapProjectDynamic; // map projection mode
        double            max_proj_distance_cart = 20000.0;                              // maximum distance from the current map projection origin in meters 
                                                                                         // before changing the projection center
    };

    ReconstructorKalman();
    virtual ~ReconstructorKalman() = default;

    BaseConfig& baseConfig() { return base_config_; }
    const BaseConfig& baseConfig() const { return base_config_; }

    double qVar() const { return Q_var_; }
    double rVar() const { return R_var_; }
    double rVarHigh() const { return R_var_high_; }
    double pVar() const { return P_var_; }
    double pVarHigh() const  { return P_var_high_; }
    double qVarResample() const { return resample_Q_var_; }

    static const QColor ColorKalman;
    static const QColor ColorKalmanSmoothed;
    static const QColor ColorKalmanResampled;

    static const float  SpeedVecLineWidth;

protected:
    boost::optional<std::vector<Reference>> reconstruct_impl(std::vector<Measurement>& measurements,
                                                             const std::string& data_info) override final;
    virtual void init_impl() {};
    virtual kalman::KalmanState kalmanState() const = 0;
    virtual boost::optional<kalman::KalmanState> kalmanStep(double dt, const Measurement& mm) = 0;
    virtual kalman::Vector xVec(const Measurement& mm) const = 0;
    virtual kalman::Vector xVecInv(const kalman::Vector& x) const = 0;
    virtual void xVec(const kalman::Vector& x) const = 0;
    virtual void xPos(double& x, double& y, const kalman::Vector& x_vec) const = 0;
    virtual void xPos(kalman::Vector& x_vec, double x, double y) const = 0;
    virtual void xPos(kalman::Vector& x, const Measurement& mm) const;
    virtual kalman::Matrix pMat(const Measurement& mm) const = 0;
    virtual kalman::Vector zVec(const Measurement& mm) const = 0;
    virtual double xVar(const kalman::Matrix& P) const = 0;
    virtual double yVar(const kalman::Matrix& P) const = 0; 
    
    virtual void storeState_impl(Reference& ref, const kalman::KalmanState& state) const = 0;
    virtual void init_impl(const Measurement& mm) const = 0;
    
    virtual boost::optional<kalman::KalmanState> interpStep(const kalman::KalmanState& state0,
                                                            const kalman::KalmanState& state1,
                                                            double dt) const = 0;
    virtual bool smoothChain_impl(std::vector<kalman::Vector>& x_smooth,
                                  std::vector<kalman::Matrix>& P_smooth,
                                  const KalmanChain& chain,
                                  const kalman::XTransferFunc& x_tr) const = 0;
    
    reconstruction::Uncertainty defaultUncertaintyOfMeasurement(const Measurement& mm) const;

private:
    friend class RecKalmanProjectionHandler;

    enum ReinitFlags
    {
        ReinitCheckTime     = 1 << 0, //checks for reinit using a timestep threshold
        ReinitCheckDistance = 1 << 1  //checks for reinit using a metric distance-based threshold
    };

    void init();
    boost::optional<std::vector<Reference>> finalize();

    //measurement & reference related
    const Reference& lastReference() const;
    void newChain();
    void addReference(Reference& ref,
                      kalman::KalmanState& state,
                      const std::string& data_info);
    void storeState(Reference& ref,
                    const kalman::KalmanState& state) const;
    Reference storeState(const kalman::KalmanState& state,
                         const Measurement& mm) const;
    void initMeasurement(Measurement& mm) const;
    void finalizeReference(Reference& ref,
                           kalman::KalmanState& state,
                           const std::string& data_info) const;

    //kalman init related
    bool needsReinit(const Reference& ref, 
                     const Measurement& mm,
                     int flags,
                     std::string* reason = nullptr) const;
    void init(Measurement& mm, const std::string& data_info, bool first_mm = false);
    bool reinitIfNeeded(Measurement& mm, 
                        const std::string& data_info,
                        int flags);

    //map projection related
    virtual void postProjectionChange(const kalman::KalmanState& state) const;
    
    //kalman chain operations
    bool smoothChain(KalmanChain& chain);
    bool filterChain(KalmanChain& chain, const boost::posix_time::ptime& t_last);
    bool resampleResult(KalmanChain& result_chain, double dt_sec);

    //various
    double timestep(const Measurement& mm) const;

    BaseConfig base_config_;

    size_t min_chain_size_;
    double Q_var_;
    double R_var_;
    double R_var_high_;
    double P_var_;
    double P_var_high_;
    double resample_Q_var_;
    double max_distance_sqr_ = 0.0;
    
    std::vector<KalmanChain> chains_;
    KalmanChain              chain_cur_;

    mutable RecKalmanProjectionHandler proj_handler_;
};

} // namespace reconstruction
