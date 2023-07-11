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

#include "reconstructor_kalman_projection.h"
#include "reconstructor_kalman.h"

#include "projection/projector.h"

namespace reconstruction
{

/**
*/
RecKalmanProjectionHandler::RecKalmanProjectionHandler(ReconstructorKalman* rec)
:   rec_ (rec)
,   proj_(new FrameProjector)
{
    assert(rec_);
    assert(proj_);
}

/**
*/
RecKalmanProjectionHandler::~RecKalmanProjectionHandler() = default;

/**
 * Check if a valid projection is set.
 */
bool RecKalmanProjectionHandler::valid() const
{
    return (proj_ && proj_->valid());
}

/**
 * Initializes the map projection depending on the current projection mode.
 */
void RecKalmanProjectionHandler::initProjection(Measurement& mm, bool project_mm)
{
    initProjection(mm.lat, mm.lon);

    if (project_mm)
        project(mm);
}

/**
 * Initializes the map projection depending on the current projection mode.
 */
void RecKalmanProjectionHandler::initProjection(double lat, 
                                                double lon)
{
    if (settings_.map_proj_mode == MapProjectionMode::MapProjectNone)
        return;

    assert(proj_);

    if (settings_.map_proj_mode == MapProjectionMode::MapProjectStatic)
    {
        assert(rec_);

        //update to measurement region only once
        if (!proj_->valid())
            proj_->update(rec_->regionOfInterestWGS84());
    }
    else if (settings_.map_proj_mode == MapProjectionMode::MapProjectDynamic)
    {
        //update to measurement location
        proj_->update(lat,lon);
    }

    assert(proj_->valid());
}

/**
 * Projects the given measurement using the current map projection.
 */
void RecKalmanProjectionHandler::project(Measurement& mm) const
{
    project(mm.x, mm.y, mm.lat, mm.lon);
}

/**
 * Projects the given geodetic coords using the current map projection.
 */
void RecKalmanProjectionHandler::project(double& x, 
                                         double& y, 
                                         double lat, 
                                         double lon) const
{
    if (settings_.map_proj_mode == MapProjectionMode::MapProjectNone)
        return;

    assert(proj_ && proj_->valid());

    //project using current projection
    proj_->project(x, y, lat, lon);
}

/**
 * Unprojects the given reference using the current map projection (obtains wgs84 location).
 */
void RecKalmanProjectionHandler::unproject(Reference& ref) const
{
    unproject(ref.lat, ref.lon, ref);
}

/**
 * Unprojects the given reference using the current map projection (obtains wgs84 location),
 * but stores the coordinates to the passed variables.
 */
void RecKalmanProjectionHandler::unproject(double& lat, 
                                           double& lon, 
                                           const Reference& ref) const
{
    lat = ref.lat;
    lon = ref.lon;

    if (settings_.map_proj_mode == MapProjectionMode::MapProjectNone)
        return;

    assert(proj_ && proj_->valid());

    //unproject using current projection
    proj_->unproject(lat, lon, ref.x, ref.y);
}

/**
 * Unprojects the given cartesian coordinate to geodetic using the given projection center.
 * Note: The projection center is cached in the projector, so calling this function
 * with the same center multiple times will not result in a map projection creation overhead.
 */
void RecKalmanProjectionHandler::unproject(double& lat, 
                                           double& lon, 
                                           double x, 
                                           double y, 
                                           const Eigen::Vector2d& proj_center) const
{
    proj_->update(proj_center.x(), proj_center.y());
    proj_->unproject(lat, lon, x, y);
}

/**
 * Unprojects the given references using their respective map projection centers.
*/
void RecKalmanProjectionHandler::unproject(std::vector<Reference>& references,
                                           const std::vector<Eigen::Vector2d>& proj_centers) const
{
    //nothing to do?
    if (settings_.map_proj_mode == MapProjectionMode::MapProjectNone)
        return;

    assert(references.size() == proj_centers.size());

    for (size_t i = 0; i < references.size(); ++i)
        unproject(references[ i ].lat, references[ i ].lon, references[ i ].x, references[ i ].y, proj_centers[ i ]);
}

/**
 * Checks if the currently set map projection is still valid or outdated.
 */
bool RecKalmanProjectionHandler::projectionValid(const Reference& ref) const
{
    if (settings_.map_proj_mode == MapProjectionMode::MapProjectDynamic)
    {
        //dynamic projection mode => check if map prjection still valid
        assert(proj_ && proj_->valid());

        const auto& center = proj_->centerCart();

        //distance from last projection origin is bigger than threshold?
        const double d_sqr = (Eigen::Vector2d(center.x(), center.y()) - Eigen::Vector2d(ref.x, ref.y)).squaredNorm();
        if (d_sqr > settings_.proj_max_dist_cart_sqr)
            return false;
    }

    //in all other cases return true
    return true;
}

/**
 * Changes the current map projection to the given reference's wgs84 coordinates,
 * and updates the given reference and kalman state accordingly.
 */
void RecKalmanProjectionHandler::changeProjection(Reference& ref,
                                                  kalman::KalmanState& state)
{
    if (settings_.map_proj_mode != MapProjectionMode::MapProjectDynamic)
        return;

    assert(rec_);

    //obtain geodetic ref coords from current projection
    double lat, lon;
    unproject(lat, lon, ref);

    //update projection to new geodetic coords
    initProjection(lat, lon);

    //update cartesian coords of ref to new map projection
    project(ref.x, ref.y, lat, lon);

    //store new position to state vector
    //note: we assume only small changes in map projection, 
    //so directions, velocities, accelerations and connected uncertainties 
    //are held constant during a projection change.
    rec_->xPos(state.x, ref);

    //log change of projection
    ref.projchange_pos = true;
}

/**
 * Checks if a projection change is needed, and if yes changes it and returns true.
 * Returns false otherwise.
 */
bool RecKalmanProjectionHandler::changeProjectionIfNeeded(Reference& ref, 
                                                          kalman::KalmanState& state)
{
    if (projectionValid(ref))
        return false;

    changeProjection(ref, state);

    return true;
}

/**
 * Reprojects the given state vector to a new map projection.
 */
void RecKalmanProjectionHandler::xReprojected(kalman::Vector& x_repr,
                                              const kalman::Vector& x,
                                              const Eigen::Vector2d& proj_center,
                                              const Eigen::Vector2d& proj_center_new)
{
    assert(rec_);
    assert(proj_);

    x_repr = x;

    //projection centers do match => nothing more to do
    if (proj_center == proj_center_new)
        return;

    //set old projection
    

    //obtain cartesian position from state vector
    double x_pos_cart, y_pos_cart;
    rec_->xPos(x_pos_cart, y_pos_cart, x);

    //unproject to geodetic using old projection center
    proj_->update(proj_center.x(), proj_center.y());
    double pos_lat, pos_lon;
    proj_->unproject(pos_lat, pos_lon, x_pos_cart, y_pos_cart);

    //project to cartesian using new projection center
    proj_->update(proj_center_new.x(), proj_center_new.y());
    proj_->project(x_pos_cart, y_pos_cart, pos_lat, pos_lon);

    //set new position in state vector
    rec_->xPos(x_repr, x_pos_cart, y_pos_cart);
}

/**
 * Generates a state vector transformation from one map projection system into another one.
 * Used e.g. for rts smoothing or kalman state interpolation.
 */
kalman::XTransferFunc RecKalmanProjectionHandler::reprojectionTransform(const KalmanChain& chain)
{
    //only dynamic mode needs a valid transformation
    if (settings_.map_proj_mode != MapProjectionMode::MapProjectDynamic)
        return {};

    auto repr_trafo = [ & ] (kalman::Vector& x_tr, const kalman::Vector& x, size_t idx_old, size_t idx_new)
    {
        //data for the map reprojection is provided by the indices into the given kalman chain
        this->xReprojected(x_tr, x, chain.proj_centers[idx_old], chain.proj_centers[idx_new]);
    };

    return repr_trafo;
}

/**
 * Returns the current projection center.
 * Note: just returns (0,0) if no valid projection is set.
*/
Eigen::Vector2d RecKalmanProjectionHandler::projectionCenter() const
{
    if (settings_.map_proj_mode == MapProjectionMode::MapProjectNone)
        return Eigen::Vector2d(0, 0);

    assert(proj_);

    return (proj_->valid() ? Eigen::Vector2d(proj_->centerLat(), proj_->centerLon()) : Eigen::Vector2d(0, 0));
}

} // namespace reconstruction
