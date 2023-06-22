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

#include "reconstructor_kalman.h"
#include "kalman.h"
#include "logger.h"
#include "spline_interpolator.h"
#include "util/timeconv.h"
#include "viewpointgenerator.h"

namespace reconstruction
{

const QColor ReconstructorKalman::ColorKalman          = QColor(255, 102, 178);
const QColor ReconstructorKalman::ColorKalmanSmoothed  = QColor(102, 178, 255);
const QColor ReconstructorKalman::ColorKalmanResampled = QColor(255, 178, 102);

const float  ReconstructorKalman::SpeedVecLineWidth    = 3.0f;

/**
*/
ReconstructorKalman::ReconstructorKalman()
:   proj_handler_(this)
{
}

/**
*/
void ReconstructorKalman::init()
{
    //reset chains
    chains_.clear();
    chain_cur_ = {};

    //cache some values
    Q_var_      = base_config_.Q_std      * base_config_.Q_std;
    R_var_      = base_config_.R_std      * base_config_.R_std;
    R_var_high_ = base_config_.R_std_high * base_config_.R_std_high;
    P_var_      = base_config_.P_std      * base_config_.P_std;
    P_var_high_ = base_config_.P_std_high * base_config_.P_std_high;

    min_chain_size_   = std::max((size_t)1, base_config_.min_chain_size);
    max_distance_sqr_ = base_config_.max_distance * base_config_.max_distance;

    //configure projection 
    proj_handler_.settings().map_proj_mode          = base_config_.map_proj_mode;
    proj_handler_.settings().proj_max_dist_cart_sqr = base_config_.max_proj_distance_cart * 
                                                      base_config_.max_proj_distance_cart;

    init_impl(); //invoke derived impl
}

/**
 * Smoothes the given kalman chain.
 */
bool ReconstructorKalman::smoothChain(KalmanChain& chain)
{
    if (chain.references.empty())
    {
        //loginf << "no references to smooth...";
        return true;
    }
    if (chain.references.size() != chain.kalman_states.size())
    {
        //loginf << "not enough kalman states: " << chain.kalman_states.size() << "/" << chain.references.size();
        return false;
    }

    if (verbosity() > 1)
    {
        loginf << "Obtains " << chain.kalman_states.size() << " info(s)";

        int cnt = 0;
        for (const auto& state : chain.kalman_states)
        {
            loginf << "Info " << cnt++ << "\n"
                   << "    x: " << state.x << "\n"
                   << "    P: " << state.P << "\n"
                   << "    Q: " << state.Q << "\n"
                   << "    F: " << state.F << "\n";
        }
    }

    //loginf << "Smoothing chain...";

    std::vector<kalman::Vector> x_smooth;
    std::vector<kalman::Matrix> P_smooth;

    if (!smoothChain_impl(x_smooth, P_smooth, chain, proj_handler_.reprojectionTransform(chain)))
        return false;

    //update chain data
    for (size_t i = 0; i < chain.references.size(); ++i)
    {
        storeState(chain.references[ i ], kalman::KalmanState(x_smooth[ i ], P_smooth[ i ]));

        chain.kalman_states[ i ].P = P_smooth[ i ];
        chain.kalman_states[ i ].x = x_smooth[ i ];
    }
    
    return true;
}

/**
*/
bool ReconstructorKalman::resampleResult(KalmanChain& result_chain, double dt_sec)
{
    if (result_chain.references.size() < 2)
        return true;

    size_t n = result_chain.references.size();

    size_t n_estim = SplineInterpolator::estimatedSamples(result_chain.references.front(), 
                                                          result_chain.references.back(),
                                                          dt_sec);
    KalmanChain resampled_chain;
    resampled_chain.reserve(n_estim);

    resampled_chain.add(result_chain.references.front(), 
                        result_chain.kalman_states.front(),
                        result_chain.proj_centers.front());

    boost::posix_time::milliseconds time_incr((int)(dt_sec * 1000.0));

    auto tcur = result_chain.references.front().t + time_incr;

    size_t small_intervals = 0;

    auto x_tr_func = proj_handler_.reprojectionTransform(result_chain);

    kalman::KalmanState state1_tr;

    auto blendFunc = [ & ] (double dt0, double dt1, double dt)
    {
        if (base_config_.interp_mode == StateInterpMode::BlendLinear)
            return dt0 / dt;

        //StateInterpMode::BlendHalf
        return 0.5;
    };

    for (size_t i = 1; i < n; ++i)
    {
        const auto& ref0         = result_chain.references   [i - 1];
        const auto& ref1         = result_chain.references   [i    ];
        const auto& state0       = result_chain.kalman_states[i - 1];
        const auto& state1       = result_chain.kalman_states[i    ];
        const auto& proj_center0 = result_chain.proj_centers [i - 1];
        const auto& proj_center1 = result_chain.proj_centers [i    ];

        auto t0 = ref0.t;
        auto t1 = ref1.t;

        if (tcur < t0 || tcur >= t1)
            continue;

        //transfer state1 to map projection of state0?
        if (x_tr_func)
        { 
            state1_tr = state1;
            x_tr_func(state1_tr.x, state1.x, i, i - 1);
        }
        const auto& state1_ref = x_tr_func ? state1_tr : state1;

        const double dt = Utils::Time::partialSeconds(t1 - t0);

        while (tcur >= t0 && tcur < t1)
        {
            double dt0 = Utils::Time::partialSeconds(tcur - t0);
            double dt1 = Utils::Time::partialSeconds(t1 - tcur);
            if (dt0 <= base_config_.min_dt)
            {
                //use first ref if timestep from first ref is very small
                resampled_chain.add(ref0, state0, proj_center0);
                ++small_intervals;
                tcur = t0 + time_incr;
                continue;
            }
            if (dt1 <= base_config_.min_dt)
            {
                //use second ref if timestep from second ref is very small
                resampled_chain.add(ref1, state1, proj_center1);
                ++small_intervals;
                tcur = t1 + time_incr;
                continue;
            }

            auto new_state0 = interpStep(state0, state1_ref,  dt0);
            auto new_state1 = interpStep(state1_ref, state0, -dt1);

            if (!new_state0.has_value() || !new_state1.has_value())
                return false;

            double interp_factor = blendFunc(dt0, dt1, dt);

            kalman::KalmanState new_state;
            new_state.x = SplineInterpolator::interpStateVector(new_state0->x, new_state1->x, interp_factor);
            new_state.P = SplineInterpolator::interpCovarianceMat(new_state0->P, new_state1->P, interp_factor);

            Reference ref;
            ref.t              = tcur;
            ref.source_id      = ref0.source_id;
            ref.noaccel_pos    = ref0.noaccel_pos || ref1.noaccel_pos;
            ref.nospeed_pos    = ref0.nospeed_pos || ref1.nospeed_pos;
            ref.nostddev_pos   = ref0.nostddev_pos || ref1.nostddev_pos;
            ref.mm_interp      = ref0.mm_interp || ref1.mm_interp;
            ref.projchange_pos = ref1.projchange_pos; //if a change in map projection happened between the two reference states, 
                                                      //this should be logged in the second reference
            ref.ref_interp     = true;

            storeState(ref, new_state);

            resampled_chain.add(ref, new_state, proj_center0); //!map projection of state0 used!

            tcur += time_incr;
        }
    }

    if (verbosity() >= 1 && small_intervals > 0)
        logdbg << "Encountered " << small_intervals << " small interval(s) during resampling";

    if (resampled_chain.size() >= 2)
    {
        resampled_chain.pop_back();
        resampled_chain.add(result_chain.references.back(), 
                            result_chain.kalman_states.back(),
                            result_chain.proj_centers.back());
    }

    result_chain = std::move(resampled_chain);

    return true;
}

/**
 * Finalizes and combines the collected chains.
 */
boost::optional<std::vector<Reference>> ReconstructorKalman::finalize()
{
    KalmanChain result_chain;

    //init viewpoint structures
    std::unique_ptr<ViewPointGenFeaturePoints>      feat_points_kalman;
    std::unique_ptr<ViewPointGenFeatureErrEllipses> feat_errors_kalman;
    std::unique_ptr<ViewPointGenFeatureLineString>  feat_lines_kalman;
    std::unique_ptr<ViewPointGenFeatureLines>       feat_speeds_kalman;

    std::unique_ptr<ViewPointGenFeaturePoints>      feat_points_kalman_smoothed;
    std::unique_ptr<ViewPointGenFeatureErrEllipses> feat_errors_kalman_smoothed;
    std::unique_ptr<ViewPointGenFeatureLineString>  feat_lines_kalman_smoothed;
    std::unique_ptr<ViewPointGenFeatureLines>       feat_speeds_kalman_smoothed;

    std::unique_ptr<ViewPointGenFeaturePoints>      feat_points_kalman_resampled;
    std::unique_ptr<ViewPointGenFeatureErrEllipses> feat_errors_kalman_resampled;
    std::unique_ptr<ViewPointGenFeatureLineString>  feat_lines_kalman_resampled;
    std::unique_ptr<ViewPointGenFeatureLines>       feat_speeds_kalman_resampled;

    if (hasViewPoint())
    {
        feat_points_kalman.reset(new ViewPointGenFeaturePoints);
        feat_errors_kalman.reset(new ViewPointGenFeatureErrEllipses);
        feat_lines_kalman.reset(new ViewPointGenFeatureLineString(false));
        feat_speeds_kalman.reset(new ViewPointGenFeatureLines(SpeedVecLineWidth));

        feat_points_kalman->setColor(ColorKalman);
        feat_errors_kalman->setColor(ColorKalman);
        feat_lines_kalman->setColor(ColorKalman);
        feat_speeds_kalman->setColor(ColorKalman);

        if (base_config_.smooth)
        {
            feat_points_kalman_smoothed.reset(new ViewPointGenFeaturePoints);
            feat_errors_kalman_smoothed.reset(new ViewPointGenFeatureErrEllipses);
            feat_lines_kalman_smoothed.reset(new ViewPointGenFeatureLineString(false));
            feat_speeds_kalman_smoothed.reset(new ViewPointGenFeatureLines(SpeedVecLineWidth));

            feat_points_kalman_smoothed->setColor(ColorKalmanSmoothed);
            feat_errors_kalman_smoothed->setColor(ColorKalmanSmoothed);
            feat_lines_kalman_smoothed->setColor(ColorKalmanSmoothed);
            feat_speeds_kalman_smoothed->setColor(ColorKalmanSmoothed);
        }

        if (base_config_.resample_result)
        {
            feat_points_kalman_resampled.reset(new ViewPointGenFeaturePoints);
            feat_errors_kalman_resampled.reset(new ViewPointGenFeatureErrEllipses);
            feat_lines_kalman_resampled.reset(new ViewPointGenFeatureLineString(false));
            feat_speeds_kalman_resampled.reset(new ViewPointGenFeatureLines(SpeedVecLineWidth));

            feat_points_kalman_resampled->setColor(ColorKalmanResampled);
            feat_errors_kalman_resampled->setColor(ColorKalmanResampled);
            feat_lines_kalman_resampled->setColor(ColorKalmanResampled);
            feat_speeds_kalman_resampled->setColor(ColorKalmanResampled);
        }
    }

    auto addReferences = [&] (std::unique_ptr<ViewPointGenFeaturePoints>& points,
                              std::unique_ptr<ViewPointGenFeatureErrEllipses>& errors,
                              std::unique_ptr<ViewPointGenFeatureLineString>& lines,
                              std::unique_ptr<ViewPointGenFeatureLines>& speeds,
                              const KalmanChain& chain)
    {
        points->reserve(points->size() + chain.size(), false);
        errors->reserve(errors->size() + chain.size(), false);
        lines->reserve(lines->size() + chain.size(), false);
        speeds->reserve(speeds->size() + chain.size() * 2, false);

        Eigen::Vector2d pos_geod;
        Eigen::Vector2d pos_geod_tip;

        for (size_t i = 0; i < chain.size(); ++i)
        {
            const auto& ref         = chain.references  [ i ];
            const auto& proj_center = chain.proj_centers[ i ];

            proj_handler_.unproject(pos_geod[ 0 ], pos_geod[ 1 ], ref.x, ref.y, proj_center);
            
            points->addPoint(pos_geod);
            errors->addPoint(pos_geod);
            errors->addSize(Eigen::Vector3d(ref.x_stddev.value(), ref.y_stddev.value(), ref.xy_cov.value()));
            lines->addPoint(pos_geod);

            if (ref.hasVelocity())
            {
                proj_handler_.unproject(pos_geod_tip[ 0 ], 
                                        pos_geod_tip[ 1 ], 
                                        ref.x + ref.vx.value(), 
                                        ref.y + ref.vy.value(), 
                                        proj_center);

                speeds->addPoint(pos_geod);
                speeds->addPoint(pos_geod_tip);
            }
        }
    };

    auto addAnnotation = [&] (const std::string& name,
                              const QColor& color,
                              std::unique_ptr<ViewPointGenFeaturePoints>& points,
                              std::unique_ptr<ViewPointGenFeatureErrEllipses>& errors,
                              std::unique_ptr<ViewPointGenFeatureLineString>& lines,
                              std::unique_ptr<ViewPointGenFeatureLines>& speeds)
    {
        auto anno_group = viewPoint()->annotations().addAnnotation(name);

        anno_group->setSymbolColor(color);

        auto anno_errors = anno_group->addAnnotation("Errors");
        auto anno_points = anno_group->addAnnotation("Positions");
        auto anno_lines  = anno_group->addAnnotation("Connection Lines");
        auto anno_speeds = anno_group->addAnnotation("Speeds");

        anno_errors->setSymbolColor(color);
        anno_points->setSymbolColor(color);
        anno_lines->setSymbolColor(color);
        anno_speeds->setSymbolColor(color);

        anno_points->addFeature(std::move(points));
        anno_errors->addFeature(std::move(errors));
        anno_lines->addFeature(std::move(lines));
        anno_speeds->addFeature(std::move(speeds));
    };

    //collect (optionally smoothed) results
    for (auto& c : chains_)
    {
        //does chain obtain enough references?
        if (c.references.size() < min_chain_size_)
        {
            if (verbosity() > 0)
                loginf << "Dropping chain of " << c.references.size() << " point(s)";
            
            continue;
        }
            
        if (verbosity() > 0)
            loginf << "Adding chain of " << c.references.size() << " point(s)";

        //add unsmoothed positions to viewpoint
        if (hasViewPoint())
        {
            addReferences(feat_points_kalman, 
                          feat_errors_kalman, 
                          feat_lines_kalman, 
                          feat_speeds_kalman, 
                          c);
        }

        //apply RTS smoother?
        if (base_config_.smooth)
        {
            if (!smoothChain(c))
            {
                logerr << "ReconstructorKalman::finalize(): RTS smoother failed";
                return {};
            }

            //add smoothed positions to viewpoint
            if (hasViewPoint())
            {
                addReferences(feat_points_kalman_smoothed, 
                              feat_errors_kalman_smoothed, 
                              feat_lines_kalman_smoothed, 
                              feat_speeds_kalman_smoothed, 
                              c);
            }
        }

        if (base_config_.resample_result)
        {
            if (!resampleResult(c, base_config_.resample_dt))
            {
                logerr << "ReconstructorKalman::finalize(): Resampling failed";
                return {};
            }

            //add resampled positions to viewpoint
            if (hasViewPoint())
            {
                addReferences(feat_points_kalman_resampled, 
                              feat_errors_kalman_resampled, 
                              feat_lines_kalman_resampled, 
                              feat_speeds_kalman_resampled,
                              c);
            }
        }

        //obtain geodetic coords
        proj_handler_.unproject(c.references, c.proj_centers);

        //add to result chain
        result_chain.add(c);
    }

    //add collected vp features to viewpoint
    if (hasViewPoint())
    {
        if (base_config_.resample_result)
        {
            addAnnotation("Kalman (resampled)", 
                          ColorKalmanResampled,
                          feat_points_kalman_resampled, 
                          feat_errors_kalman_resampled, 
                          feat_lines_kalman_resampled,
                          feat_speeds_kalman_resampled);
        }
        if (base_config_.smooth)
        {
            addAnnotation("Kalman (smoothed)", 
                          ColorKalmanSmoothed, 
                          feat_points_kalman_smoothed, 
                          feat_errors_kalman_smoothed, 
                          feat_lines_kalman_smoothed,
                          feat_speeds_kalman_smoothed);
        }
        addAnnotation("Kalman", 
                      ColorKalman, 
                      feat_points_kalman, 
                      feat_errors_kalman, 
                      feat_lines_kalman,
                      feat_speeds_kalman);
    }
    
    return result_chain.references;
}

/**
*/
const Reference& ReconstructorKalman::lastReference() const
{
    return chain_cur_.references.back();
}

/**
*/
void ReconstructorKalman::newChain()
{
    if (!chain_cur_.empty())
        chains_.push_back(chain_cur_);

    chain_cur_ = {};
}

/**
*/
void ReconstructorKalman::addReference(Reference& ref,
                                       kalman::KalmanState& state,
                                       const std::string& data_info)
{
    //finalize reference (unprojection, projection change etc.)
    finalizeReference(ref, state, data_info);

    //add to current chain
    chain_cur_.add(ref, state, proj_handler_.projectionCenter());
}

/**
*/
void ReconstructorKalman::xPos(kalman::Vector& x, const Measurement& mm) const
{
    xPos(x, mm.x, mm.y);
}

/**
*/
void ReconstructorKalman::storeState(Reference& ref,
                                     const kalman::KalmanState& state) const
{
    storeState_impl(ref, state); //invoke derived impl

    ref.cov = state.P;
}

/**
*/
Reference ReconstructorKalman::storeState(const kalman::KalmanState& state,
                                          const Measurement& mm) const
{
    Reference ref;
    ref.source_id    =  mm.source_id;
    ref.t            =  mm.t;
    ref.nostddev_pos = !mm.hasStdDevPosition();
    ref.mm_interp    =  mm.mm_interp;

    storeState(ref, state);
    
    return ref;
}

/**
*/
bool ReconstructorKalman::needsReinit(const Reference& ref, 
                                      const Measurement& mm, 
                                      int flags,
                                      std::string* reason) const
{
    if ((flags & ReinitFlags::ReinitCheckDistance) && base_config_.max_distance > 0)
    {
        const double d_sqr = distanceSqr(ref, mm, CoordSystem::Cart);
        if (d_sqr > max_distance_sqr_)
        {
            if (reason)
                *reason = "distance^2 = " + std::to_string(d_sqr) + " (>" + std::to_string(max_distance_sqr_) + ")"; 
            return true;
        }
    }

    if ((flags & ReinitFlags::ReinitCheckTime) && base_config_.max_dt > 0)
    {
        double dt = Reconstructor::timestep(ref, mm);
        if (dt > base_config_.max_dt)
        {
            if (reason)
                *reason = "dt = " + std::to_string(dt) + " (>" + std::to_string(base_config_.max_dt) + ")"; 
            return true;
        }
    }

    return false;
}

/**
*/
void ReconstructorKalman::init(Measurement& mm,
                               const std::string& data_info)
{
    //init projection and project measurement
    proj_handler_.initProjection(mm, true);

    //start new chain
    newChain();

    //reinit kalman state
    init_impl(mm); //invoke derived impl

    //add first state
    kalman::KalmanState state = kalmanState();
    Reference           ref   = storeState(state, mm);

    ref.reset_pos   = true;
    ref.nospeed_pos = mm.hasVelocity();
    ref.noaccel_pos = mm.hasAcceleration();

    addReference(ref, state, data_info);
}

/**
*/
bool ReconstructorKalman::reinitIfNeeded(Measurement& mm, 
                                         const std::string& data_info,
                                         int flags)
{
    const auto& last_ref = lastReference();

    std::string msg;
    if (needsReinit(last_ref, mm, flags, &msg))
    {
        if (verbosity() > 0)
            loginf << data_info << ": Reinitializing kalman filter at t = " << mm.t << ", " << msg;

        init(mm, data_info);
        return true;
    }

    return false;
}

/**
*/
double ReconstructorKalman::timestep(const Measurement& mm) const
{
    return Reconstructor::timestep(lastReference(), mm);
}

/**
*/
reconstruction::Uncertainty ReconstructorKalman::defaultUncertaintyOfMeasurement(const Measurement& mm) const
{
    //init to standard values
    reconstruction::Uncertainty uncert;
    uncert.pos_var   = rVar();
    uncert.speed_var = rVar();
    uncert.acc_var   = rVar();

    //try to get uncertainty for source
    const auto& source_uncert = sourceUncertainty(mm.source_id);
    if (source_uncert)
        uncert = source_uncert.value();

    //set to high uncertainty if value is missing (pos is always available)
    if (!tracksVelocity() || !mm.hasVelocity())
        uncert.speed_var = rVarHigh();
    if (!tracksAcceleration() || !mm.hasAcceleration())
        uncert.acc_var = rVarHigh();

    return uncert;
}

/**
 * Implements reconstruction for all kalman-based reconstructors.
*/
boost::optional<std::vector<Reference>> ReconstructorKalman::reconstruct_impl(std::vector<Measurement>& measurements,
                                                                              const std::string& data_info)
{
    init();

    if (measurements.size() < min_chain_size_)
        return {};

    //init kalman using first target report
    auto& mm0 = measurements.at(0);
    init(mm0, data_info);

    size_t n = measurements.size();

    for (size_t i = 1; i < n; ++i)
    {
        auto& mm = measurements[ i ];

        //reinit based on timestep threshold?
        if (reinitIfNeeded(mm, data_info, ReinitFlags::ReinitCheckTime))
            continue;

        double dt = timestep(mm);
        if (dt <= base_config_.min_dt)
        {
            if (verbosity() > 0)
                loginf << data_info << ": Skipping very small timestep of " << dt << " @ mm=" << i << " t=" << mm.t;
            
            continue;
        }

        //initialize measurement (handle projection etc.)
        initMeasurement(mm);

        //do kalman step
        auto state = kalmanStep(dt, mm);
        if (!state.has_value())
        {
            //@TODO: what to do?
            logerr << data_info << ": Kalman step failed @ mm=" << i << " t=" << mm.t;
            return {};
        }

        //store new state
        auto ref = storeState(state.value(), mm);

        //reinit based on distance threshold?
        if (reinitIfNeeded(mm, data_info, ReinitFlags::ReinitCheckDistance))
            continue;

        //collect reference
        addReference(ref, state.value(), data_info);
    }

    //add last uncollected chain
    newChain();

    //finalize and check result
    auto result = finalize();
    if (!result.has_value() || result->size() < min_chain_size_)
        return {};

    return result;
}

/**
 * Initializes some measurement internals.
 */
void ReconstructorKalman::initMeasurement(Measurement& mm) const
{
    //project measurement
    //note: the last kalman state is always in sync with the current projection,
    //so the projected measurement will lie in the same map projection system as the last state.
    proj_handler_.project(mm);
}

/**
 * Finalizes some reference internals (e.g. handles map projection changes).
 */
void ReconstructorKalman::finalizeReference(Reference& ref,
                                            kalman::KalmanState& state,
                                            const std::string& data_info) const
{
    if (base_config_.map_proj_mode != MapProjectionMode::None)
    {
        if (!chain_cur_.empty())
        {
            //check if a projection change is needed, if yes trigger post proc
            if (proj_handler_.changeProjectionIfNeeded(ref, state))
            {
                postProjectionChange(state);

                if (verbosity() > 1)
                    loginf << data_info << ": Changed map projection @t=" << ref.t;
            }
        }

        assert(proj_.valid());
    }
}

/**
 * Updates the given kalman state after a change of map projection.
 */
void ReconstructorKalman::postProjectionChange(const kalman::KalmanState& state) const
{
    //update state vector in kalman instance
    xVec(state.x);
}

} // namespace reconstruction
