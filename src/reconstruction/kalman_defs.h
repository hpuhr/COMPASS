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

#include <Eigen/Core>

#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace kalman
{

enum KalmanType
{
    UMKalman2D     = 0, // uniform motion kalman in the plane
    AMKalman2D,         // accelerated motion kalman in the plane
    IMMKalman2D         // mixed kalman model filter in the plane
};

enum SmoothFailStrategy
{
    None = 0,
    Stop,
    SetInvalid
};

enum class KalmanError
{
    NoError = 0,
    Numeric,
    InvalidState,
    Unknown
};

enum KalmanInfoFlags
{
    InfoState        = 1 << 0,
    InfoStateExt     = 1 << 1,
    InfoIntermSteps  = 1 << 2,
    InfoAll          = 255
};

typedef Eigen::MatrixXd         Matrix;
typedef Eigen::VectorXd         Vector;
typedef boost::optional<Matrix> OMatrix;
typedef boost::optional<Vector> OVector;

/**
 * Transfer func for a state vector.
 * Used to transfer the given state vector between two collected kalman states.
 * This functional is kept very general on purpose and can be used e.g. to convert from
 * one map projection system to another one.
 * params: state vec transformed | state vec | index of the old kalman state | index of the new kalman state
 */
typedef std::function<void(Vector&,const Vector&,size_t,size_t)> XTransferFunc;

/**
*/
struct KalmanState
{
    struct IMMState
    {
        std::vector<KalmanState> filter_states;

        Vector mu;
        Matrix omega;
        Vector cbar;
    };

    KalmanState() {}
    KalmanState(const Vector& _x, const Matrix& _P) : x(_x), P(_P) {}
    KalmanState(const Vector& _x, const Matrix& _P, double _dt) : dt(_dt), x(_x), P(_P) {}
    KalmanState(const KalmanState& other)
    :   dt(other.dt)
    ,   x (other.x )
    ,   P (other.P )
    ,   F (other.F )
    ,   Q (other.Q )
    {
        if (other.imm_state)
        {
            imm_state.reset(new IMMState);
            *imm_state = *other.imm_state;
        }
    }
    KalmanState(KalmanState&& other)
    :   dt(other.dt)
    ,   x (other.x )
    ,   P (other.P )
    ,   F (other.F )
    ,   Q (other.Q )
    {
        if (other.imm_state)
            imm_state = std::move(other.imm_state);
    }

    KalmanState& operator=(const KalmanState& other)
    {
        dt = other.dt;
        x  = other.x;
        P  = other.P;
        F  = other.F;
        Q  = other.Q;

        if (other.imm_state)
            immState() = *other.imm_state;
        else if (imm_state)
            imm_state.reset();

        return *this;
    }

    KalmanState& operator=(KalmanState&& other)
    {
        dt = other.dt;
        x  = other.x;
        P  = other.P;
        F  = other.F;
        Q  = other.Q;

        if (other.imm_state)
            imm_state = std::move(other.imm_state);
        else if (imm_state)
            imm_state.reset();

        return *this;
    }

    IMMState& immState()
    {
        if (!imm_state)
            imm_state.reset(new IMMState);
        return *imm_state;
    }

    double dt = 0.0; // used timestep
    Vector x;        // state
    Matrix P;        // state uncertainty
    Matrix F;        // transition mat
    Matrix Q;        // process noise

    std::unique_ptr<IMMState> imm_state;
};

/**
 * Minimal information about a kalman update's state, coord system and validity.
 */
struct KalmanUpdateMinimal
{
    void resetFlags()
    {
        valid = false;
    }

    Vector                   x;                 // state vector
    Matrix                   P;                 // covariance matrix
    boost::posix_time::ptime t;                 // time of update
    Eigen::Vector2d          projection_center; // center of the local stereographic projection used for this update
    double                   lat;               // latitude of state position (do not confuse with center of local projection!)
    double                   lon;               // longitude of state position (do not confuse with center of local projection!)

    bool has_wgs84_pos = false; // lat/long are set
    bool valid         = false; // kalman update is valid
};

/**
 * Low-level struct for a kalman update.
 * Contains everything needed for postprocessing and extraction of a final reference.
 */
struct KalmanUpdate
{
    KalmanUpdate() {}
    KalmanUpdate(const KalmanUpdateMinimal& update_min)
    {
        state.x  = update_min.x;
        state.P  = update_min.P;
        state.F  = kalman::Matrix();
        state.Q  = kalman::Matrix();
        state.dt = 0.0; // not defined by min update

        projection_center = update_min.projection_center;
        lat               = update_min.lat;
        lon               = update_min.lon;
        t                 = update_min.t;
        has_wgs84_pos     = update_min.has_wgs84_pos;
        valid             = update_min.valid;
        reinit            = false; // not defined by min update
    }

    void resetFlags()
    {
        has_wgs84_pos = false;
        valid         = false;
        reinit        = false;
    }

    void minimalInfo(KalmanUpdateMinimal& info) const
    {
        info.x                 = state.x;
        info.P                 = state.P;
        info.t                 = t;
        info.projection_center = projection_center;
        info.lat               = lat;
        info.lon               = lon;
        info.has_wgs84_pos     = has_wgs84_pos;
        info.valid             = valid;
    };

    KalmanUpdateMinimal minimalInfo() const
    {
        KalmanUpdateMinimal info;
        minimalInfo(info);

        return info;
    }

    KalmanState              state;             // kalman internal state, can be used for rts smooting, state interpolation, etc.
    Eigen::Vector2d          projection_center; // center of the local stereographic projection used for this update
    double                   lat;               // latitude of state position (do not confuse with center of local projection!)
    double                   lon;               // longitude of state position (do not confuse with center of local projection!)
    boost::posix_time::ptime t;                 // time of update

    bool has_wgs84_pos = false; // lat/long are set
    bool valid         = false; // kalman update is valid
    bool reinit        = false; // kalman was reinitialized at this update, represents the beginning of a new kalman chain
};

} // namespace kalman
