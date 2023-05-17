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
#include "kalman_defs.h"

#include <Eigen/Core>

namespace reconstruction
{

/**
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
    }

    void add(const KalmanChain& other)
    {
        references.insert(references.end(), other.references.begin(), other.references.end());
        kalman_states.insert(kalman_states.end(), other.kalman_states.begin(), other.kalman_states.end());
    }

    void add(const Reference& ref, const kalman::KalmanState& state)
    {
        references.push_back(ref);
        kalman_states.push_back(state);
    }

    std::vector<Reference>           references;    //reconstructed positions
    std::vector<kalman::KalmanState> kalman_states; //data needed for RTS smoother
};

/**
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

        bool   smooth         = true; // use RTS smoother
        double smooth_scale   = 1.0;  // scale factor for RTS smoothing
        size_t min_chain_size = 2;    // minimum number of connected points used as result (and input for RTS smoother)

        double max_distance = 0.0;    // maximum allowed distance of consecutive target reports in meters (0 = do not check)
        double min_dt       = 1e-06;  // minimum allowed time difference of consecutive target reports in seconds (0 = do not check)
        double max_dt       = 60.0;   // maximum allowed time difference of consecutive target reports in seconds (0 = do not check)

        bool   resample_result = false; //resample result references using kalman infos
        double resample_dt     = 1.0;   //resampling step size in seconds
    };

    ReconstructorKalman() = default;
    virtual ~ReconstructorKalman() = default;

    BaseConfig& baseConfig() { return base_config_; }
    const BaseConfig& baseConfig() const { return base_config_; }

    double qVar() const { return Q_var_; }
    double rVar() const { return R_var_; }
    double rVarHigh() const { return R_var_high_; }
    double pVar() const { return P_var_; }
    double pVarHigh() const  { return P_var_high_; }

    static const QColor ColorKalman;
    static const QColor ColorKalmanSmoothed;
    static const QColor ColorKalmanResampled;

protected:
    boost::optional<std::vector<Reference>> reconstruct_impl(const std::vector<Measurement>& measurements,
                                                             const std::string& data_info) override final;
    virtual void init_impl() {};
    virtual kalman::KalmanState kalmanState() const = 0;
    virtual boost::optional<kalman::KalmanState> kalmanStep(double dt, const Measurement& mm) = 0;
    virtual kalman::Vector xVec(const Measurement& mm) const = 0;
    virtual kalman::Matrix pMat(const Measurement& mm) const = 0;
    virtual kalman::Vector zVec(const Measurement& mm) const = 0;
    virtual void storeState_impl(Reference& ref,
                                 const kalman::KalmanState& state) const = 0;
    virtual void init_impl(const Measurement& mm) const = 0;
    virtual boost::optional<kalman::KalmanState> interpStep(const kalman::KalmanState& state0,
                                                            const kalman::KalmanState& state1,
                                                            double dt) const = 0;
    virtual bool smoothChain_impl(std::vector<kalman::Vector>& x_smooth,
                                  std::vector<kalman::Matrix>& P_smooth,
                                  const KalmanChain& chain) const = 0;

    reconstruction::Uncertainty defaultUncertaintyOfMeasurement(const Measurement& mm) const;

private:
    void init();
    boost::optional<std::vector<Reference>> finalize();

    const Reference& lastReference() const;

    void newChain();
    void addReference(const Reference& ref,
                      const kalman::KalmanState& rts_info);

    void storeState(Reference& ref,
                    const kalman::KalmanState& state) const;
    Reference storeState(const kalman::KalmanState& state,
                         const Measurement& mm) const;

    bool needsReinit(const Reference& ref, const Measurement& mm) const;
    void init(const Measurement& mm);
    bool reinitIfNeeded(const Measurement& mm, const std::string& data_info);

    double timestep(const Measurement& mm) const;

    bool smoothChain(KalmanChain& chain);
    bool resampleResult(KalmanChain& result_chain, double dt_sec);

    BaseConfig base_config_;

    size_t min_chain_size_;
    double Q_var_;
    double R_var_;
    double R_var_high_;
    double P_var_;
    double P_var_high_;

    std::vector<KalmanChain> chains_;
    KalmanChain              chain_cur_;
};

} // namespace reconstruction
